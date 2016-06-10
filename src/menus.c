 /*
  * Pidgin SendScreenshot third-party plugin - menus and menuitems-
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

#include "main.h"
#include "menus.h"
#include "prefs.h"
#include "screencap.h"
#include "error.h"

#ifdef ENABLE_UPLOAD

#include "upload_utils.h"
#include "http_upload.h"
#include "ftp_upload.h"

//static void
//on_screenshot_insert_as_link_aux 

static void
send_as_link (PidginWindow * win,
                                  PidginConversation * _gtkconv) {
    PurplePlugin *plugin;

    plugin = purple_plugins_find_with_id (PLUGIN_ID);

    if (PLUGIN (locked))
        return;                 /* Just return, don't fail. */
    else {
        PidginConversation *gtkconv;

        PLUGIN (locked) = TRUE;
        PLUGIN (send_as) = SEND_AS_HTTP_LINK;

        if (win != NULL)
            gtkconv =
                PIDGIN_CONVERSATION
                (pidgin_conv_window_get_active_conversation (win));
        else
            gtkconv = _gtkconv;

        if (!strcmp (purple_prefs_get_string (PREF_UPLOAD_TO), HOST_DISABLED)) {
            purple_notify_error (plugin, PLUGIN_NAME, PLUGIN_ERROR,
                                 PLUGIN_HOST_DISABLED_ERROR);
            plugin_stop (plugin);
            return;
        }
        REMEMBER_ACCOUNT (gtkconv);

        PLUGIN (conv_features) = gtkconv->active_conv->features;
        freeze_desktop (plugin, FALSE);
    }
}


/*
static void
on_screenshot_insert_as_link_fromwin_activate_cb (PidginWindow * win) {
    on_screenshot_insert_as_link_aux (win, NULL);
}

static void
on_screenshot_insert_as_link_activate_cb (PidginConversation * gtkconv) {
    on_screenshot_insert_as_link_aux (NULL, gtkconv);
}
*/

static void
on_screenshot_insert_as_link_show_cb (GtkWidget * as_link,
                                      PidginConversation * gtkconv) {
    PurpleConversation *conv = gtkconv->active_conv;

    /*
     * Depending on which protocol the conv is associated with,
     * html is supported or not...
     */
#if GTK_CHECK_VERSION(2,16,0)
    if (purple_conversation_get_features (conv) & PURPLE_CONNECTION_NO_IMAGES)
        gtk_menu_item_set_label (GTK_MENU_ITEM (as_link),
                                 SEND_AS_HTML_LINK_TXT);
    else
        gtk_menu_item_set_label (GTK_MENU_ITEM (as_link),
                                 SEND_AS_HTML_URL_TXT);
#else
    if (purple_conversation_get_features (conv) & PURPLE_CONNECTION_NO_IMAGES)
        gtk_label_set_label (GTK_LABEL (GTK_BIN (as_link)->child),
                             SEND_AS_HTML_LINK_TXT);
    else
        gtk_label_set_label (GTK_LABEL (GTK_BIN (as_link)->child),
                             SEND_AS_HTML_URL_TXT);
#endif
}


//static void
//on_screenshot_insert_as_ftp_link_aux 

static void
send_as_ftp_link (PidginWindow * win,
                                      PidginConversation * _gtkconv) {
    PurplePlugin *plugin;


    plugin = purple_plugins_find_with_id (PLUGIN_ID);
    if (PLUGIN (locked))
        return;                 /* Just return, don't fail. */
    else {
        PidginConversation *gtkconv;
        PLUGIN (locked) = TRUE;

        if (win != NULL)
            gtkconv =
                PIDGIN_CONVERSATION
                (pidgin_conv_window_get_active_conversation (win));
        else
            gtkconv = _gtkconv;

        REMEMBER_ACCOUNT (gtkconv);

        PLUGIN (send_as) = SEND_AS_FTP_LINK;

        PLUGIN (conv_features) = gtkconv->active_conv->features;
        freeze_desktop (plugin, FALSE);
    }
}

/*

static void
on_screenshot_insert_as_ftp_link_fromwin_activate_cb (PidginWindow * win) {
    on_screenshot_insert_as_ftp_link_aux (win, NULL);
}


static void
on_screenshot_insert_as_ftp_link_activate_cb (PidginConversation * gtkconv) {
    on_screenshot_insert_as_ftp_link_aux (NULL, gtkconv);
}
*/

