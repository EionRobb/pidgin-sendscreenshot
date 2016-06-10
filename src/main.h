/*
 * Pidgin SendScreenshot plugin - header -
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
 *
 */

#ifndef __SCREENSHOT_H__
#define __SCREENSHOT_H__ 1

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#ifdef DISABLE_LIBCURL
#undef HAVE_LIBCURL
#endif

#ifndef PURPLE_PLUGINS
#define PURPLE_PLUGINS
#endif
#include <gtkmenutray.h>
#include <glib/gi18n-lib.h>
#include <plugin.h>
#include <debug.h>
#include <pidginstock.h>
#include <version.h>
#include <gtkplugin.h>
#include <gtkconv.h>
#include <gtkimhtmltoolbar.h>
#include <string.h>             /* strrchr() */
#include <gdk/gdkkeysyms.h>     /* GDK_Escape, GDK_Down */
#include <gtkutils.h>
#include <gtkblist.h>
#include <glib/gstdio.h>

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#endif

#ifdef G_OS_WIN32
#include <win32dep.h>
#define PLUGIN_DATADIR wpurple_install_dir()
#endif

#ifndef G_MARKUP_ERROR_MISSING_ATTRIBUTE
#define G_MARKUP_ERROR_MISSING_ATTRIBUTE G_MARKUP_ERROR_PARSE
#endif

#define gettext_noop(String) String

/* various infos about this plugin */
#define PLUGIN_ID "gtk-rberger-screenshot"

#define PLUGIN_NAME	_("Send Screenshot")
#define PLUGIN_SUMMARY	_("Capture and send a screenshot.")
#define PLUGIN_DESCRIPTION _("This plugin will capture a screenshot given "\
			     "a rectangular area.")

#define PLUGIN_AUTHOR	"Raoul Berger <"PACKAGE_BUGREPORT">"
#define PLUGIN_WEBSITE 	"http://code.google.com/p/pidgin-sendscreenshot"

G_LOCK_EXTERN (unload);

#define PLUGIN_INFO _("Information")

/* error reporting strings */
#define PLUGIN_ERROR _("Error")

#define PLUGIN_ALREADY_LOCKED_ERROR _("Another instance of %s is already running.\n"\
                                       "Please wait before sending an other screenshot.")

#define PLUGIN_LOAD_DATA_ERROR _("Cannot allocate enough memory (%lu bytes) to load plugin data !")
#define PLUGIN_MEMORY_ERROR _("Failed to allocate enough memory to create the screenshot."\
			      " You will need to quit some applications and retry.")
#define PLUGIN_SAVE_TO_FILE_ERROR _("Failed to save your screenshot to '%s'.")

#define PLUGIN_GET_DATA_ERROR _("Failed to get '%s' data.")
#define PLUGIN_STORE_FAILED_ERROR _("Failed to insert your screenshot in the text area.")

#define PLUGIN_SIGNATURE_TOOBIG_ERROR _("The image used to sign the screenshot is too big.\n"\
					"%dx%d is the maximum allowed.")

#define PLUGIN_UNEXPECTED_ERROR _("An unexpected error occured, see debug window...")

/* see on_screenshot_insert_menuitem_activate_cb () */
#define MSEC_TIMEOUT_VAL 500

/* darken / lighten */
#define PIXEL_VAL 85

#define HOST_DISABLED _("No selection")
#define PIDGIN_HOST_TOS _("Terms Of Service")

#define PIXBUF_HOSTS_SIZE 32

#define SIGN_MAXWIDTH 128
#define SIGN_MAXHEIGHT 32

#define SCREENSHOT_INSERT_MENUITEM_LABEL _("_Screenshot")
#define SCREENSHOT_MENUITEM_LABEL _("Insert _Screenshot...")
#define SCREENSHOT_SEND_MENUITEM_LABEL _("Send a _Screenshot...")

#define CAPTURE _("capture")

