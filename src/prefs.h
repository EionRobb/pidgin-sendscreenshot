 /*
  *  Pidgin SendScreenshot plugin - preferences, header -
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

#ifndef __PREFS_H__
#define __PREFS_H__ 1

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <gtkpluginpref.h>
#include <gtkprefs.h>
#include "main.h"

/* xml prefs keys */
#define	PREF_PREFIX	"/plugins/gtk/" PLUGIN_ID
#define PREF_SEND_TYPE PREF_PREFIX "/send-type"
#define PREF_IMAGE_TYPE PREF_PREFIX "/image-type"
#define PREF_JPEG_QUALITY	PREF_PREFIX "/jpeg-quality"
#define PREF_PNG_COMPRESSION	PREF_PREFIX "/png-compression"
#define PREF_HIGHLIGHT_MODE PREF_PREFIX "/highlight-mode"
#define PREF_STORE_FOLDER PREF_PREFIX "/store-folder"
#define PREF_ASK_FILENAME PREF_PREFIX "/ask-filename"
#define PREF_ONLY_SAVE_WHEN PREF_PREFIX "/only-save-when"
#define PREF_SHOW_VISUAL_CUES PREF_PREFIX "/show_visualcues"
#define PREF_WAIT_BEFORE_SCREENSHOT PREF_PREFIX "/wait_time"
#define PREF_ADD_SIGNATURE PREF_PREFIX "/add-signature"
#define PREF_SIGNATURE_FILENAME PREF_PREFIX "/signature-filename"

/* #define PREF_HOTKEYS_MODIFIERS PREF_PREFIX "/hotkeys-modifiers" */


#define PREF_HOTKEYS_SEND_AS_FILE PREF_PREFIX "/hotkeys-sendas-file"
#define PREF_HOTKEYS_SEND_AS_FILE_MDFS PREF_PREFIX "/hotkeys-sendas-file-mdfs"

#define PREF_HOTKEYS_SEND_AS_IMAGE PREF_PREFIX "/hotkeys-sendas-image"
#define PREF_HOTKEYS_SEND_AS_IMAGE_MDFS PREF_PREFIX "/hotkeys-sendas-image-mdfs"

#ifdef ENABLE_UPLOAD
#define PREF_HOTKEYS_SEND_AS_FTP PREF_PREFIX "/hotkeys-sendas-ftp"
#define PREF_HOTKEYS_SEND_AS_FTP_MDFS PREF_PREFIX "/hotkeys-sendas-ftp-mdfs"
#define PREF_HOTKEYS_SEND_AS_HTTP PREF_PREFIX "/hotkeys-sendas-http"
#define PREF_HOTKEYS_SEND_AS_HTTP_MDFS PREF_PREFIX "/hotkeys-sendas-http-mdfs"
#endif

#define pref_keyval_modifier(key_combo, key)	\
  gchar *key_combo = g_strdup (key);		\
  gchar_add (&key_combo, "mdfs", "-")

#ifdef ENABLE_UPLOAD

/* http upload prefs */
#define PREF_UPLOAD_TO PREF_PREFIX "/upload-to"
#define PREF_UPLOAD_TIMEOUT PREF_PREFIX "/upload-timeout"
#define PREF_UPLOAD_CONNECTTIMEOUT PREF_PREFIX "/upload-connecttimeout"

/* ftp upload prefs */
#define PREF_FTP_REMOTE_URL PREF_PREFIX "/ftp-remote-url"
#define PREF_FTP_WEB_ADDR PREF_PREFIX "/ftp-web-address"
#define PREF_FTP_USERNAME PREF_PREFIX "/ftp-username"
#define PREF_FTP_PASSWORD PREF_PREFIX "/ftp-password"

#endif /* ENABLE_UPLOAD */


/* prefs UI strings */
#define PREF_UI_FRAME1 _("Image parameters")
#define PREF_UI_FRAME2 _("Display behaviour")
#define PREF_UI_FRAME6 _("Saving")
#define PREF_UI_FRAME7 _("Misc")

