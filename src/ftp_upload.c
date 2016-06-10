 /*
  * Pidgin SendScreenshot plugin - ftp upload  -
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
  *
  *
  * --  Raoul Berger <contact@raoulito.info>
  *
  */

#include "ftp_upload.h"
#include "prefs.h"
#include "upload_utils.h"
#include "error.h"

G_LOCK_DEFINE (unload);

/* store screenshot contents in buf */
static size_t
read_callback (void *buf, size_t size, size_t nmemb, void *stream) {
    PurplePlugin *plugin;
    GIOChannel *io_chan;
    GError *error = NULL;
    size_t ret;

    plugin = purple_plugins_find_with_id (PLUGIN_ID);
    io_chan = (GIOChannel *) stream;

    g_io_channel_read_chars (io_chan, buf, size * nmemb, &ret, &error);

    if (error != NULL) {
        g_propagate_error (&PLUGIN (error), error);
        return CURL_READFUNC_ABORT;
    }

    PLUGIN (read_size) += ret;  /* progress bar */
    return ret;
}

/* inspired from http://curl.haxx.se/libcurl/c/ftpupload.html */
static gpointer
ftp_upload (PurplePlugin * plugin) {
    CURL *curl;
    CURLcode res;
    GIOChannel *io_chan = NULL;

    struct stat file_info;
    gchar *remote_url = NULL;
    gchar *basename = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    G_LOCK (unload);
    /* get the file size of the local file */
    if (g_stat (PLUGIN (capture_path_filename), &file_info) == -1) {
        g_assert (PLUGIN (error) == NULL);
        g_set_error (&PLUGIN (error),
                     SENDSCREENSHOT_PLUGIN_ERROR,
                     PLUGIN_ERROR_OPEN_SCREENSHOT_FILE,
                     "%s\n%s",
                     PLUGIN_ERROR_OPEN_SCREENSHOT_FILE_s, g_strerror (errno));
        THREAD_QUIT;
    }
    PLUGIN (total_size) = file_info.st_size;

    basename = g_path_get_basename (PLUGIN (capture_path_filename));
    remote_url =
        g_strdup_printf ("%s/%s",
                         purple_prefs_get_string (PREF_FTP_REMOTE_URL),
                         basename);

    g_free (basename);
    basename = NULL;

    io_chan =
        g_io_channel_new_file (PLUGIN (capture_path_filename), "r",
                               &PLUGIN (error));
    if (io_chan == NULL) {
        g_assert (PLUGIN (error) != NULL);
        g_free (remote_url);
        THREAD_QUIT;
    }
    /* binary data, this should never fail */
    g_io_channel_set_encoding (io_chan, NULL, NULL);

    /* get a curl handle */
    curl = curl_easy_init ();
    if (curl != NULL) {
        static char curl_error[CURL_ERROR_SIZE];

        plugin_curl_set_common_opts (curl, plugin);

        /* we want to use our own read function */
        curl_easy_setopt (curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt (curl, CURLOPT_READDATA, io_chan);

        /* enable uploading */
        curl_easy_setopt (curl, CURLOPT_UPLOAD, 1L);

        /* specify target */
        curl_easy_setopt (curl, CURLOPT_URL, remote_url);

        /* specify username and password */
        curl_easy_setopt (curl, CURLOPT_USERNAME,
                          purple_prefs_get_string (PREF_FTP_USERNAME));

        curl_easy_setopt (curl, CURLOPT_PASSWORD,
                          purple_prefs_get_string (PREF_FTP_PASSWORD));

        curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, curl_error);

        /* Now run off and do what you've been told! */
        res = curl_easy_perform (curl);

        PLUGIN (read_size) = 0;

        /* always cleanup */
        curl_easy_cleanup (curl);
        g_free (remote_url);

        if (PLUGIN (error) != NULL) {   /* read_callback() failed */
            g_io_channel_unref (io_chan);
            THREAD_QUIT;
        }
        else if (res != 0) {
            g_assert (PLUGIN (error) == NULL);
            g_set_error (&PLUGIN (error),
                         SENDSCREENSHOT_PLUGIN_ERROR,
                         PLUGIN_ERROR_FTP_UPLOAD,
                         "%s\n%s", PLUGIN_ERROR_FTP_UPLOAD_s, curl_error);
        }
    }
    if ((g_io_channel_shutdown (io_chan, TRUE, NULL)) == G_IO_STATUS_ERROR) {
        g_io_channel_unref (io_chan);
        THREAD_QUIT;
    }
    g_io_channel_unref (io_chan);
    THREAD_QUIT;
}

static gboolean
insert_ftp_link_cb (PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);

    /* still uploading... */
    if (PLUGIN (libcurl_thread) != NULL) {
        GtkProgressBar *progress_bar =
            g_object_get_data (G_OBJECT (PLUGIN (uploading_dialog)),
                               "progress-bar");

        g_assert (progress_bar != NULL);

        if (PLUGIN (read_size) == 0)
            gtk_progress_bar_pulse (progress_bar);
        else {

            gchar *str = NULL;
            gdouble val = PLUGIN (read_size) / (gdouble) PLUGIN (total_size);

            str = g_strdup_printf ("%d%%", (gint) (val * 100));
            gtk_progress_bar_set_text (progress_bar, str);
            g_free (str);
            gtk_progress_bar_set_fraction (progress_bar, val);

        }
        return TRUE;
    }
    else {
        PLUGIN (timeout_cb_handle) = 0;
        gtk_widget_destroy (PLUGIN (uploading_dialog));
        PLUGIN (uploading_dialog) = NULL;

        /* an error occured */
        if (PLUGIN (error) != NULL) {
            NotifyUploadError ("%s", PLUGIN (error)->message);
            g_error_free (PLUGIN (error));
            PLUGIN (error) = NULL;
        }
        else {
            gchar *remote_url;
            gchar *basename;

            basename = g_path_get_basename (PLUGIN (capture_path_filename));

            if (strcmp (purple_prefs_get_string (PREF_FTP_WEB_ADDR), ""))
                /* not only a ftp server, but also a html server */
                remote_url = g_strdup_printf ("%s/%s",
                                              purple_prefs_get_string
                                              (PREF_FTP_WEB_ADDR), basename);
            else
                remote_url = g_strdup_printf ("%s/%s",
                                              purple_prefs_get_string
                                              (PREF_FTP_REMOTE_URL),
                                              basename);

            real_insert_link (plugin, remote_url);
            g_free (remote_url);
            g_free (basename);
        }
    }
    plugin_stop (plugin);
    return FALSE;
}

void
ftp_upload_prepare (PurplePlugin * plugin) {
    struct host_param_data *host_data;

    g_assert (plugin != NULL && plugin->extra != NULL);

    host_data = PLUGIN (host_data);

    g_assert (PLUGIN (uploading_dialog) == NULL);
    g_assert (PLUGIN (libcurl_thread) == NULL);

    PLUGIN (read_size) = 0;

    PLUGIN (uploading_dialog) =
        show_uploading_dialog (plugin,
                               purple_prefs_get_string (PREF_FTP_REMOTE_URL));
    PLUGIN (libcurl_thread) =
        g_thread_create ((GThreadFunc) ftp_upload, plugin, FALSE, NULL);

    PLUGIN (timeout_cb_handle) =
        g_timeout_add (PLUGIN_UPLOAD_PROGRESS_INTERVAL,
                       (GSourceFunc) insert_ftp_link_cb, plugin);
}

/* end of ftp_upload.c */
