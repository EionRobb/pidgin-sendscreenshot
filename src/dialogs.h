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
  * --  Raoul Berger <contact@raoulito.info>
  *
  */

#ifndef __DIALOGS_H__
#define __DIALOGS_H__ 1

#include "main.h"

void screenshot_maybe_rename (PurplePlugin * plugin, gchar ** basename);
void show_countdown_dialog (PurplePlugin * plugin);

#define COUNTDOWN_MSG _("Please be ready ! Desktop will freeze in...")

#endif

/* end of dialogs.h */
