 /*
  * Pidgin SendScreenshot - preferences -.
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
  * --  Raoul Berger <raoul.berger@yahoo.fr>
  *
  */

#include "prefs.h"
#include "pixbuf_utils.h"
#include "error.h"

#ifdef ENABLE_UPLOAD
#include "upload_utils.h"
#include "http_upload.h"
#endif

/* stolen from pidgin/gtkprefs.c */
static void
update_spin_value (GtkWidget * w, GtkWidget * spin) {
    const char *key = g_object_get_data (G_OBJECT (spin), "val");
    int value;

    value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin));

    purple_prefs_set_int (key, value);
    (void) w;
}

static void
entry_set (GtkEntry * entry, gpointer data) {
    const char *key = (const char *) data;

    purple_prefs_set_string (key, gtk_entry_get_text (entry));
}


/* same as pidgin_prefs_labeled_spin_button() but with a custom step */
static GtkWidget *
pidgin_prefs_labeled_spin_button_custom (GtkWidget * box, const gchar * title,
                                         const char *key,
                                         int min, int max, int step,
                                         GtkSizeGroup * sg) {
    GtkWidget *spin;
    GtkObject *adjust;
    int val;

    val = purple_prefs_get_int (key);

    adjust = gtk_adjustment_new (val, min, max, step, 1, 0);
    spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1, 0);
    g_object_set_data (G_OBJECT (spin), "val", (char *) key);
    if (max < 10000)
        gtk_widget_set_size_request (spin, 50, -1);
    else
        gtk_widget_set_size_request (spin, 60, -1);
    g_signal_connect (G_OBJECT (adjust), "value-changed",
                      G_CALLBACK (update_spin_value), GTK_WIDGET (spin));
    gtk_widget_show (spin);

    return pidgin_add_widget_to_vbox (GTK_BOX (box), title, sg, spin, FALSE,
                                      NULL);
}

#ifdef ENABLE_UPLOAD
/* same as pidgin_prefs_labeled_entry() but using invisible chars */
static GtkWidget *
pidgin_prefs_labeled_entry_custom (GtkWidget * page, const gchar * title,
                                   const char *key,
                                   gint max_length,
                                   gboolean visible, GtkSizeGroup * sg) {
    GtkWidget *entry;
    const gchar *value;

    value = purple_prefs_get_string (key);

    entry = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY (entry), visible);
    if (max_length > 0)
        gtk_entry_set_max_length (GTK_ENTRY (entry), max_length);
    gtk_entry_set_text (GTK_ENTRY (entry), value);
    g_signal_connect (G_OBJECT (entry), "changed",
                      G_CALLBACK (entry_set), (char *) key);
    gtk_widget_show (entry);

    return pidgin_add_widget_to_vbox (GTK_BOX (page), title, sg, entry, TRUE,
                                      NULL);
}

static void
hosts_combobox_changed_cb (GtkComboBox * hosts_combobox) {
    enum { COLUMN_PIXBUF, COLUMN_STRING, N_COLUMNS };

    gchar *host_name = NULL;
    gchar *tos_filename = NULL;
    gchar *host_name_txt = NULL;
    gchar *contents = NULL;
    GtkTreeIter iter;
    GdkPixbuf *pb;              /* useless here */

    gtk_combo_box_get_active_iter (hosts_combobox, &iter);

    gtk_tree_model_get (gtk_combo_box_get_model (hosts_combobox),
                        &iter,
                        COLUMN_PIXBUF, &pb, COLUMN_STRING, &host_name, -1);

    host_name_txt = g_strdup_printf ("%s.txt", host_name);

    tos_filename =
        g_build_filename (PLUGIN_DATADIR, "pidgin_screenshot_data", "tos",
                          host_name_txt, NULL);

    if (g_file_get_contents (tos_filename, &contents, NULL, NULL) == TRUE) {
        GtkWidget *tos_dialog;
        GtkWidget *content_area;
        GtkWidget *view;
        GtkWidget *scrolled_view;
        GtkTextBuffer *buffer;
        gint result;

        scrolled_view = gtk_scrolled_window_new (NULL, NULL);

        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_view),
                                        GTK_POLICY_NEVER,
                                        GTK_POLICY_AUTOMATIC);
        view = gtk_text_view_new ();

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

        gtk_text_buffer_set_text (buffer, contents, -1);
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view),
                                     GTK_WRAP_WORD_CHAR);
        gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);

        tos_dialog = gtk_dialog_new_with_buttons (PIDGIN_HOST_TOS,
                                                  NULL,
                                                  GTK_DIALOG_MODAL |
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_STOCK_APPLY,
                                                  GTK_RESPONSE_YES,
                                                  GTK_STOCK_CANCEL,
                                                  GTK_RESPONSE_NO, NULL);

