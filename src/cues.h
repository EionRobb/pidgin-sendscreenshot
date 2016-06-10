
/*
  *  Pidgin SendScreenshot plugin - cues funcs, header -
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

#ifndef __CUES_H__
#define __CUES_H__ 1

#define CUE_LENGTH 16

#include "main.h"

void draw_cues (gboolean double_buff, PurplePlugin * plugin);

void erase_cues (PurplePlugin * plugin);

#endif /* __CUES_H__ */

/* end of cues.h */
