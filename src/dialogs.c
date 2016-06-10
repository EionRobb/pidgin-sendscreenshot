 /*
  * Pidgin SendScreenshot third-party plugin - dialogs.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any laterr version.
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
  * --  Raoul Berger <raoul.berger@yahoo.fr>
  *
  */

#include "dialogs.h"
#include "prefs.h"
#include "screencap.h"

static guint
update_countdown (PurplePlugin * plugin) {
    GtkWidget *progress_bar = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    if (PLUGIN (countdown_dialog) == NULL)      /* forced or canceled  */
        return FALSE;

    progress_bar =
        g_object_get_data (G_OBJECT (PLUGIN (countdown_dialog)),
                           "progress-bar");

    if (progress_bar != NULL) {
        gdouble val =
            gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress_bar));

        gdouble incr = 250.0 / ((gdouble)
                                ((purple_prefs_get_int
                                  (PREF_WAIT_BEFORE_SCREENSHOT) -
                                  1)) * 1000.0);
        if (val < 0.99) {
            gchar *text = NULL;
            text = g_strdup_printf ("%.02f sec.", ((gdouble)
                                                   purple_prefs_get_int
                                                   (PREF_WAIT_BEFORE_SCREENSHOT))
                                    * (1.0 - (val + incr)));
            gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar),
                                           MIN (val + incr, 1.0));
            gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progress_bar), text);
            g_free (text);

            return TRUE;
        }
        else {
            gtk_widget_destroy (PLUGIN (countdown_dialog));
            PLUGIN (countdown_dialog) = NULL;
        }
    }
    return FALSE;
}

static
    void
on_countdown_dialog_response_cb (PurplePlugin * plugin, gint response_id) {

    g_assert (plugin != NULL && plugin->extra != NULL);

    switch (response_id) {
    case GTK_RESPONSE_ACCEPT:
        {
            if (purple_timeout_remove (PLUGIN (timeout_source))) {
                PLUGIN (timeout_source) = 0;
                /* delete_infobar (plugin); */

                gtk_widget_destroy (PLUGIN (countdown_dialog));
                PLUGIN (countdown_dialog) = NULL;

                PLUGIN (timeout_source) =
                    purple_timeout_add
                    (MSEC_TIMEOUT_VAL, (GSourceFunc) timeout_freeze_screen,
                     plugin);

            }
            break;
        }
    case GTK_RESPONSE_REJECT:
    case GTK_RESPONSE_DELETE_EVENT:
        {

            if (purple_timeout_remove (PLUGIN (timeout_source))) {
                PLUGIN (timeout_source) = 0;
                gtk_widget_destroy (PLUGIN (countdown_dialog));
                PLUGIN (countdown_dialog) = NULL;
                plugin_stop (plugin);
            }
            break;
        }
    }

}

void
show_countdown_dialog (PurplePlugin * plugin) {
    GtkWidget *content_area;
    GtkWidget *img;
    GtkWidget *hbox, *vbox;
    GtkWidget *progress_bar;
    GtkWidget *gtkconv_window;
    GtkWidget *blist_window;
    GtkWidget *label = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    g_assert (PLUGIN (countdown_dialog) == NULL);

    progress_bar = gtk_progress_bar_new ();
    img =
        gtk_image_new_from_stock (PIDGIN_STOCK_INFO,
                                  gtk_icon_size_from_name
                                  (PIDGIN_ICON_SIZE_TANGO_SMALL));

    hbox = gtk_hbox_new (FALSE, PIDGIN_HIG_BOX_SPACE);
    vbox = gtk_vbox_new (FALSE, PIDGIN_HIG_BOX_SPACE);

    gtkconv_window = get_receiver_window (plugin);
    blist_window = pidgin_blist_get_default_gtk_blist ()->window;

    PLUGIN (countdown_dialog) =
        gtk_dialog_new_with_buttons (PLUGIN_NAME,
                                     GTK_WINDOW ((gtkconv_window) ?
                                                 gtkconv_window :
                                                 blist_window),
                                     0, GTK_STOCK_EXECUTE,
                                     GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
                                     GTK_RESPONSE_REJECT, NULL);

    gtk_window_set_resizable (GTK_WINDOW (PLUGIN (countdown_dialog)), FALSE);
    gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR (progress_bar), 0.05);

    g_object_set_data (G_OBJECT (PLUGIN (countdown_dialog)), "progress-bar",
                       progress_bar);

    label = gtk_label_new (COUNTDOWN_MSG);

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), 0);