#if GTK_CHECK_VERSION (2,14,0)
        content_area = gtk_dialog_get_content_area (GTK_DIALOG (tos_dialog));
#else
        content_area = (GTK_DIALOG (tos_dialog))->vbox;
#endif

        gtk_box_pack_start (GTK_BOX (content_area), scrolled_view, TRUE, TRUE,
                            0);
        gtk_widget_set_size_request (view, 450, 450);
        gtk_container_add (GTK_CONTAINER (scrolled_view), view);

        gtk_dialog_set_default_response (GTK_DIALOG (tos_dialog),
                                         GTK_RESPONSE_NO);
        gtk_widget_show (content_area);
        gtk_widget_show (scrolled_view);
        gtk_widget_show (view);

        result = gtk_dialog_run (GTK_DIALOG (tos_dialog));

        if (result == GTK_RESPONSE_YES)
            purple_prefs_set_string (PREF_UPLOAD_TO, host_name);
        else
            gtk_combo_box_set_active (GTK_COMBO_BOX (hosts_combobox), 0);

        gtk_widget_destroy (tos_dialog);
    }
    /* No TOS available, apply ! */
    else
        purple_prefs_set_string (PREF_UPLOAD_TO, host_name);

    g_free (host_name_txt);
    g_free (host_name);
    g_free (tos_filename);
    if (pb != NULL)
        g_object_unref (pb);
    if (contents != NULL)
        g_free (contents);
}

static gboolean
website_link_motion_cb (GtkWidget * button, GdkEventCrossing * event) {
    pidgin_set_cursor (button, GDK_HAND2);
    return TRUE;
    (void) event;
}

static gboolean
website_link_clicked_cb (PurplePlugin * plugin, GdkEventButton * event) {
    purple_notify_uri (NULL, purple_plugin_get_homepage (plugin));
    return TRUE;
    (void) event;
}

static void
insert_error_frame (const gchar * error_msg, GtkWidget * vbox,
                    PurplePlugin * plugin) {
    GtkWidget *err_label;
    GtkWidget *plugin_website;
    GtkWidget *website_link;

    gchar *err_msg = NULL, *buf = NULL;

    website_link = gtk_event_box_new ();
#if GTK_CHECK_VERSION(2,4,0)
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (website_link), FALSE);
#endif

    err_msg = g_strdup_printf ("<span foreground='red'><b>%s.</b></span>",
                               error_msg);
    plugin_website = gtk_label_new (NULL);

    err_label = gtk_label_new (err_msg);
    g_free (err_msg);

    gtk_label_set_use_markup (GTK_LABEL (err_label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (err_label), TRUE);

    buf = g_strdup_printf ("<span underline=\"single\" "
                           "foreground=\"blue\">%s</span>", MORE_INFO);
    gtk_label_set_markup (GTK_LABEL (plugin_website), buf);
    g_free (buf);

    gtk_container_add (GTK_CONTAINER (website_link),
                       GTK_WIDGET (plugin_website));

    pidgin_add_widget_to_vbox (GTK_BOX (vbox), "", NULL, err_label, FALSE,
                               NULL);
    pidgin_add_widget_to_vbox (GTK_BOX (vbox), "", NULL, website_link,
                               FALSE, NULL);

    g_signal_connect_swapped (website_link, "button-release-event",
                              G_CALLBACK (website_link_clicked_cb), plugin);
    g_signal_connect (website_link, "enter-notify-event",
                      G_CALLBACK (website_link_motion_cb), NULL);
    g_signal_connect (website_link, "leave-notify-event",
                      G_CALLBACK (pidgin_clear_cursor), NULL);
}

