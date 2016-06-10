 /*
  *  Pidgin SendScreenshot plugin - common upload funcs, header -
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

#ifndef __UPLOAD_UTILS_H__
#define __UPLOAD_UTILS_H__ 1

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#ifndef ENABLE_UPLOAD
#error "***** ENABLE_UPLOAD is not defined ! *****"
#endif

#include "main.h"

#define PLUGIN_UPLOAD_PROGRESS_INTERVAL 50

#define PLUGIN_UPLOAD_DISCLOC_ERROR\
  _("Location of your screenshot on disk:")

#define PLUGIN_SENDING_INFO\
  _("Uploading to \"%s\"...")

#define PLUGIN_UPLOAD_CLOSED_CONV_ERROR\
  _("Failed to insert a link, possible reasons are:\n"\
    " - account not connected,\n"\
    " - conversation window closed.\n"\
    "\nThe link is:\n%s")

G_LOCK_EXTERN (unload);

#define THREAD_QUIT\
  PLUGIN (libcurl_thread) = NULL;\
  G_UNLOCK (unload);\
  return NULL

/* protos */
void plugin_curl_set_common_opts (CURL * curl, PurplePlugin * plugin);
void real_insert_link (PurplePlugin * plugin, const gchar * url);
GtkWidget *show_uploading_dialog (PurplePlugin * plugin, const gchar * str);

#endif /* __UPLOAD_UTILS_H__ */

/* end of upload_utils.h */
