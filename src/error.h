 /*
  * Pidgin SendScreenshot plugin - error stuff -
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

#ifndef __ERROR_H__
#define __ERROR_H__ 1

#include <glib.h>
#include <errno.h>

#define NotifyError(strval,arg...)\
  {\
  gchar *strmsg;\
  strmsg = g_strdup_printf(strval, arg);\
    purple_notify_error(plugin, PLUGIN_NAME, PLUGIN_ERROR, strmsg);\
    g_free(strmsg);\
  }

#define NotifyUploadError(strval,arg...)				\
  {									\
  gchar *strmsg;							\
  if (need_save())							\
      strmsg = g_strdup_printf(strval"\n\n%s\n%s", arg,			\
			       PLUGIN_UPLOAD_DISCLOC_ERROR,		\
			       PLUGIN (capture_path_filename));		\
  else									\
    strmsg = g_strdup_printf(strval, arg);				\
  purple_notify_error(plugin, PLUGIN_NAME, PLUGIN_ERROR, strmsg);	\
  g_free(strmsg);							\
  }

typedef enum {
    PLUGIN_ERROR_FTP_UPLOAD,
    PLUGIN_ERROR_HTTP_UPLOAD,
    PLUGIN_ERROR_OPEN_SCREENSHOT_FILE
} PluginError;

#define PLUGIN_CONFIGURE_ERROR\
  _("Direct IM of images is not allowed with current protocol, please configure the plugin to send images remotely !")

#define PLUGIN_ERROR_FTP_UPLOAD_s\
  _("FTP upload failed:")

#define PLUGIN_ERROR_HTTP_UPLOAD_s\
  _("Failed to upload the screenshot to '%s'!\n%s")

#define PLUGIN_ERROR_OPEN_SCREENSHOT_FILE_s\
  _("Failed to open screenshot file:")

#define SENDSCREENSHOT_PLUGIN_ERROR\
  g_quark_from_static_string ("rberger-plugin-error-quark")

#endif

/* end of error.h */