static void
prefs_start_element_handler (GMarkupParseContext * context,
                             const gchar * element_name,
                             const gchar ** attribute_names,
                             const gchar ** attribute_values,
                             gpointer user_data, GError ** error) {
    gint line_number, char_number;
    struct host_param_data *host_data = (struct host_param_data *) user_data;

    g_markup_parse_context_get_position (context, &line_number, &char_number);

    if (!strcmp (element_name, "upload_hosts")) {       /* fetch xml host version */
        if (attribute_names[0] && !strcmp (attribute_names[0], "version")
            && !host_data->xml_hosts_version)
            host_data->xml_hosts_version = g_strdup (attribute_values[0]);
        else {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         PLUGIN_PARSE_XML_ERRMSG_MISSATTR,
                         line_number, char_number, element_name);
        }
    }
    else if (!strcmp (element_name, "host")) {
        if (attribute_names[0]) {
            if (!strcmp (attribute_names[0], "name")) {
                if (strcmp (attribute_values[0], HOST_DISABLED)) {      /* just in case */
                    gchar *host_name = g_strdup (attribute_values[0]);
                    g_array_append_val (host_data->host_names, host_name);
                }
            }
            else
                g_set_error (error,
                             G_MARKUP_ERROR,
                             G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
                             PLUGIN_PARSE_XML_ERRMSG_INVATTR,
                             line_number, char_number,
                             attribute_names[0], element_name);
        }
        else
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         PLUGIN_PARSE_XML_ERRMSG_MISSATTR,
                         line_number, char_number, element_name);

    }
    (void) context;
    (void) error;
}

