 /*
  * Pidgin SendScreenshot plugin - http upload  -
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
#include "http_upload.h"
#include "prefs.h"
#include "error.h"

/* progressively obtains http response (stored in buf) */
static size_t
write_function (void *buf, size_t size, size_t nmemb, void *data) {
    PurplePlugin *plugin = (PurplePlugin *) data;

    gchar *previous = PLUGIN (host_data)->html_response;
    gsize previous_size = 0;

    if (previous != NULL)
        previous_size = strlen (previous);

    if ((previous = g_try_realloc (previous,
                                   previous_size * sizeof (gchar) +
                                   nmemb * size + 1)) != NULL) {
        guint c;
        gchar *new = previous + previous_size;

        for (c = 0; c < nmemb; c++)
            *new++ = *((gchar *) buf++);

        *new = '\0';
        PLUGIN (host_data)->html_response = previous;
        return nmemb * size;
    }
    else
        return CURLE_WRITE_ERROR;
}

static gpointer
http_upload_thread (PurplePlugin * plugin) {
    guint i;
    CURL *curl;
    CURLcode res;

    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";        /* disable expectation */
    gchar *img_ctype = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    /* prevent the plugin from beeing unloaded */
    G_LOCK (unload);
    img_ctype =
        g_strdup_printf ("image/%s",
                         purple_prefs_get_string (PREF_IMAGE_TYPE));
    /* fill in extra fields */
    for (i = 0; i < PLUGIN (host_data)->extra_names->len; i++) {
        curl_formadd (&formpost,
                      &lastptr,
                      CURLFORM_COPYNAME,
                      g_array_index (PLUGIN (host_data)->extra_names, gchar *,
                                     i), CURLFORM_COPYCONTENTS,
                      g_array_index (PLUGIN (host_data)->extra_values,
                                     gchar *, i), CURLFORM_END);
    }
    /* fill in the file upload field */
    curl_formadd (&formpost,
                  &lastptr,
                  CURLFORM_COPYNAME,
                  PLUGIN (host_data)->file_input_name,
                  CURLFORM_FILE, PLUGIN (capture_path_filename),
                  CURLFORM_CONTENTTYPE, img_ctype, CURLFORM_END);
    g_free (img_ctype);

    headerlist = curl_slist_append (headerlist, buf);
    if ((curl = curl_easy_init ()) != NULL) {
        static char curl_error[CURL_ERROR_SIZE];

        curl_easy_setopt (curl, CURLOPT_URL, PLUGIN (host_data)->form_action);

        plugin_curl_set_common_opts (curl, plugin);

        curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);

        curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, curl_error);

        /* hmm... or InternetExplorer? */
        curl_easy_setopt (curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        curl_easy_setopt (curl, CURLOPT_WRITEDATA, plugin);
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headerlist);

        curl_easy_setopt (curl, CURLOPT_HTTPPOST, formpost);

        res = curl_easy_perform (curl);

        curl_formfree (formpost);
        curl_slist_free_all (headerlist);
        curl_easy_cleanup (curl);

        if (res != 0) {
            g_assert (PLUGIN (error) == NULL);
            g_set_error (&PLUGIN (error),
                         SENDSCREENSHOT_PLUGIN_ERROR,
                         PLUGIN_ERROR_HTTP_UPLOAD,
                         PLUGIN_ERROR_HTTP_UPLOAD_s,
                         PLUGIN (host_data)->selected_hostname, curl_error);
        }
    }
    THREAD_QUIT;
}

/*
 * Retrieve informations to post to a server.
 */