static void
on_screenshot_insert_as_ftp_link_show_cb (GtkWidget *
                                          as_link,
                                          PidginConversation * gtkconv) {
    PurpleConversation *conv = gtkconv->active_conv;

    /*
     * Depending on which protocol the conv is associated with,
     * html is supported or not...
     */
#if GTK_CHECK_VERSION(2,16,0)
    if (purple_conversation_get_features (conv) & PURPLE_CONNECTION_NO_IMAGES)
        gtk_menu_item_set_label (GTK_MENU_ITEM (as_link),
                                 SEND_AS_FTP_LINK_TXT);
    else
        gtk_menu_item_set_label (GTK_MENU_ITEM (as_link),
                                 SEND_AS_FTP_URL_TXT);
#else
    if (purple_conversation_get_features (conv) & PURPLE_CONNECTION_NO_IMAGES)
        gtk_label_set_label (GTK_LABEL (GTK_BIN (as_link)->child),
                             SEND_AS_FTP_LINK_TXT);
    else
        gtk_label_set_label (GTK_LABEL (GTK_BIN (as_link)->child),
                             SEND_AS_FTP_URL_TXT);
#endif
}
#endif /* ENABLE_UPLOAD */

//static void
//on_screenshot_insert_as_image_aux

static void
send_as_image (PidginWindow * win,
               PidginConversation * _gtkconv) {
    PurplePlugin *plugin = purple_plugins_find_with_id (PLUGIN_ID);

    if (PLUGIN (locked))
        return;                 /* Just return, don't fail. */
    else {
        PidginConversation *gtkconv;

        PLUGIN (locked) = TRUE;
        PLUGIN (send_as) = SEND_AS_IMAGE;

        if (win != NULL)
            gtkconv =
                PIDGIN_CONVERSATION
                (pidgin_conv_window_get_active_conversation (win));
        else
            gtkconv = _gtkconv;


        REMEMBER_ACCOUNT (gtkconv);

        PLUGIN (conv_features) = gtkconv->active_conv->features;
        freeze_desktop (plugin, FALSE);
    }
}


/*
static void
on_screenshot_insert_as_image_fromwin_activate_cb (PidginWindow * win) {
    on_screenshot_insert_as_image_aux (win, NULL);
}

static void
on_screenshot_insert_as_image_activate_cb (PidginConversation * gtkconv) {
    on_screenshot_insert_as_image_aux (NULL, gtkconv);
}
*/

/*
static void
on_screenshot_insert_as_image_show_cb (GtkWidget *
                                       as_image,
                                       PidginConversation * gtkconv) {
    PurpleConversation *conv = gtkconv->active_conv;

   
    gtk_widget_set_sensitive (as_image,
                              !(purple_conversation_get_features (conv) &
                                PURPLE_CONNECTION_NO_IMAGES));
}
*/

/**
 * Handle hiding and showing stuff based on what type of conv this is...
 */
static void
on_conversation_menu_show_cb (PidginWindow * win) {
    PurpleConversation *conv;
    GtkWidget *conversation_menu, *img_menuitem, *screenshot_menuitem;
#ifdef ENABLE_UPLOAD
    GtkWidget *link_menuitem, *ftp_link_menuitem;
#endif

    PurplePlugin *plugin = purple_plugins_find_with_id (PLUGIN_ID);

    conv = pidgin_conv_window_get_active_conversation (win);
    conversation_menu =
        gtk_item_factory_get_widget (win->menu.item_factory,
                                     N_("/Conversation"));
#ifdef ENABLE_UPLOAD
    link_menuitem =
        g_object_get_data (G_OBJECT (conversation_menu), "link_menuitem");
    ftp_link_menuitem =
        g_object_get_data (G_OBJECT (conversation_menu), "ftp_link_menuitem");
    on_screenshot_insert_as_link_show_cb (link_menuitem,
                                          PIDGIN_CONVERSATION (conv));
    on_screenshot_insert_as_ftp_link_show_cb (ftp_link_menuitem,
                                              PIDGIN_CONVERSATION (conv));
#endif

    img_menuitem =
        g_object_get_data (G_OBJECT (conversation_menu), "img_menuitem");
    
    //  on_screenshot_insert_as_image_show_cb (img_menuitem,
    //                                     PIDGIN_CONVERSATION (conv));

    screenshot_menuitem =
        g_object_get_data (G_OBJECT (conversation_menu),
                           "screenshot_menuitem");

    gtk_widget_set_sensitive (screenshot_menuitem, !PLUGIN (locked));
}

/**
   Create a submenu with send as Image, Link (Http) and Link (Ftp) menuitems.
 */