static GMarkupParser prefs_parser = {
    prefs_start_element_handler,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

#if GTK_CHECK_VERSION(2,6,0)
static void
change_store_folder_cb (GtkFileChooser * folder_chooser) {
    gchar *new_folder = gtk_file_chooser_get_current_folder (folder_chooser);

    if (new_folder != NULL) {
        purple_prefs_set_string (PREF_STORE_FOLDER, new_folder);
        g_free (new_folder);
    }
}

static void
change_signature_cb (GtkFileChooser * file_chooser, GtkImage * sign_image) {
    gchar *new_signature = gtk_file_chooser_get_filename (file_chooser);

    if (new_signature != NULL) {
        GdkPixbuf *sign_pixbuf;

        sign_pixbuf = gdk_pixbuf_new_from_file (new_signature, NULL);

        if (!mygdk_pixbuf_check_maxsize
            (sign_pixbuf, SIGN_MAXWIDTH, SIGN_MAXHEIGHT)) {
            PurplePlugin *plugin = purple_plugins_find_with_id (PLUGIN_ID);

            NotifyError (PLUGIN_SIGNATURE_TOOBIG_ERROR, SIGN_MAXWIDTH,
                         SIGN_MAXHEIGHT);
        }
        else {
            gtk_image_set_from_pixbuf (sign_image, sign_pixbuf);
            purple_prefs_set_string (PREF_SIGNATURE_FILENAME, new_signature);
        }
        g_object_unref (sign_pixbuf);
        sign_pixbuf = NULL;
        g_free (new_signature);
    }
}
#endif /* GTK_CHECK_VERSION(2,6,0) */

static void
gchar_add (gchar ** str, const gchar * add, const gchar * separator) {
    if (*str != NULL) {
        gchar *tmp = g_strjoin (separator, *str, add, NULL);
        g_free (*str);
        *str = tmp;
    }
    else
        *str = g_strdup (add);
}

#define maybe_add_modifier(m, m_strval)		\
  if (event->state & m) {			\
    gchar_add (&combo, m_strval, "+");		\
    all_modifiers |= m;				\
  }

#define SHIFT_MODIFIER_STR "Shift"
#define CONTROL_MODIFIER_STR "Ctrl"
#define ALT_MODIFIER_STR "Alt"

static gchar *
get_combo (PurplePlugin * plugin, const gchar * key_combo) {

    gchar *res = NULL;

    guint combo = purple_prefs_get_int (key_combo);

    if (combo & GDK_SHIFT_MASK)
        gchar_add (&res, SHIFT_MODIFIER_STR, "+");
    if (combo & GDK_CONTROL_MASK)
        gchar_add (&res, CONTROL_MODIFIER_STR, "+");
    if (combo & GDK_MOD1_MASK)
        gchar_add (&res, ALT_MODIFIER_STR, "+");
    (void) plugin;
    return res;
}

static gboolean
on_combo_entry_key_press_cb (GtkWidget * entry,
                             GdkEventKey * event, const gchar * key) {
    if (event->is_modifier == FALSE) {
        gchar *combo = NULL;
        guint all_modifiers = 0;

        maybe_add_modifier (GDK_SHIFT_MASK, SHIFT_MODIFIER_STR);
        maybe_add_modifier (GDK_CONTROL_MASK, CONTROL_MODIFIER_STR);
        maybe_add_modifier (GDK_MOD1_MASK, ALT_MODIFIER_STR);

        /* validate modifiers combo */
        if (all_modifiers
            && event->keyval
            && event->keyval != GDK_BackSpace
            && event->keyval != GDK_Tab && event->keyval != GDK_Return) {

	    pref_keyval_modifier(key_combo, key);

            gchar_add (&combo,
                       gdk_keyval_name (gdk_keyval_to_lower (event->keyval)),
                       "+");
            gtk_entry_set_text (GTK_ENTRY (entry), combo);

            /* save keyval */
            purple_prefs_set_int (key, gdk_keyval_to_lower (event->keyval));

            /* save combo of modifiers */
            purple_prefs_set_int (key_combo,
				  all_modifiers);

            g_free (key_combo);
            g_free (combo);
            return TRUE;
        }
        gtk_entry_set_text (GTK_ENTRY (entry), STR_INVALID_KEY);
        purple_prefs_set_int (key, 0);
    }
    return TRUE;
}

#define add_hotkey_entry(vbox, title, key)				\
  {									\
    GtkWidget * _entry;							\
    gchar * entry_combo = NULL;						\
    pref_keyval_modifier(key_combo, key);				\
    entry_combo = get_combo(plugin, key_combo);				\
    if (entry_combo)							\
      gchar_add (&entry_combo, gdk_keyval_name(purple_prefs_get_int(key)), "+"); \
    else								\
      entry_combo = g_strdup (STR_INVALID_KEY);				\
    _entry = gtk_entry_new ();						\
    gtk_entry_set_text (GTK_ENTRY (_entry), entry_combo);		\
    pidgin_add_widget_to_vbox (GTK_BOX (vbox), title, NULL, _entry, TRUE,  NULL); \
    g_signal_connect (G_OBJECT(_entry), "key-release-event",		\
		      G_CALLBACK(on_combo_entry_key_press_cb), key);	\
    g_free (key_combo);							\
    g_free (entry_combo);						\
  }

GtkWidget *
get_plugin_pref_frame (PurplePlugin * plugin) {
    GtkWidget *tab1, *tab2, *tab3;
    GtkWidget *vbox = NULL;
    GtkWidget *dropdown_imgtype;
    GtkWidget *dropdown_sndtype;
    GtkWidget *hbox_imgtype, *hbox_sign;
    GtkWidget *prefs_book;

#if GTK_CHECK_VERSION(2,6,0)
    GdkPixbuf *sign_pixbuf;
    GtkWidget *folder_chooser, *file_chooser, *sign_image;
    GtkFileFilter *filter;
#endif

    g_assert (plugin != NULL && plugin->extra != NULL);

#ifdef ENABLE_UPLOAD
    GtkWidget *hbox_ftp, *hbox_html;
    enum { COLUMN_PIXBUF, COLUMN_STRING, N_COLUMNS };
    GtkListStore *list_store;
    GtkTreeIter iter;
    guint active_idx = 0;
    GtkCellRenderer *text_renderer;
    GtkCellRenderer *pixbuf_renderer;
    GtkWidget *hosts_combobox;

    gchar *contents = NULL;
    gsize length;
    GError *error = NULL;
#else
    (void) plugin;
#endif /* ENABLE_UPLOAD */

    prefs_book = gtk_notebook_new ();

    tab1 = gtk_vbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);
    gtk_container_set_border_width (GTK_CONTAINER (tab1), PIDGIN_HIG_BORDER);

    tab2 = gtk_vbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);
    gtk_container_set_border_width (GTK_CONTAINER (tab2), PIDGIN_HIG_BORDER);


    gtk_notebook_append_page (GTK_NOTEBOOK (prefs_book), tab1,
                              gtk_label_new (PREFS_TAB1));


    gtk_notebook_append_page (GTK_NOTEBOOK (prefs_book), tab2,
                              gtk_label_new (PREFS_TAB2));