static void
xml_get_host_data_start_element_handler (GMarkupParseContext * context,
                                         const gchar * element_name,
                                         const gchar ** attribute_names,
                                         const gchar ** attribute_values,
                                         gpointer user_data, GError ** error) 
{
    struct host_param_data *host_data = (struct host_param_data *) user_data;
    gint line_number, char_number;

    if (host_data->quit_handlers == TRUE)       /* don't parse anymore */
        return;

    g_markup_parse_context_get_position (context, &line_number, &char_number);

    /* found host in xml */
    if (!strcmp (element_name, "upload_hosts")) {
        return;
    }
    else if (!strcmp (element_name, "host") && !host_data->is_inside) {
        if (attribute_names[0]) {
            if (!strcmp (attribute_names[0], "name")) {
                if (!strcmp
                    (attribute_values[0], host_data->selected_hostname))
                    host_data->is_inside = TRUE;
            }
            else
                g_set_error (error,
                             G_MARKUP_ERROR,
                             G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
                             PLUGIN_PARSE_XML_ERRMSG_INVATTR,
                             line_number, char_number,
                             attribute_names[0], element_name);
        }
        else {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         PLUGIN_PARSE_XML_ERRMSG_MISSATTR,
                         line_number, char_number, element_name);
        }
    }
    else if (!strcmp (element_name, "param")) {
        if (host_data->is_inside) {
            if (attribute_names[0]) {
                if (!strcmp (attribute_names[0], "form_action")
                    && !host_data->form_action) {
                    host_data->form_action = g_strdup (attribute_values[0]);
                }
                else if (!strcmp (attribute_names[0], "file_input_name")
                         && !host_data->file_input_name) {
                    host_data->file_input_name =
                        g_strdup (attribute_values[0]);
                }
                else if (!strcmp (attribute_names[0], "regexp")
                         && !host_data->regexp) {
                    host_data->regexp = g_strdup (attribute_values[0]);
                }
                else if (!strcmp (attribute_names[0], "name")) {
                    if (attribute_names[1]
                        && !strcmp (attribute_names[1], "value")) {
                        gchar *extra_name = g_strdup (attribute_values[0]);
                        gchar *extra_value = g_strdup (attribute_values[1]);

                        g_array_append_val (host_data->extra_names,
                                            extra_name);
                        g_array_append_val (host_data->extra_values,
                                            extra_value);
                    }
                    else {
                        g_set_error (error,
                                     G_MARKUP_ERROR,
                                     G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                                     PLUGIN_PARSE_XML_ERRMSG_MISSATTRVAL,
                                     line_number, char_number);
                    }
                }
                else if (!strcmp (attribute_names[0], "location")) {
                    /* ignored, fixme */
                }
                else {
                    g_set_error (error,
                                 G_MARKUP_ERROR,
                                 G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
                                 PLUGIN_PARSE_XML_ERRMSG_INVATTR,
                                 line_number, char_number,
                                 attribute_names[0], element_name);
                }
            }
            else {
                g_set_error (error,
                             G_MARKUP_ERROR,
                             G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                             PLUGIN_PARSE_XML_ERRMSG_MISSATTR,
                             line_number, char_number, element_name);
            }
        }
    }
    else {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                     PLUGIN_PARSE_XML_ERRMSG_INVELEM,
                     line_number, char_number, element_name);
    }
}

static void
xml_get_host_data_end_element_handler (GMarkupParseContext * context,
                                       const gchar * element_name,
                                       gpointer user_data, GError ** error) {
    struct host_param_data *host_data = (struct host_param_data *) user_data;

    if (host_data->quit_handlers)
        return;

    if (!strcmp (element_name, "host") && host_data->is_inside) {
        host_data->quit_handlers = TRUE;        /* don't parse anymore */
    }
    (void) context;
    (void) error;
}

GMarkupParser xml_get_host_data_parser = {
    xml_get_host_data_start_element_handler,
    xml_get_host_data_end_element_handler,
    NULL,
    NULL,
    NULL
};