/*
static GtkWidget *
create_plugin_submenu (PidginConversation * gtkconv, gboolean multiconv) {
    GtkWidget *submenu;
    GtkWidget *as_image;

#ifdef ENABLE_UPLOAD
    GtkWidget *as_link, *as_ftp_link;
#endif

    submenu = gtk_menu_new ();

    as_image = gtk_menu_item_new_with_mnemonic (SEND_AS_IMAGE_TXT);

#ifdef ENABLE_UPLOAD
    as_link = gtk_menu_item_new_with_mnemonic (SEND_AS_HTML_LINK_TXT);
    as_ftp_link = gtk_menu_item_new_with_mnemonic (SEND_AS_FTP_LINK_TXT);
    gtk_menu_shell_insert (GTK_MENU_SHELL (submenu), as_ftp_link, 0);
    gtk_menu_shell_insert (GTK_MENU_SHELL (submenu), as_link, 1);
    gtk_menu_shell_insert (GTK_MENU_SHELL (submenu), as_image, 2);
#else
    gtk_menu_shell_insert (GTK_MENU_SHELL (submenu), as_image, 0);
#endif

    if (multiconv) {
        PidginWindow *win = pidgin_conv_get_window (gtkconv);
        GtkWidget *conversation_menu =
            gtk_item_factory_get_widget (win->menu.item_factory,
                                         N_("/Conversation"));

        g_signal_connect_swapped (G_OBJECT (conversation_menu), "show",
                                  G_CALLBACK (on_conversation_menu_show_cb),
                                  win);

        g_signal_connect_swapped (G_OBJECT (as_image),
                                  "activate",
                                  G_CALLBACK
                                  (on_screenshot_insert_as_image_fromwin_activate_cb),
                                  win);
        g_object_set_data (G_OBJECT (conversation_menu),
                           "img_menuitem", as_image);

#ifdef ENABLE_UPLOAD
        g_signal_connect_swapped (G_OBJECT (as_link), "activate",
                                  G_CALLBACK
                                  (on_screenshot_insert_as_link_fromwin_activate_cb),
                                  win);

        g_signal_connect_swapped (G_OBJECT (as_ftp_link), "activate",
                                  G_CALLBACK
                                  (on_screenshot_insert_as_ftp_link_fromwin_activate_cb),
                                  win);

        g_object_set_data (G_OBJECT (conversation_menu),
                           "link_menuitem", as_link);
        g_object_set_data (G_OBJECT (conversation_menu),
                           "ftp_link_menuitem", as_ftp_link);
#endif
    }
    else {
        g_signal_connect (G_OBJECT (as_image), "show",
                          G_CALLBACK
                          (on_screenshot_insert_as_image_show_cb), gtkconv);

        g_signal_connect_swapped (G_OBJECT (as_image),
                                  "activate",
                                  G_CALLBACK
                                  (on_screenshot_insert_as_image_activate_cb),
                                  gtkconv);
#ifdef ENABLE_UPLOAD
        g_signal_connect (G_OBJECT (as_link), "show",
                          G_CALLBACK
                          (on_screenshot_insert_as_link_show_cb), gtkconv);
        g_signal_connect_swapped (G_OBJECT (as_link), "activate",
                                  G_CALLBACK
                                  (on_screenshot_insert_as_link_activate_cb),
                                  gtkconv);
        g_signal_connect_swapped (G_OBJECT (as_ftp_link), "activate",
                                  G_CALLBACK
                                  (on_screenshot_insert_as_ftp_link_activate_cb),
                                  gtkconv);
        g_signal_connect (G_OBJECT (as_ftp_link), "show",
                          G_CALLBACK
                          (on_screenshot_insert_as_ftp_link_show_cb),
                          gtkconv);
#endif
    }
    return submenu;
}
*/


/**
   If the plugin is locked, don't activate the menuitem.
   This tipically happens when we're sending the capture to a remote server.
 */
static void
on_insert_menu_show_cb (GtkWidget * screenshot_insert_menuitem) {
    PurplePlugin *plugin = purple_plugins_find_with_id (PLUGIN_ID);
    gtk_widget_set_sensitive (screenshot_insert_menuitem, !PLUGIN (locked));
}


/**
   The user wants to insert a sreenshot.
 */