#ifdef ENABLE_UPLOAD
    tab3 = gtk_vbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);
    gtk_container_set_border_width (GTK_CONTAINER (tab3), PIDGIN_HIG_BORDER);

    gtk_notebook_append_page (GTK_NOTEBOOK (prefs_book), tab3,
                              gtk_label_new (PREFS_TAB3));
#endif /* ENABLE_UPLOAD */


    /* =========================================================================
     *      IMAGE PARAMETERS
     * ========================================================================= */
    vbox = pidgin_make_frame (tab1, PREF_UI_FRAME1);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);

    hbox_imgtype = gtk_hbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);
    hbox_sign = gtk_hbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);

    dropdown_sndtype =
        pidgin_prefs_dropdown (vbox, PREF_UI_SEND_TYPE, PURPLE_PREF_STRING,
                               PREF_SEND_TYPE, PREF_SEND_IMG_FTP_HTTP, "img-ftp-http",
			       NULL);
    dropdown_imgtype =
        pidgin_prefs_dropdown (vbox, PREF_UI_IMAGE_TYPE, PURPLE_PREF_STRING,
                               PREF_IMAGE_TYPE, PREF_TYPE_JPG, "jpeg",
                               PREF_TYPE_PNG, "png", PREF_TYPE_BMP, "bmp",
                               NULL);

    gtk_box_pack_start (GTK_BOX (vbox), hbox_imgtype, TRUE, FALSE, 0);
    pidgin_prefs_labeled_spin_button (hbox_imgtype,
                                      PREF_UI_PNG_COMRPESSION,
                                      PREF_PNG_COMPRESSION, 0, 9, NULL);
    pidgin_prefs_labeled_spin_button (hbox_imgtype,
                                      PREF_UI_JPEG_QUALITY,
                                      PREF_JPEG_QUALITY, 0, 100, NULL);

#if GTK_CHECK_VERSION(2,6,0)
    filter = gtk_file_filter_new ();
    gtk_file_filter_add_pixbuf_formats (filter);

    gtk_file_filter_set_name (filter, _("Image"));

    file_chooser = gtk_file_chooser_button_new (PREF_UI_SIGNATURE,
                                                GTK_FILE_CHOOSER_ACTION_OPEN);

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser), filter);

    pidgin_prefs_checkbox (PREF_UI_SIGNATURE, PREF_ADD_SIGNATURE, hbox_sign);

    sign_pixbuf =
        gdk_pixbuf_new_from_file (purple_prefs_get_string
                                  (PREF_SIGNATURE_FILENAME), NULL);

    sign_image = gtk_image_new_from_pixbuf (NULL);

    if (sign_pixbuf != NULL) {
        if (!mygdk_pixbuf_check_maxsize
            (sign_pixbuf, SIGN_MAXWIDTH, SIGN_MAXHEIGHT)) {
            NotifyError (PLUGIN_SIGNATURE_TOOBIG_ERROR, SIGN_MAXWIDTH,
                         SIGN_MAXHEIGHT);
            purple_prefs_set_string (PREF_SIGNATURE_FILENAME, "");
        }
        gtk_image_set_from_pixbuf (GTK_IMAGE (sign_image), sign_pixbuf);
        g_object_unref (sign_pixbuf);
        sign_pixbuf = NULL;
    }

    gtk_box_pack_start (GTK_BOX (hbox_sign), sign_image, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_sign), file_chooser, TRUE, FALSE, 0);

    g_signal_connect (G_OBJECT (file_chooser), "selection-changed",
                      G_CALLBACK (change_signature_cb), sign_image);

    gtk_box_pack_start (GTK_BOX (vbox), hbox_sign, TRUE, FALSE, 0);