#if GTK_CHECK_VERSION (2,14,0)
    content_area =
        gtk_dialog_get_content_area (GTK_DIALOG (PLUGIN (countdown_dialog)));
#else
    content_area = (GTK_DIALOG (PLUGIN (countdown_dialog)))->vbox;
#endif

    gtk_window_set_deletable (GTK_WINDOW (PLUGIN (countdown_dialog)), FALSE);
    gtk_box_pack_start (GTK_BOX (content_area), vbox, FALSE, FALSE, 0);
    gtk_box_pack_start_defaults (GTK_BOX (vbox), hbox);

    gtk_box_pack_start (GTK_BOX (hbox), img, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), progress_bar, FALSE, FALSE, 0);

    gtk_widget_show_all (PLUGIN (countdown_dialog));

    purple_timeout_add (250, (GSourceFunc) update_countdown, plugin);

    g_signal_connect_swapped (G_OBJECT (PLUGIN (countdown_dialog)),
                              "response",
                              G_CALLBACK (on_countdown_dialog_response_cb),
                              plugin);
}

/* stolen from gtkfilechooserentry.c */
static gboolean
entry_select_filename_in_focus_cb (GtkWidget * entry, GdkEventFocus * event) {
    const gchar *str, *ext;
    glong len = -1;

    str = gtk_entry_get_text (GTK_ENTRY (entry));
    ext = g_strrstr (str, ".");

    if (ext)
        len = g_utf8_pointer_to_offset (str, ext);

    gtk_editable_select_region (GTK_EDITABLE (entry), 0, (gint) len);

    (void) event;
    return TRUE;
}

/* part stolen from gtkutils.c */
static void
set_sensitive_if_input_and_noexist (GtkWidget * entry,
                                    GtkWidget * dlgbox_rename) {
    GtkWidget *warn_label;
    gchar *capture_path_filename = NULL;
    const gchar *text;

    warn_label = g_object_get_data (G_OBJECT (dlgbox_rename), "warn-label");

    text = purple_escape_filename (gtk_entry_get_text (GTK_ENTRY (entry)));

    capture_path_filename =
        g_build_filename (purple_prefs_get_string (PREF_STORE_FOLDER),
                          text, NULL);

    gtk_dialog_set_response_sensitive (GTK_DIALOG (dlgbox_rename),
                                       GTK_RESPONSE_OK, (*text != '\0'));
    /* warn user if file already exists */
    if (g_file_test (capture_path_filename, G_FILE_TEST_EXISTS)
        && (*text != '\0'))
        gtk_widget_show (warn_label);
    else
        gtk_widget_hide (warn_label);

    if (capture_path_filename != NULL)
        g_free (capture_path_filename);
}

/* part stolen from pidgin/gtkconv.c */
static GtkWidget *
capture_rename (PurplePlugin * plugin, const gchar * entry_init) {
    GdkColor red = { 0, 65535, 0, 0 };
    GtkWidget *dlgbox_rename;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *warn_label;
    GtkWidget *gtkconv_window;
    GtkWidget *blist_window;
    GtkWidget *img;
    GtkWidget *content_area;

    img =
        gtk_image_new_from_stock (PIDGIN_STOCK_DIALOG_QUESTION,
                                  gtk_icon_size_from_name
                                  (PIDGIN_ICON_SIZE_TANGO_HUGE));
    gtkconv_window = get_receiver_window (plugin);
    blist_window = pidgin_blist_get_default_gtk_blist ()->window;

    dlgbox_rename = gtk_dialog_new_with_buttons (DLGBOX_CAPNAME_TITLE,
                                                 GTK_WINDOW ((gtkconv_window)
                                                             ? gtkconv_window
                                                             : blist_window),
                                                 GTK_DIALOG_MODAL |
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_STOCK_OK,
                                                 GTK_RESPONSE_OK, NULL);

#if GTK_CHECK_VERSION(2,14,0)
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dlgbox_rename));
#else
    content_area = GTK_DIALOG (dlgbox_rename)->vbox;