#define PREF_UI_FRAME9 _("Combo of modifiers + keys")

#define PREF_UI_FRAME3 _("HTTP upload")
#define PREF_UI_FRAME4 _("FTP upload")
#define PREF_UI_FRAME5 _("Upload options")

#define PREF_UI_IMAGE_TYPE _("Image format: ")
#define PREF_TYPE_JPG "JPEG"
#define PREF_TYPE_PNG "PNG"
#define PREF_TYPE_BMP "BMP"

#define PREF_UI_SEND_TYPE _("Sending method: ")
#define PREF_SEND_IMG_FTP_HTTP "Direct IM or FTP or HTTP"



#define PREF_JPEG_QUALITY_DEFAULT 93
#define PREF_UI_JPEG_QUALITY _("JPEG quality level:")
#define PREF_PNG_COMPRESSION_DEFAULT 9
#define PREF_UI_PNG_COMRPESSION _("PNG compression level:")

#define PREF_UI_SIGNATURE _("Always add this signature:")

#define PREF_UI_HOTKEYS_MODIFIERS _("Hold the modifier keys and press %s...")

#define PREF_UI_HOTKEYS_SEND_AS_FILE _("Send as File: ")
#define PREF_UI_HOTKEYS_SEND_AS_IMAGE _("Send as Image: ")
#ifdef ENABLE_UPLOAD
#define PREF_UI_HOTKEYS_SEND_AS_FTP _("Send as FTP link: ")
#define PREF_UI_HOTKEYS_SEND_AS_HTTP _("Send as HTTP link: ")
#endif

#define PREF_UI_HIGHLIGHT_MODE _("Highlight mode:")
#define PREF_HIGHLIGHT_MODE_01 _("lighten up desktop")
#define PREF_HIGHLIGHT_MODE_02 _("darken desktop")
#define PREF_HIGHLIGHT_MODE_03 _("invert selection only")
#define PREF_HIGHLIGHT_MODE_04 _("borders only")
#define PREF_HIGHLIGHT_MODE_05 _("grayscale desktop")

#define PREF_UI_STORE_FOLDER _("Folder to store captures in:")

#define PREF_UI_ASK_FILENAME _("Always ask for filename when sending as a file.")
#define PREF_UI_ONLY_SAVE_WHEN _("Only save when sending as a File.")

#define PREF_UI_SHOW_VISUAL_CUES _("Show visual cues.")

#define PREF_UI_WAIT_BEFORE_SCREENSHOT _("Seconds to wait before desktop freezes:")


#define PREFS_TAB1 _("General")
#define PREFS_TAB2 _("Hotkeys")

#ifdef ENABLE_UPLOAD
#define PREFS_TAB3 _("Upload")

#define PREF_UI_UPLOAD_TO _("Pick an image hosting provider "\
			    "(<span foreground='blue'><u>list v.%s</u></span>)")
#define PREF_UI_UPLOAD_CONNECTTIMEOUT _("Connect timeout (sec): ")
#define PREF_UI_UPLOAD_TIMEOUT _("Upload timeout (sec): ")
#define PREF_UI_FTP_REMOTE_URL _("Remote FTP URL:")
#define PREF_UI_FTP_WEB_ADDR _("Corresponding Web address:")
#define PREF_UI_FTP_USERNAME _("User:")
#define PREF_UI_FTP_PASSWORD _("Password:")


#define STR_INVALID_KEY _("invalid...")

#endif /* ENABLE_UPLOAD */

GtkWidget *get_plugin_pref_frame (PurplePlugin * plugin);

/* should we save or discard the screenshot file ? */
#define need_save()\
  ((purple_prefs_get_bool (PREF_ONLY_SAVE_WHEN) && PLUGIN (send_as) == SEND_AS_FILE) || \
   !purple_prefs_get_bool (PREF_ONLY_SAVE_WHEN))

#endif

/* end of prefs.h */