#endif


    /* =========================================================================
     *      GENERAL
     * ========================================================================= */

    vbox = pidgin_make_frame (tab1, PREF_UI_FRAME2);

    pidgin_prefs_dropdown (vbox, PREF_UI_HIGHLIGHT_MODE, PURPLE_PREF_INT,
                           PREF_HIGHLIGHT_MODE,
                           PREF_HIGHLIGHT_MODE_01, 1,
                           PREF_HIGHLIGHT_MODE_02, 2,
                           PREF_HIGHLIGHT_MODE_03, 3,
                           PREF_HIGHLIGHT_MODE_04, 4,
                           PREF_HIGHLIGHT_MODE_05, 5, NULL);
    pidgin_prefs_checkbox (PREF_UI_SHOW_VISUAL_CUES, PREF_SHOW_VISUAL_CUES,
                           vbox);

    vbox = pidgin_make_frame (tab1, PREF_UI_FRAME6);
#if GTK_CHECK_VERSION(2,6,0)
    folder_chooser = gtk_file_chooser_button_new (PREF_UI_STORE_FOLDER,
                                                  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (folder_chooser),
                                         purple_prefs_get_string
                                         (PREF_STORE_FOLDER));

    g_signal_connect (G_OBJECT (folder_chooser), "current-folder-changed",
                      G_CALLBACK (change_store_folder_cb), NULL);

    pidgin_add_widget_to_vbox (GTK_BOX (vbox), PREF_UI_STORE_FOLDER, NULL,
                               folder_chooser, FALSE, NULL);
#else
    pidgin_prefs_labeled_entry (vbox,
                                PREF_UI_STORE_FOLDER, PREF_STORE_FOLDER,
                                NULL);
#endif


    pidgin_prefs_checkbox (PREF_UI_ASK_FILENAME, PREF_ASK_FILENAME, vbox);
    pidgin_prefs_checkbox (PREF_UI_ONLY_SAVE_WHEN, PREF_ONLY_SAVE_WHEN, vbox);

    /* == misc == */
    vbox = pidgin_make_frame (tab1, PREF_UI_FRAME7);
    pidgin_prefs_labeled_spin_button_custom (vbox,
                                             PREF_UI_WAIT_BEFORE_SCREENSHOT,
                                             PREF_WAIT_BEFORE_SCREENSHOT, 0,
                                             30, 5, NULL);

    /* =========================================================================
     *      HOTKEYS
     * ========================================================================= */
    vbox = pidgin_make_frame (tab2, PREF_UI_FRAME9);

    add_hotkey_entry (vbox,
                      PREF_UI_HOTKEYS_SEND_AS_FILE,
                      PREF_HOTKEYS_SEND_AS_FILE);
#ifdef ENABLE_UPLOAD
    add_hotkey_entry (vbox,
                      PREF_UI_HOTKEYS_SEND_AS_FTP, PREF_HOTKEYS_SEND_AS_FTP);

    add_hotkey_entry (vbox,
                      PREF_UI_HOTKEYS_SEND_AS_HTTP,
                      PREF_HOTKEYS_SEND_AS_HTTP);
#endif
    add_hotkey_entry (vbox,
                      PREF_UI_HOTKEYS_SEND_AS_IMAGE,
                      PREF_HOTKEYS_SEND_AS_IMAGE);


