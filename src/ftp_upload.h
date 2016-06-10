 /*
  *  Pidgin SendScreenshot plugin - ftp upload, header -
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

#ifndef __FTP_UPLOAD_H__
#define __FTP_UPLOAD_H__ 1

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#ifndef ENABLE_UPLOAD
#error "***** ENABLE_UPLOAD is not defined ! *****"
#endif

#include "main.h"

void ftp_upload_prepare (PurplePlugin * plugin);

#define PLUGIN_FTP_SENDING_INFO _("Uploading %d/%d bytes...")

#define SEND_AS_FTP_LINK_TXT _("as a link (_FTP upload)")
#define SEND_AS_FTP_URL_TXT _("as a URL (_FTP upload)")

#endif