static void
on_screenshot_insert_menuitem_activate_cb (GtkWidget * screenshot_insert_menuitem,
					   PidginConversation *gtkconv) {
    PurplePlugin *plugin = purple_plugins_find_with_id (PLUGIN_ID);
   
  
    if (!strcmp (purple_prefs_get_string (PREF_SEND_TYPE), "img-ftp-http")) {
      PurpleConversation *conv = gtkconv->active_conv;
      
      /* Direct IM of image is allowed by current protocol */
      if (!(purple_conversation_get_features (conv) & PURPLE_CONNECTION_NO_IMAGES)) {
	    send_as_image (NULL, gtkconv);
      } else if (strcmp(purple_prefs_get_string (PREF_FTP_REMOTE_URL), "") && 
		 strcmp(purple_prefs_get_string (PREF_FTP_USERNAME), "") && 
		 strcmp(purple_prefs_get_string (PREF_FTP_PASSWORD), "")) {
	    send_as_ftp_link (NULL, gtkconv);
      } else if (strcmp (purple_prefs_get_string (PREF_UPLOAD_TO), HOST_DISABLED)) {
	    send_as_link (NULL, gtkconv);
      } else {
	    purple_notify_error (plugin, PLUGIN_NAME, PLUGIN_ERROR,
                                 PLUGIN_CONFIGURE_ERROR);
            plugin_stop (plugin);
      }
    }
}


static gboolean
catch_hotkeys_cb (PidginWindow * win, GdkEventKey * event) {
    if (event->is_modifier == FALSE) {
        gint all_modifiers = 0;
        PurpleConversation *conv = NULL;

        if (event->state & GDK_SHIFT_MASK)
            all_modifiers |= GDK_SHIFT_MASK;
        if (event->state & GDK_CONTROL_MASK)
            all_modifiers |= GDK_CONTROL_MASK;
        if (event->state & GDK_MOD1_MASK)
            all_modifiers |= GDK_MOD1_MASK;

        conv = pidgin_conv_window_get_active_conversation (win);

        if (gdk_keyval_to_lower (event->keyval) ==
            (guint) purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_FILE)
            &&
            all_modifiers ==
            purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_FILE_MDFS))
        {

            PurpleConnection *gc = NULL;
            PurplePluginProtocolInfo *prpl_info = NULL;

            gc = purple_conversation_get_gc (conv);

            if ((gc != NULL) &&
                ((purple_conversation_get_type (conv) !=
                  PURPLE_CONV_TYPE_CHAT)
                 || !purple_conv_chat_has_left (PURPLE_CONV_CHAT (conv)))) {

                prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO (gc->prpl);
                if (purple_conversation_get_type (conv) ==
                    PURPLE_CONV_TYPE_IM
                    && prpl_info->send_file != NULL
                    && (!prpl_info->can_receive_file
                        || prpl_info->can_receive_file (gc,
                                                        purple_conversation_get_name
                                                        (conv)))) {
                    PurplePlugin *plugin =
                        purple_plugins_find_with_id (PLUGIN_ID);

                    if (PLUGIN (locked))
                        return FALSE;   /* Just return, don't fail. */
                    else {
                        PLUGIN (locked) = TRUE;
                        PLUGIN (send_as) = SEND_AS_FILE;
                        REMEMBER_ACCOUNT (PIDGIN_CONVERSATION (conv));
                        freeze_desktop (plugin, FALSE);
                    }
                }
            }
        }
#ifdef ENABLE_UPLOAD
        else if (gdk_keyval_to_lower (event->keyval) ==
                 (guint) purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_FTP)
                 &&
                 all_modifiers ==
                 purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_FTP_MDFS)
            )
	  send_as_ftp_link(win, NULL);
	  //on_screenshot_insert_as_ftp_link_fromwin_activate_cb (win);
        else if (gdk_keyval_to_lower (event->keyval) ==
                 (guint) purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_HTTP)
                 &&
                 all_modifiers ==
                 purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_HTTP_MDFS)
            )
	  send_as_link(win, NULL);
	  //on_screenshot_insert_as_link_fromwin_activate_cb (win);
#endif
        else if (gdk_keyval_to_lower (event->keyval) == (guint)
                 purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_IMAGE)
                 &&
                 all_modifiers ==
                 purple_prefs_get_int (PREF_HOTKEYS_SEND_AS_IMAGE_MDFS)
		 ) {
          //  if (!(purple_conversation_get_features (conv) &
	  //      PURPLE_CONNECTION_NO_IMAGES))
	  //  on_screenshot_insert_as_image_fromwin_activate_cb (win);
        }
        else {
            /* nothing match ! */
        }
        return TRUE;
    }
    return FALSE;               /* let the signal be handled by other callbacks */
}