#ifdef ENABLE_UPLOAD


    /* =========================================================================
     *      FTP UPLOAD
     * ========================================================================= */
    vbox = pidgin_make_frame (tab3, PREF_UI_FRAME4);

    pidgin_prefs_labeled_entry (vbox, PREF_UI_FTP_REMOTE_URL,
                                PREF_FTP_REMOTE_URL, NULL);
    pidgin_prefs_labeled_entry (vbox, PREF_UI_FTP_WEB_ADDR,
                                PREF_FTP_WEB_ADDR, NULL);

    hbox_ftp = gtk_hbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);

    gtk_box_pack_start (GTK_BOX (vbox), hbox_ftp, TRUE, FALSE, 0);

    pidgin_prefs_labeled_entry (hbox_ftp, PREF_UI_FTP_USERNAME,
                                PREF_FTP_USERNAME, NULL);
    pidgin_prefs_labeled_entry_custom (hbox_ftp, PREF_UI_FTP_PASSWORD,
                                       PREF_FTP_PASSWORD, -1, FALSE, NULL);

    /* =========================================================================
     *      HTTP UPLOAD
     * ========================================================================= */

    vbox = pidgin_make_frame (tab3, PREF_UI_FRAME3);

    if (g_file_get_contents (PLUGIN (xml_hosts_filename),
                             &contents, &length, &error) == FALSE) {
        g_assert (error != NULL);
        insert_error_frame (error->message, vbox, plugin);
        g_error_free (error);
        error = NULL;
        if (contents != NULL)
            g_free (contents);
    }
    else {
        GMarkupParseContext *context;
        struct host_param_data *host_data = PLUGIN (host_data);

        context =
            g_markup_parse_context_new (&prefs_parser, 0, host_data, NULL);

        host_data->host_names = g_array_new (FALSE, FALSE, sizeof (gchar *));

        if (!g_markup_parse_context_parse (context, contents, length, &error)) {
            gchar *err_msg_escaped = NULL;

            err_msg_escaped = g_markup_escape_text (error->message, -1);

            insert_error_frame (err_msg_escaped, vbox, plugin);
            g_error_free (error);
            error = NULL;
            g_free (err_msg_escaped);
            g_free (contents);
            g_markup_parse_context_free (context);
            CLEAR_HOST_PARAM_DATA (host_data);
        }
        else {
            if (host_data->host_names->len > 0) {
                gchar *ui_upload_to = NULL;

                GdkPixbuf *disabled_pb;
                gchar *disabled;
                guint i;
                GtkWidget *hostlist_label;
                GtkWidget *hostnames_link;

                hostnames_link = gtk_event_box_new ();
#if GTK_CHECK_VERSION(2,4,0)
                gtk_event_box_set_visible_window (GTK_EVENT_BOX
                                                  (hostnames_link), FALSE);
#endif

                ui_upload_to = g_strdup_printf (PREF_UI_UPLOAD_TO,
                                                host_data->xml_hosts_version);

                hostlist_label = gtk_label_new (NULL);
                gtk_label_set_use_markup (GTK_LABEL (hostlist_label), TRUE);
                gtk_label_set_markup (GTK_LABEL (hostlist_label),
                                      ui_upload_to);
                g_free (ui_upload_to);
                ui_upload_to = NULL;

                gtk_container_add (GTK_CONTAINER (hostnames_link),
                                   GTK_WIDGET (hostlist_label));

                g_signal_connect_swapped (hostnames_link,
                                          "button-release-event",
                                          G_CALLBACK
                                          (website_link_clicked_cb), plugin);
                g_signal_connect (hostnames_link, "enter-notify-event",
                                  G_CALLBACK (website_link_motion_cb), NULL);
                g_signal_connect (hostnames_link, "leave-notify-event",
                                  G_CALLBACK (pidgin_clear_cursor), NULL);

                list_store =
                    gtk_list_store_new (N_COLUMNS, GDK_TYPE_PIXBUF,
                                        G_TYPE_STRING);

                disabled = g_strdup (HOST_DISABLED);
                disabled_pb = gtk_widget_render_icon (vbox,
                                                      GTK_STOCK_CANCEL,
                                                      GTK_ICON_SIZE_BUTTON,
                                                      NULL);

                gtk_list_store_append (list_store, &iter);
                gtk_list_store_set (list_store, &iter,
                                    COLUMN_PIXBUF, disabled_pb,
                                    COLUMN_STRING, disabled, -1);
                g_object_unref (disabled_pb);
                g_free (disabled);

                for (i = 0; i < host_data->host_names->len; i++) {
                    gchar *host_name =
                        g_array_index (host_data->host_names, gchar *, i);
                    gchar *host_name_ico =
                        g_strdup_printf ("%s.png", host_name);

                    gchar *pixbuf_filename = g_build_filename (PLUGIN_DATADIR,
                                                               "pidgin_screenshot_data",
                                                               "icons",
                                                               host_name_ico,
                                                               NULL);
                    GdkPixbuf *pixbuf =
                        gdk_pixbuf_new_from_file (pixbuf_filename, NULL);

                    if ((pixbuf != NULL)
                        && (gdk_pixbuf_get_width (pixbuf) > PIXBUF_HOSTS_SIZE
                            || gdk_pixbuf_get_height (pixbuf) >
                            PIXBUF_HOSTS_SIZE)) {
                        g_object_unref (pixbuf);
                        pixbuf = NULL;
                    }

                    if (!strcmp
                        (host_name, purple_prefs_get_string (PREF_UPLOAD_TO)))
                        active_idx = i + 1;

                    gtk_list_store_append (list_store, &iter);
                    gtk_list_store_set (list_store, &iter,
                                        COLUMN_PIXBUF, pixbuf,
                                        COLUMN_STRING, host_name, -1);

                    g_free (host_name_ico);
                    if (pixbuf != NULL)
                        g_object_unref (pixbuf);
                    g_free (pixbuf_filename);
                }

                text_renderer = gtk_cell_renderer_text_new ();
                pixbuf_renderer = gtk_cell_renderer_pixbuf_new ();
                hosts_combobox =
                    gtk_combo_box_new_with_model (GTK_TREE_MODEL
                                                  (list_store));
                g_object_unref (list_store);

                gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (hosts_combobox),
                                            pixbuf_renderer, FALSE);
                gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (hosts_combobox),
                                            text_renderer, TRUE);
                /* bindings */
                gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT
                                                (hosts_combobox),
                                                text_renderer, "text",
                                                COLUMN_STRING, NULL);
                gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT
                                                (hosts_combobox),
                                                pixbuf_renderer, "pixbuf",
                                                COLUMN_PIXBUF, NULL);