static gboolean
insert_html_link_cb (PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);

    /* still uploading... */
    if (PLUGIN (libcurl_thread) != NULL) {
        GtkProgressBar *progress_bar = NULL;
        progress_bar =
            g_object_get_data (G_OBJECT (PLUGIN (uploading_dialog)),
                               "progress-bar");
        g_assert (progress_bar != NULL);        /* can't close dialog */

        gtk_progress_bar_pulse (progress_bar);

        return TRUE;
    }
    else {
        GError *error = NULL;

        PLUGIN (timeout_cb_handle) = 0;
        gtk_widget_destroy (PLUGIN (uploading_dialog));
        PLUGIN (uploading_dialog) = NULL;

        if (PLUGIN (error) != NULL) {
            NotifyUploadError ("%s", PLUGIN (error)->message);
            g_error_free (PLUGIN (error));
            PLUGIN (error) = NULL;
        }
        else {
            GRegex *url_regex = NULL;
            GRegex *space_regex = NULL;

            if ((url_regex =
                 g_regex_new (PLUGIN (host_data)->regexp,
                              G_REGEX_MULTILINE, 0, &error)) == NULL) {
                gchar *errmsg;

                g_assert (error != NULL);
                errmsg = g_strdup_printf (PLUGIN_UPLOAD_BAD_REGEXP_ERROR,
                                          error->message);

                NotifyUploadError ("%s", errmsg);
                g_free (errmsg);
                g_error_free (error);
            }
            else {
                GMatchInfo *url_match_info = NULL;
                gchar *nospace = NULL;

                /* remove every newlines first */
                space_regex = g_regex_new ("\\s", G_REGEX_MULTILINE, 0, NULL);
                nospace = g_regex_replace_literal (space_regex,
                                                   PLUGIN
                                                   (host_data)->html_response,
                                                   -1, 0, "", 0, NULL);
                g_regex_unref (space_regex);
                if (g_regex_match (url_regex, nospace, 0, &url_match_info) ==
                    FALSE) {
                    gchar *errmsg;
                    if (url_match_info != NULL)
                        g_match_info_free (url_match_info);
                    g_regex_unref (url_regex);
                    g_free (nospace);

                    errmsg =
                        g_strdup_printf (PLUGIN_UPLOAD_FETCHURL_ERROR,
                                         PLUGIN
                                         (host_data)->selected_hostname);

                    purple_debug_info (PLUGIN_ID,
                                       gettext_noop
                                       ("Regexp doesn't match HTTP upload response !\nServer: %s\nRegexp:\n%s\nResponse:\n%s\n"),
                                       PLUGIN (host_data)->selected_hostname,
                                       PLUGIN (host_data)->regexp,
                                       PLUGIN (host_data)->html_response);

                    NotifyUploadError ("%s", errmsg);
                    g_free (errmsg);
                }
                else {
                    gchar *url = g_match_info_fetch (url_match_info, 1);

                    g_regex_unref (url_regex);
                    g_free (nospace);
                    g_match_info_free (url_match_info);
                    real_insert_link (plugin, url);
                    g_free (url);
                }
                g_free (PLUGIN (host_data)->html_response);
                PLUGIN (host_data)->html_response = NULL;
            }
        }
    }
    CLEAR_HOST_PARAM_DATA_FULL (PLUGIN (host_data));
    plugin_stop (plugin);
    return FALSE;
}