void
create_plugin_menuitems (PurpleConversation * conv) {
    if (PIDGIN_IS_PIDGIN_CONVERSATION (conv)) {
        PidginConversation *gtkconv;
        PurplePlugin *plugin;
        PidginWindow *win;
        GtkWidget *conversation_menu, *screenshot_menuitem;
        GtkWidget *screenshot_insert_menuitem;

        gtkconv = PIDGIN_CONVERSATION (conv);
        plugin = purple_plugins_find_with_id (PLUGIN_ID);
        win = pidgin_conv_get_window (gtkconv);

        conversation_menu =
            gtk_item_factory_get_widget (win->menu.item_factory,
                                         N_("/Conversation"));
        screenshot_insert_menuitem =
            g_object_get_data (G_OBJECT (gtkconv->toolbar),
                               "screenshot_insert_menuitem");
        screenshot_menuitem =
            g_object_get_data (G_OBJECT (conversation_menu),
                               "screenshot_menuitem");

        /* Intercept hotkeys defined in pref window */
        g_signal_connect_swapped (G_OBJECT
                                  (pidgin_conv_get_window (gtkconv)->window),
                                  "key_release_event",
                                  G_CALLBACK (catch_hotkeys_cb), win);

        /* Add us to the conv "Insert" menu */
        if (screenshot_insert_menuitem == NULL) {
	  GtkWidget *insert_menu;//, *submenu;
            if ((insert_menu =
                 g_object_get_data (G_OBJECT (gtkconv->toolbar),
                                    "insert_menu")) != NULL) {
                /* add us to the "insert" list */
                screenshot_insert_menuitem =
                    gtk_menu_item_new_with_mnemonic
                    (SCREENSHOT_INSERT_MENUITEM_LABEL);

                //submenu = create_plugin_submenu (gtkconv, FALSE);


                g_signal_connect_swapped (G_OBJECT (insert_menu), "show",
                                          G_CALLBACK (on_insert_menu_show_cb),
                                          screenshot_insert_menuitem);


		   g_signal_connect  (G_OBJECT (screenshot_insert_menuitem), "activate",
                                  G_CALLBACK (on_screenshot_insert_menuitem_activate_cb),
                                 gtkconv);

		/*
                gtk_menu_item_set_submenu (GTK_MENU_ITEM
                                           (screenshot_insert_menuitem),
                                           submenu);
		*/
                gtk_menu_shell_insert (GTK_MENU_SHELL (insert_menu), screenshot_insert_menuitem, 1);    /* 0 = Image */

                /* register new widget */
                g_object_set_data (G_OBJECT (gtkconv->toolbar),
                                   "screenshot_insert_menuitem",
                                   screenshot_insert_menuitem);
            }
        }

        /* Add us to the conv "Conversation" menu. */
        if (screenshot_menuitem == NULL) {
            GList *children = NULL, *head_chld = NULL;  /* don't g_list_free() it */
            guint i = 0;
            //GtkWidget *submenu = create_plugin_submenu (gtkconv, TRUE);

            screenshot_menuitem =
                gtk_menu_item_new_with_mnemonic (SCREENSHOT_MENUITEM_LABEL);

            /*gtk_menu_item_set_submenu (GTK_MENU_ITEM
	      (screenshot_menuitem), submenu); */

            /*gtk_widget_show_all (submenu);*/

            children =
                gtk_container_get_children (GTK_CONTAINER
                                            (conversation_menu));
            head_chld = children;       /* keep first element addr */

            /* pack our menuitem at correct place */
            while (children != NULL && children->data !=
                   (gpointer) win->menu.insert_image) {
                children = g_list_next (children);
                i++;
            }
            g_list_free (head_chld);

            gtk_menu_shell_insert (GTK_MENU_SHELL (conversation_menu),
                                   screenshot_menuitem, i + 1);
            gtk_widget_show (screenshot_menuitem);

            g_object_set_data (G_OBJECT (conversation_menu),
                               "screenshot_menuitem", screenshot_menuitem);
        }
    }
}

