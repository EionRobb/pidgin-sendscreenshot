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
  * --  Raoul Berger <contact@raoulito.info>
  *
  */

#ifndef __MENUS_H__
#define __MENUS_H__ 1

#include "main.h"

void create_plugin_menuitems (PurpleConversation * conv);

void remove_pidgin_menuitems (PurpleConversation * conv);

void buddy_context_menu_add_item (PurpleBlistNode * node,
                                  GList ** menu, PurplePlugin * plugin);
#endif

/* end of menus.h */