#if GTK_CHECK_VERSION (2,6,0)
                gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (hosts_combobox),
                                              2);
#endif

                gtk_combo_box_set_active (GTK_COMBO_BOX (hosts_combobox),
                                          active_idx);

                g_signal_connect (G_OBJECT (hosts_combobox), "changed",
                                  G_CALLBACK (hosts_combobox_changed_cb),
                                  NULL);

                hbox_html = gtk_hbox_new (FALSE, PIDGIN_HIG_CAT_SPACE);

                gtk_box_pack_start (GTK_BOX (vbox), hbox_html, TRUE, FALSE,
                                    0);
                gtk_box_pack_start (GTK_BOX (hbox_html), hostnames_link, TRUE,
                                    FALSE, 0);
                gtk_box_pack_end (GTK_BOX (hbox_html), hosts_combobox, TRUE,
                                  FALSE, 0);
                g_markup_parse_context_free (context);
                g_free (contents);
            }
            else {
                insert_error_frame (PLUGIN_UPLOAD_XML_ERROR, vbox, plugin);
            }
            CLEAR_HOST_PARAM_DATA (host_data);
        }
    }
    /* =========================================================================
     *   GENERAL UPLOAD OPTIONS
     * ========================================================================= */
    vbox = pidgin_make_frame (tab3, PREF_UI_FRAME5);
    pidgin_prefs_labeled_spin_button (vbox,
                                      PREF_UI_UPLOAD_CONNECTTIMEOUT,
                                      PREF_UPLOAD_CONNECTTIMEOUT, 1, 240,
                                      NULL);
    pidgin_prefs_labeled_spin_button (vbox, PREF_UI_UPLOAD_TIMEOUT,
                                      PREF_UPLOAD_TIMEOUT, 1, 240, NULL);
#endif /* ENABLE_UPLOAD */

    return prefs_book;
}

/* end of prefs.c */