#define SEND_AS_IMAGE_TXT _("as an _Image")

#define DLGBOX_CAPNAME_TITLE _("Set capture name")
#define DLGBOX_CAPNAME_LABEL _("Capture name:")
#define DLGBOX_CAPNAME_WARNEXISTS _("File already exists!")

/* convenient? macros */
#define PLUGIN_EXTRA(plugin)\
  ((PluginExtraVars*)(plugin->extra))

#define PLUGIN(what)\
  ((PluginExtraVars*)(plugin->extra))->what

#define REMEMBER_ACCOUNT(conv)\
  PLUGIN (conv_type) = purple_conversation_get_type (conv->active_conv);\
  PLUGIN (account) = purple_conversation_get_account (conv->active_conv);\
  PLUGIN (name) = g_strdup_printf ("%s", purple_conversation_get_name(conv->active_conv))

#ifdef ENABLE_UPLOAD
typedef enum { SEND_AS_FILE, SEND_AS_IMAGE, SEND_AS_HTTP_LINK,
    SEND_AS_FTP_LINK
} SendType;
#else
typedef enum { SEND_AS_FILE, SEND_AS_IMAGE } SendType;
#endif

typedef enum { SELECT_REGULAR, SELECT_MOVE } SelectionMode;

typedef enum {
    ResizeAny,
    ResizeNone,
    ResizeBottomLeft,
    ResizeBottomRight,
    ResizeTopLeft,
    ResizeTopRight,
    ResizeLeft,
    ResizeRight,
    ResizeTop,
    ResizeBottom,
} ResizeMode;


/* functions */
GtkWidget *get_receiver_window (PurplePlugin * plugin);
GtkIMHtml *get_receiver_imhtml (PurplePlugin * plugin);
void plugin_stop (PurplePlugin * plugin);
gboolean receiver_window_is_iconified (PurplePlugin * plugin);

#define selection_defined(plugin)\
  (PLUGIN (x1) >= 0)

/* main struct holding data */
typedef struct {
    /* prevent two instances of SndScreenshot to run
       simultenaously */
    gboolean locked;

    SendType send_as;

    /* to display frozen desktop state */
    GdkGC *gc;
    GtkWidget *root_window;
    gboolean root_exposed;
    /* used to catch events */
    GtkWidget *root_events;

    /* original image */
    GdkPixbuf *root_pixbuf_orig;
    /* modified image (highlight mode) */
    GdkPixbuf *root_pixbuf_x;

    /* where to send capture ? */
    PurpleConnectionFlags conv_features;
    PurpleConversationType conv_type;
    PurpleAccount *account;
    gchar *name;

    /* capture area */
    gint x1, y1, x2, y2;

    SelectionMode select_mode;
    ResizeMode resize_mode;
    gboolean resize_allow;

    /* cues */
    gint cue_offset;
    gint mouse_x, mouse_y, __mouse_x, __mouse_y;

    /* freeze cb handle */
    guint timeout_source;

    /* screenshot's location */
    gchar *capture_path_filename;

    GError *error;

    GtkWidget *countdown_dialog;

#ifdef ENABLE_UPLOAD
    GtkWidget *uploading_dialog;
    GThread *libcurl_thread;
    gchar *xml_hosts_filename;
    guint timeout_cb_handle;

    /* ftp stuff */
    off_t read_size;
    off_t total_size;

    /* host data from xml */
    struct host_param_data {
        gchar *xml_hosts_version;
        gchar *selected_hostname;
        gchar *form_action;
        gchar *file_input_name;
        gchar *regexp;

        gchar *html_response;

        GArray *host_names;
        GArray *extra_names;
        GArray *extra_values;

        gboolean is_inside;
        gboolean quit_handlers;

        /* needed by conf dialog */
    } *host_data;
#endif
} PluginExtraVars;

#endif /* __SCREENSHOT_H__ */

/* end of main.h */
