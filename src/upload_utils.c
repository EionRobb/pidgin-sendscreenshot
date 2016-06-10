 /*
  * Pidgin SendScreenshot plugin - common upload utilities -
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

#include "upload_utils.h"
#include "prefs.h"
#include "error.h"

/*
 * Set common curl options for http and ftp upload.
 */
void
plugin_curl_set_common_opts (CURL * curl, PurplePlugin * plugin) {
    const PurpleProxyInfo *gpi = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);
    g_assert (curl != NULL);
    g_assert (PLUGIN (account) != NULL);

    /* install timeouts */
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT,
                      purple_prefs_get_int (PREF_UPLOAD_CONNECTTIMEOUT));
    curl_easy_setopt (curl, CURLOPT_TIMEOUT,
                      purple_prefs_get_int (PREF_UPLOAD_TIMEOUT));

    /* use proxy settings */
    if ((gpi = purple_proxy_get_setup (PLUGIN (account))) != NULL) {
        PurpleProxyType proxy_type = purple_proxy_info_get_type (gpi);
        if (proxy_type != PURPLE_PROXY_NONE) {
            long curl_proxy_type;
            const gchar *proxy_username = NULL;
            const gchar *proxy_password = NULL;

            proxy_username = purple_proxy_info_get_username (gpi);
            proxy_password = purple_proxy_info_get_password (gpi);

            /* set proxy type */
            if (proxy_type == PURPLE_PROXY_HTTP)
                curl_proxy_type = CURLPROXY_HTTP;
            else if (proxy_type == PURPLE_PROXY_SOCKS4) {
                if (purple_prefs_get_bool ("/purple/proxy/socks4_remotedns"))
                    curl_proxy_type = CURLPROXY_SOCKS4A;
                else
                    curl_proxy_type = CURLPROXY_SOCKS4;
            }
            else if (proxy_type == PURPLE_PROXY_SOCKS5)
                curl_proxy_type = CURLPROXY_SOCKS5;
            else {
                /* should not happen */
                NotifyError ("proxy type :'%d' is invalid", proxy_type);
                return;
            }
            curl_easy_setopt (curl, CURLOPT_PROXYTYPE, curl_proxy_type);
            curl_easy_setopt (curl, CURLOPT_PROXYPORT,
                              purple_proxy_info_get_port (gpi));
            curl_easy_setopt (curl, CURLOPT_PROXY,
                              purple_proxy_info_get_host (gpi));

            if (proxy_username && !strcmp (proxy_username, "")) {
                curl_easy_setopt (curl, CURLOPT_PROXYUSERNAME,
                                  proxy_username);

                if (proxy_password && !strcmp (proxy_password, ""))
                    curl_easy_setopt (curl, CURLOPT_PROXYPASSWORD,
                                      proxy_password);
            }
        }
    }
}

/*
 * Insert the html link we just fetched from server.
 */
void
real_insert_link (PurplePlugin * plugin, const gchar * url) {
    GtkIMHtml *imhtml = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    if ((imhtml = get_receiver_imhtml (plugin)) == NULL) {
        NotifyError (PLUGIN_UPLOAD_CLOSED_CONV_ERROR, url);
    }
    else {
        GtkTextBuffer *textbf = NULL;
        GtkTextMark *mark = NULL;
        GtkTextIter iter, bw_iter;

        textbf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (imhtml));
        mark = gtk_text_buffer_get_insert (textbf);

        gtk_text_buffer_get_iter_at_mark (textbf, &iter, mark);
        bw_iter = iter;

        /* add a space to prevent messing up with previous text */
        if (gtk_text_iter_backward_char (&bw_iter)) {
            gchar *last_char = NULL;
            last_char = gtk_text_iter_get_slice (&bw_iter, &iter);
            /* not a space nor a tabulation */
            if (last_char && last_char[0] != 0x20 && last_char[0] != 0x9) {
                gtk_text_buffer_insert (textbf, &iter, " ", 1);
                mark = gtk_text_buffer_get_insert (textbf);
                gtk_text_buffer_get_iter_at_mark (textbf, &iter, mark);
            }
            if (last_char != NULL)
                g_free (last_char);
        }

        /* support for HTML-formatted messages */
        if (PLUGIN (conv_features) & PURPLE_CONNECTION_HTML) {
            gchar *basename =
                g_path_get_basename (PLUGIN (capture_path_filename));
            gtk_imhtml_insert_link (imhtml, mark, url, basename);
            g_free (basename);
        }
        else
            gtk_text_buffer_insert (textbf, &iter, url, (gint) strlen (url));
	gtk_widget_grab_focus (GTK_WIDGET(imhtml));
    }
}

/*
 * Let the user know that we are uploading...
 */
GtkWidget *
show_uploading_dialog (PurplePlugin * plugin, const gchar * str) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *img;
    GtkWidget *hbox, *vbox;
    GtkWidget *progress_bar;
    GtkWidget *gtkconv_window;
    GtkWidget *blist_window;
    GtkWidget *label = NULL;
    gchar *send_msg = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    progress_bar = gtk_progress_bar_new ();
    img =
        gtk_image_new_from_stock (PIDGIN_STOCK_UPLOAD,
                                  gtk_icon_size_from_name
                                  (PIDGIN_ICON_SIZE_TANGO_SMALL));

    hbox = gtk_hbox_new (FALSE, PIDGIN_HIG_BOX_SPACE);
    vbox = gtk_vbox_new (FALSE, PIDGIN_HIG_BOX_SPACE);

    gtkconv_window = get_receiver_window (plugin);
    blist_window = pidgin_blist_get_default_gtk_blist ()->window;

    dialog = gtk_dialog_new_with_buttons (PLUGIN_NAME,
                                          GTK_WINDOW ((gtkconv_window) ?
                                                      gtkconv_window :
                                                      blist_window),
                                          GTK_DIALOG_NO_SEPARATOR, NULL);

    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
    gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR (progress_bar), 0.05);

    g_object_set_data (G_OBJECT (dialog), "progress-bar", progress_bar);

    send_msg = g_strdup_printf (PLUGIN_SENDING_INFO, str);

    label = gtk_label_new (send_msg);

    /* gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progress_bar), send_msg); */

    g_free (send_msg);
    send_msg = NULL;

#if GTK_CHECK_VERSION (2,14,0)
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
#else
    content_area = (GTK_DIALOG (dialog))->vbox;
#endif

    gtk_window_set_deletable (GTK_WINDOW (dialog), FALSE);
    gtk_box_pack_start (GTK_BOX (content_area), vbox, FALSE, FALSE, 0);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), hbox);

    gtk_box_pack_start (GTK_BOX (hbox), img, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), progress_bar, FALSE, FALSE, 0);

    gtk_widget_show_all (dialog);
    return dialog;
}

/* end of upload_utils.c */