void
http_upload_prepare (PurplePlugin * plugin) {
    struct host_param_data *host_data;
    gsize length;
    GError *error = NULL;
    GMarkupParseContext *context;
    gchar *xml_contents = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    host_data = PLUGIN (host_data);
    host_data->selected_hostname =
        g_strdup (purple_prefs_get_string (PREF_UPLOAD_TO));

    if (!g_file_get_contents
        (PLUGIN (xml_hosts_filename), &xml_contents, &length, &error)) {
        NotifyError (PLUGIN_LOAD_XML_ERROR, error->message, PLUGIN_WEBSITE);

        g_error_free (error);
        g_free (host_data->selected_hostname);
        host_data->selected_hostname = NULL;

        plugin_stop (plugin);
        return;
    }
    context =
        g_markup_parse_context_new (&xml_get_host_data_parser, 0, host_data,
                                    NULL);

    host_data->extra_names = g_array_new (FALSE, FALSE, sizeof (gchar *));
    host_data->extra_values = g_array_new (FALSE, FALSE, sizeof (gchar *));

    if (!g_markup_parse_context_parse (context, xml_contents, length, &error)
        || !g_markup_parse_context_end_parse (context, &error)
        || !host_data->form_action || !host_data->file_input_name
        || !host_data->regexp) {

        if (error != NULL) {
            gchar *errmsg_parse;
            gchar *errmsg_referto;

            errmsg_parse = g_strdup_printf (PLUGIN_PARSE_XML_ERROR,
                                            PLUGIN (xml_hosts_filename));

            errmsg_referto = g_strdup_printf (PLUGIN_PLEASE_REFER_TO,
                                              PLUGIN_WEBSITE);

            NotifyError ("%s\n%s\n%s", errmsg_parse, error->message,
                         errmsg_referto);

            g_free (errmsg_referto);
            g_free (errmsg_parse);
            g_error_free (error);

        }
        else if (!host_data->is_inside) {
            gchar *errmsg_uploadto;
            gchar *errmsg_referto;

            errmsg_uploadto = g_strdup_printf (PLUGIN_XML_STUFF_MISSING,
                                               PLUGIN
                                               (host_data)->selected_hostname);
            errmsg_referto =
                g_strdup_printf (PLUGIN_PLEASE_REFER_TO, PLUGIN_WEBSITE);

            NotifyError ("%s\n\t%s\n\n%s",
                         errmsg_uploadto,
                         PLUGIN_PARSE_XML_MISSING_HOST, errmsg_referto);

            g_free (errmsg_uploadto);
            g_free (errmsg_referto);

        }
        else if (!host_data->form_action) {
            gchar *errmsg_uploadto;
            gchar *errmsg_referto;

            errmsg_uploadto = g_strdup_printf (PLUGIN_XML_STUFF_MISSING,
                                               PLUGIN
                                               (host_data)->selected_hostname);
            errmsg_referto =
                g_strdup_printf (PLUGIN_PLEASE_REFER_TO, PLUGIN_WEBSITE);

            NotifyError ("%s\n\t%s\n\n%s",
                         errmsg_uploadto,
                         PLUGIN_PARSE_XML_MISSING_ACTION, errmsg_referto);

            g_free (errmsg_uploadto);
            g_free (errmsg_referto);

        }
        else if (!host_data->file_input_name) {
            gchar *errmsg_uploadto;
            char *errmsg_referto;

            errmsg_uploadto = g_strdup_printf (PLUGIN_XML_STUFF_MISSING,
                                               PLUGIN
                                               (host_data)->selected_hostname);
            errmsg_referto =
                g_strdup_printf (PLUGIN_PLEASE_REFER_TO, PLUGIN_WEBSITE);

            NotifyError ("%s\n\t%s\n\n%s",
                         errmsg_uploadto,
                         PLUGIN_PARSE_XML_MISSING_INPUT, errmsg_referto);

            g_free (errmsg_uploadto);
            g_free (errmsg_referto);

        }
        else if (!host_data->regexp) {
            gchar *errmsg_uploadto;
            gchar *errmsg_referto;

            errmsg_uploadto = g_strdup_printf (PLUGIN_XML_STUFF_MISSING,
                                               PLUGIN
                                               (host_data)->selected_hostname);
            errmsg_referto =
                g_strdup_printf (PLUGIN_PLEASE_REFER_TO, PLUGIN_WEBSITE);

            NotifyError ("%s\n\t%s\n\n%s",
                         errmsg_uploadto,
                         PLUGIN_PARSE_XML_MISSING_REGEXP, errmsg_referto);

            g_free (errmsg_uploadto);
            g_free (errmsg_referto);
        }

        g_markup_parse_context_free (context);
        g_free (xml_contents);
        CLEAR_HOST_PARAM_DATA_FULL (host_data);
        plugin_stop (plugin);
        return;
    }
    g_markup_parse_context_free (context);
    g_free (xml_contents);


    /* upload to server */
    g_assert (PLUGIN (uploading_dialog) == NULL);
    g_assert (PLUGIN (libcurl_thread) == NULL);

    PLUGIN (uploading_dialog) =
        show_uploading_dialog (plugin, PLUGIN (host_data)->selected_hostname);

    PLUGIN (libcurl_thread) =
        g_thread_create ((GThreadFunc) http_upload_thread, plugin, FALSE,
                         NULL);
    PLUGIN (timeout_cb_handle) =
        g_timeout_add (PLUGIN_UPLOAD_PROGRESS_INTERVAL,
                       (GSourceFunc) insert_html_link_cb, plugin);
}

/* end of http_upload.c*/