#endif

    gtk_dialog_set_default_response (GTK_DIALOG (dlgbox_rename),
                                     GTK_RESPONSE_OK);

    gtk_container_set_border_width (GTK_CONTAINER
                                    (dlgbox_rename), PIDGIN_HIG_BOX_SPACE);
    gtk_window_set_resizable (GTK_WINDOW (dlgbox_rename), FALSE);
    gtk_dialog_set_has_separator (GTK_DIALOG (dlgbox_rename), FALSE);
    gtk_box_set_spacing (GTK_BOX (content_area), PIDGIN_HIG_BORDER);
    gtk_container_set_border_width (GTK_CONTAINER (content_area),
                                    PIDGIN_HIG_BOX_SPACE);

    hbox = gtk_hbox_new (FALSE, PIDGIN_HIG_BORDER);
    gtk_container_add (GTK_CONTAINER (content_area), hbox);
    gtk_box_pack_start (GTK_BOX (hbox), img, FALSE, FALSE, 0);

    gtk_misc_set_alignment (GTK_MISC (img), 0, 0);
    label = gtk_label_new (DLGBOX_CAPNAME_LABEL);

    warn_label = gtk_label_new (NULL);

    gtk_widget_modify_fg (warn_label, GTK_STATE_NORMAL, &red);
    gtk_label_set_text (GTK_LABEL (warn_label), DLGBOX_CAPNAME_WARNEXISTS);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (content_area), warn_label, FALSE, FALSE, 0);
    entry = gtk_entry_new ();

    gtk_entry_set_text (GTK_ENTRY (entry), entry_init);

    gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);

    g_object_set (gtk_widget_get_settings (entry),
                  "gtk-entry-select-on-focus", FALSE, NULL);
    g_signal_connect (G_OBJECT (entry), "changed",
                      G_CALLBACK (set_sensitive_if_input_and_noexist),
                      dlgbox_rename /* plugin */ );
    g_signal_connect (G_OBJECT (entry), "focus-out-event",
                      G_CALLBACK (entry_select_filename_in_focus_cb), plugin);

    g_signal_connect (G_OBJECT (entry), "focus-in-event",
                      G_CALLBACK (entry_select_filename_in_focus_cb), plugin);
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

    gtk_widget_show (label);
    gtk_widget_show (entry);
    gtk_widget_show (img);
    gtk_widget_show (hbox);
    gtk_widget_show (content_area);

    g_object_set_data (G_OBJECT (dlgbox_rename), "entry", entry);
    g_object_set_data (G_OBJECT (dlgbox_rename), "warn-label", warn_label);
    return dlgbox_rename;
}

/** Eventually ask for a custom capture name when sending :
    - as file 
    - to a remote FTP server
*/
void
screenshot_maybe_rename (PurplePlugin * plugin, gchar ** basename) {
    g_assert (plugin != NULL && plugin->extra != NULL);

    if (purple_prefs_get_bool (PREF_ASK_FILENAME) &&
        (PLUGIN (send_as) == SEND_AS_FILE
#ifdef ENABLE_UPLOAD
         || PLUGIN (send_as) == SEND_AS_FTP_LINK)
#else
        )
#endif
        ) {
        GtkWidget *dlgbox_rename = NULL;
        gint result = 0;

        dlgbox_rename = capture_rename (plugin, *basename);
        result = gtk_dialog_run (GTK_DIALOG (dlgbox_rename));

        if (result == GTK_RESPONSE_OK) {
            GtkWidget *entry = NULL;

            entry = g_object_get_data (G_OBJECT (dlgbox_rename), "entry");

            g_free (*basename);
            *basename = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
        }
        gtk_widget_destroy (dlgbox_rename);
    }
}

/* end of dialogs.h */
