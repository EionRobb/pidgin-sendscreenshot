 /*
  * Pidgin SendScreenshot third-party plugin - root window capture, callbacks.
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

#ifndef __SCREENCAP_H__
#define __SCREENCAP_H__ 1

#include "main.h"

/* selection border width :
   should we add an option to modify ? */
#define BORDER_WIDTH 1

typedef enum {
    LighenUp = 1,
    Darken = 2,
    InvertOnly = 3,
    BordersOnly = 4,
    Grayscale = 5
} HighlightMode;

/* Give back focus to Pidgin. */
#define THAW_DESKTOP()					\
  gtk_widget_hide (PLUGIN (root_events));		\
  gtk_widget_hide (PLUGIN (root_window));		\
  PLUGIN (root_exposed) = FALSE;

#define CAPTURE_X0(plugin)\
  MIN(PLUGIN (x1), PLUGIN (x2))
#define CAPTURE_Y0(plugin)\
  MIN(PLUGIN (y1), PLUGIN (y2))
#define CAPTURE_WIDTH(plugin)\
  ABS(PLUGIN (x2) - PLUGIN (x1)) + 1
#define CAPTURE_HEIGHT(plugin)\
  ABS(PLUGIN (y2) - PLUGIN (y1)) + 1

/* background = not (roi) */
#define BACKGROUND_PIXBUF\
  PLUGIN (root_pixbuf_x) != NULL ?  PLUGIN (root_pixbuf_x) : PLUGIN (root_pixbuf_orig)

#define RESET_SELECTION(plugin)\
  PLUGIN (x1) = -1;\
  PLUGIN (y1) = -1;\
  PLUGIN (x2) = -1;\
  PLUGIN (y2) = -1;\
  PLUGIN (resize_mode) = ResizeAny;\
  PLUGIN (resize_allow) = FALSE;\
  PLUGIN (select_mode) = SELECT_REGULAR

#define FORCE_CANCEL_MSG _("An other program has requested to use the display, canceling...")

/* prototypes */
void freeze_desktop (PurplePlugin * plugin, gboolean ignore_delay);
void prepare_root_window (PurplePlugin * plugin);
guint timeout_freeze_screen (PurplePlugin * plugin);

#endif /* __SCREENCAP_H__ */

/* end of screencap.h */