void
remove_pidgin_menuitems (PurpleConversation * conv) {
    if (PIDGIN_IS_PIDGIN_CONVERSATION (conv)) {
        PurplePlugin *plugin;
        GtkWidget *screenshot_insert_menuitem;
        GtkWidget *screenshot_menuitem, *conversation_menu;
        GtkWidget *insert_menu;
        PidginWindow *win;
        PidginConversation *gtkconv;

        gtkconv = PIDGIN_CONVERSATION (conv);
        plugin = purple_plugins_find_with_id (PLUGIN_ID);

        win = pidgin_conv_get_window (gtkconv);
        if (win != NULL) {
            if ((conversation_menu =
                 gtk_item_factory_get_widget (win->menu.item_factory,
                                              N_("/Conversation"))) != NULL) {
                /* remove signal */
                gulong handler = g_signal_handler_find (conversation_menu,
                                                        G_SIGNAL_MATCH_FUNC,
                                                        0,
                                                        0,
                                                        NULL,
                                                        G_CALLBACK
                                                        (on_conversation_menu_show_cb),
                                                        NULL);
                if (handler)
                    g_signal_handler_disconnect (conversation_menu, handler);

                if ((screenshot_menuitem =
                     g_object_get_data (G_OBJECT (conversation_menu),
                                        "screenshot_menuitem")) != NULL) {
                    gtk_widget_destroy (screenshot_menuitem);
                    g_object_steal_data (G_OBJECT (conversation_menu),
                                         "screenshot_menuitem");
                    g_object_steal_data (G_OBJECT (conversation_menu),
                                         "img_menuitem");
#ifdef ENABLE_UPLOAD
                    g_object_steal_data (G_OBJECT (conversation_menu),
                                         "link_menuitem");
                    g_object_steal_data (G_OBJECT (conversation_menu),
                                         "ftp_link_menuitem");
#endif
                }
            }
        }
        /* remove signal */
        if ((insert_menu =
             g_object_get_data (G_OBJECT (gtkconv->toolbar),
                                "insert_menu")) != NULL) {
            gulong handler = g_signal_handler_find (insert_menu,
                                                    G_SIGNAL_MATCH_FUNC,
                                                    0,
                                                    0,
                                                    NULL,
                                                    G_CALLBACK
                                                    (on_insert_menu_show_cb),
                                                    NULL);
            if (handler)
                g_signal_handler_disconnect (insert_menu, handler);
        }

        screenshot_insert_menuitem =
            g_object_get_data (G_OBJECT (gtkconv->toolbar),
                               "screenshot_insert_menuitem");
        if (screenshot_insert_menuitem != NULL) {
            gtk_widget_destroy (screenshot_insert_menuitem);
            g_object_steal_data (G_OBJECT (gtkconv->toolbar),
                                 "screenshot_insert_menuitem");
        }
    }
}

static void
on_blist_context_menu_send_capture (PurpleBuddy * buddy,
                                    PurplePlugin * plugin) {


    if (PLUGIN (locked))
        return;                 /* Just return, don't fail. */
    else {
        PLUGIN (locked) = TRUE;

        PLUGIN (send_as) = SEND_AS_FILE;

        PLUGIN (conv_type) = PURPLE_CONV_TYPE_UNKNOWN;  /* no conv opened */
        PLUGIN (account) = purple_buddy_get_account (buddy);
        PLUGIN (name) = g_strdup_printf ("%s", purple_buddy_get_name (buddy));

        /* see on_screenshot_insert_as_image_activate_cb () */
        freeze_desktop (plugin, FALSE);
    }
}

/* stolen from autoaccept.c and gtkblist.c files. */
void
buddy_context_menu_add_item (PurpleBlistNode * node, GList ** menu,
                             PurplePlugin * plugin) {
    PurplePluginProtocolInfo *prpl_info;

    if (PURPLE_BLIST_NODE_IS_BUDDY (node)) {
        prpl_info =
            PURPLE_PLUGIN_PROTOCOL_INFO (((PurpleBuddy *) node)->account->
                                         gc->prpl);
        if (prpl_info && prpl_info->send_file) {
            if (!prpl_info->can_receive_file ||
                prpl_info->can_receive_file (((PurpleBuddy *) node)->
                                             account->gc,
                                             ((PurpleBuddy *) node)->name)) {
                PurpleMenuAction *action;

                if (!PURPLE_BLIST_NODE_IS_BUDDY (node)
                    && !PURPLE_BLIST_NODE_IS_CONTACT (node)
                    && !(purple_blist_node_get_flags (node) &
                         PURPLE_BLIST_NODE_FLAG_NO_SAVE))
                    return;

                action =
                    purple_menu_action_new (SCREENSHOT_SEND_MENUITEM_LABEL,
                                            PURPLE_CALLBACK
                                            (on_blist_context_menu_send_capture),
                                            plugin, NULL);
                (*menu) = g_list_prepend (*menu, action);       /* add */
            }
        }
    }
}

/* end of menus.c */
