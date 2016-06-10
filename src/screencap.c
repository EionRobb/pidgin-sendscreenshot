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

#include "screencap.h"

#include "prefs.h"
#include "pixbuf_utils.h"
#include "dialogs.h"
#include "error.h"
#include "cues.h"

#ifdef ENABLE_UPLOAD
#include "upload_utils.h"
#include "http_upload.h"
#include "ftp_upload.h"
#endif

#ifdef G_OS_WIN32
#include "mswin-freeze.h"
#endif

/*
 * If we immediately freeze the screen, then the menuitem we just
 * click on may remain. That's we wait for a small timeout.
 */
void
freeze_desktop (PurplePlugin * plugin, gboolean ignore_delay) {
    int delay = 0;
    g_assert (plugin != NULL && plugin->extra != NULL);
    g_assert (selection_defined (plugin) == FALSE);

    if ( ! ignore_delay &&
	purple_prefs_get_int (PREF_WAIT_BEFORE_SCREENSHOT) > 0)
        show_countdown_dialog (plugin);

    if ( ! ignore_delay)
      delay = purple_prefs_get_int (PREF_WAIT_BEFORE_SCREENSHOT);

    g_assert (PLUGIN (timeout_source) == 0);
    
    PLUGIN (timeout_source) =
        purple_timeout_add
        (MAX (MSEC_TIMEOUT_VAL, delay * 1000),
         (GSourceFunc) timeout_freeze_screen, plugin);
}

/**
 * Get, copy and show current desktop image.
 */
guint
timeout_freeze_screen (PurplePlugin * plugin) {
    GdkWindow *gdkroot;
    GdkCursor *cursor;
    gint x_orig, y_orig, width, height;

#ifdef G_OS_WIN32
    GdkScreen *screen;
    GdkRectangle rect;
    gint i;

    g_assert (plugin != NULL && plugin->extra != NULL);

    /* Under Windows, the primary monitor origin is always (0;0),
     * so other monitor might have negative coordinates.
     * If so, calling gdk_drawable_get_image() from gdk_get_default_root_window ()
     * will fail to retrieve all the pixels at negative coordinates.
     * We fallback using Window API...
     */
    gint x_offset, y_offset;

    screen = gdk_screen_get_default ();
    gdk_screen_get_monitor_geometry (screen, 0, &rect);

    x_offset = -rect.x;
    y_offset = -rect.y;

    rect.x += x_offset;
    rect.y += y_offset;

    for (i = 1; i < gdk_screen_get_n_monitors (screen); i++) {
        GdkRectangle monitor_rect;
        gdk_screen_get_monitor_geometry (screen, i, &monitor_rect);
        monitor_rect.x += x_offset;
        monitor_rect.y += y_offset;
        gdk_rectangle_union (&rect, &monitor_rect, &rect);
    }
    x_orig = rect.x;
    y_orig = rect.y;
    width = rect.width;
    height = rect.height;

    /* primary monitor not at the top-left corner, use Win32 API. */
    if (rect.x != 0 || rect.y != 0) {
        if (mswin_freeze_screen (plugin, rect) == FALSE) {
            purple_notify_error (plugin, PLUGIN_NAME, PLUGIN_ERROR,
                                 PLUGIN_MEMORY_ERROR);
            plugin_stop (plugin);
            return 0;
        }
    }
    else {
#endif
        /* vvvvvv
           Part stolen from gnome-screenshot code, part
           of GnomeUtils ( See http://live.gnome.org/GnomeUtils)
         */
        gint x_real_orig, y_real_orig;
        gint real_width, real_height;

        gdkroot = gdk_get_default_root_window ();

        gdk_drawable_get_size (gdkroot, &real_width, &real_height);
        gdk_window_get_origin (gdkroot, &x_real_orig, &y_real_orig);

        x_orig = x_real_orig;
        y_orig = y_real_orig;
        width = real_width;
        height = real_height;

        if (x_orig < 0) {
            width = width + x_orig;
            x_orig = 0;
        }

        if (y_orig < 0) {
            height = height + y_orig;
            y_orig = 0;
        }

        if (x_orig + width > gdk_screen_width ())
            width = gdk_screen_width () - x_orig;

        if (y_orig + height > gdk_screen_height ())
            height = gdk_screen_height () - y_orig;
        /* ^^^^ */

        g_assert (PLUGIN (root_pixbuf_orig) == NULL);
        if ((PLUGIN (root_pixbuf_orig) =
             gdk_pixbuf_get_from_drawable (NULL, gdkroot, NULL,
                                           x_orig, y_orig, 0, 0, width,
                                           height)) == NULL) {
            purple_notify_error (plugin, PLUGIN_NAME, PLUGIN_ERROR,
                                 PLUGIN_MEMORY_ERROR);
            plugin_stop (plugin);
            return 0;
        }
#ifdef G_OS_WIN32
    }
#endif

    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) < 3 ||
        purple_prefs_get_int (PREF_HIGHLIGHT_MODE) == Grayscale) {
        g_assert (PLUGIN (root_pixbuf_x) == NULL);
        g_assert (PLUGIN (root_pixbuf_orig) != NULL);
        if ((PLUGIN (root_pixbuf_x) =
             gdk_pixbuf_copy (PLUGIN (root_pixbuf_orig))) == NULL) {
            purple_notify_error (plugin, PLUGIN_NAME, PLUGIN_ERROR,
                                 PLUGIN_MEMORY_ERROR);
            g_object_unref (PLUGIN (root_pixbuf_orig));
            PLUGIN (root_pixbuf_orig) = NULL;
            plugin_stop (plugin);
            return 0;
        }
    }
    /* apply effects */
    if (PLUGIN (root_pixbuf_x) != NULL) {
        if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) == LighenUp)
            mygdk_pixbuf_lighten (PLUGIN (root_pixbuf_x), PIXEL_VAL);
        else if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) == Darken)
            mygdk_pixbuf_darken (PLUGIN (root_pixbuf_x), PIXEL_VAL);
        else if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) == Grayscale)
            mygdk_pixbuf_grey (PLUGIN (root_pixbuf_x));
    }

    /* reset */
    RESET_SELECTION (plugin);

    PLUGIN (timeout_source) = 0;

    /* let the user capture an area... */
    gtk_widget_show (PLUGIN (root_events));     /* focus is grabbed */
    gtk_widget_show (PLUGIN (root_window));

    cursor = gdk_cursor_new (GDK_CROSSHAIR);
    gdk_window_set_cursor (PLUGIN (root_window)->window, cursor);
    gdk_cursor_unref (cursor);

    return 0;
}

static void
paint_region (GdkRegion * region,
              GdkWindow * gdkwin, GdkPixbuf * pixbuf, PurplePlugin * plugin) {
    GdkRectangle *rectangles = NULL;
    gint n_rectangles, idx;

    gdk_region_get_rectangles (region, &rectangles, &n_rectangles);

    for (idx = 0; idx < n_rectangles; idx++) {
        gdk_draw_pixbuf (gdkwin, PLUGIN (gc), pixbuf,   /* src */
                         (rectangles[idx]).x, (rectangles[idx]).y,
                         (rectangles[idx]).x, (rectangles[idx]).y,
                         (rectangles[idx]).width, (rectangles[idx]).height,
                         GDK_RGB_DITHER_NONE, 0, 0);
    }
    g_free (rectangles);
}

static void
on_root_window_realize_cb (PurplePlugin * plugin) {
    GdkWindow *gdkwin;
    GtkWidget *root_window = PLUGIN (root_window);


#if GTK_CHECK_VERSION(2,14,0)
    gdkwin = gtk_widget_get_window (root_window);
#else
    gdkwin = root_window->window;
#endif
    /* be sensitive to user interaction  */
    gdk_window_set_events (gdkwin, GDK_EXPOSURE_MASK |
                           GDK_BUTTON_PRESS_MASK |
                           GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    gdk_window_set_back_pixmap (gdkwin, NULL, FALSE);
}


/* cancel whole screenshot process and come back to Pidgin */
static void
plugin_cancel (PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);

    if (receiver_window_is_iconified (plugin))
        gtk_window_deiconify (GTK_WINDOW (get_receiver_window (plugin)));

    THAW_DESKTOP ();
    plugin_stop (plugin);
}

#define DETECT_THRESHOLD 7

static void
maybe_change_cursor (gint x, gint y, PurplePlugin * plugin) {
    GdkWindow *gdkwin;
    GdkCursor *cursor;

    gint x1, x2, y1, y2;

    g_assert (plugin != NULL && plugin->extra != NULL);

    x1 = CAPTURE_X0 (plugin);
    y1 = CAPTURE_Y0 (plugin);

    x2 = x1 + CAPTURE_WIDTH (plugin) - 1;
    y2 = y1 + CAPTURE_HEIGHT (plugin) - 1;

    gdkwin = gtk_widget_get_window (PLUGIN (root_window));


    PLUGIN (select_mode) = SELECT_REGULAR;

    /* top left */
    if (ABS (y - y1) < DETECT_THRESHOLD && ABS (x - x1) < DETECT_THRESHOLD) {
        cursor = gdk_cursor_new (GDK_TOP_LEFT_CORNER);
        PLUGIN (resize_mode) = ResizeTopLeft;
    }
    /* bottom right */
    else if (ABS (y - y2) < DETECT_THRESHOLD
             && ABS (x - x2) < DETECT_THRESHOLD) {
        cursor = gdk_cursor_new (GDK_BOTTOM_RIGHT_CORNER);
        PLUGIN (resize_mode) = ResizeBottomRight;
    }
    /* bottom left */
    else if (ABS (y - y2) < DETECT_THRESHOLD
             && ABS (x - x1) < DETECT_THRESHOLD) {
        cursor = gdk_cursor_new (GDK_BOTTOM_LEFT_CORNER);
        PLUGIN (resize_mode) = ResizeBottomLeft;
    }
    /* top right */
    else if (ABS (y - y1) < DETECT_THRESHOLD
             && ABS (x - x2) < DETECT_THRESHOLD) {
        cursor = gdk_cursor_new (GDK_TOP_RIGHT_CORNER);
        PLUGIN (resize_mode) = ResizeTopRight;
    }
    /* left */
    else if (ABS (x - x1) < DETECT_THRESHOLD && y >= y1 && y <= y2) {
        cursor = gdk_cursor_new (GDK_LEFT_SIDE);
        PLUGIN (resize_mode) = ResizeLeft;
    }
    /* right */
    else if (ABS (x - x2) < DETECT_THRESHOLD && y >= y1 && y <= y2) {
        cursor = gdk_cursor_new (GDK_RIGHT_SIDE);
        PLUGIN (resize_mode) = ResizeRight;
    }
    /* top */
    else if (ABS (y - y1) < DETECT_THRESHOLD && x >= x1 && x <= x2) {
        cursor = gdk_cursor_new (GDK_TOP_SIDE);
        PLUGIN (resize_mode) = ResizeTop;
    }
    /* bottom */
    else if (ABS (y - y2) < DETECT_THRESHOLD && x >= x1 && x <= x2) {
        cursor = gdk_cursor_new (GDK_BOTTOM_SIDE);
        PLUGIN (resize_mode) = ResizeBottom;
    }
    /* inside */
    else if (x > x1 && x < x2 && y > y1 && y < y2) {
        cursor = gdk_cursor_new (GDK_FLEUR);
        PLUGIN (resize_mode) = ResizeAny;
        PLUGIN (select_mode) = SELECT_MOVE;
    }
    else {
        cursor = gdk_cursor_new (GDK_LEFT_PTR);
        PLUGIN (resize_mode) = ResizeNone;
    }
    gdk_window_set_cursor (gdkwin, cursor);
    gdk_cursor_unref (cursor);
}

static void
draw_selection (gint _x1, gint _x2, gint _y1, gint _y2, PurplePlugin * plugin) {

    GdkWindow *gdkwin;
    GdkRectangle old_r, new_r;
    /* regions to paint into */
    GdkRegion *border_inter = NULL;
    GdkRegion
        * border_new = NULL,
        *border_old = NULL, *new_region = NULL, *old_region = NULL;

#if GTK_CHECK_VERSION(2,14,0)
    gdkwin = gtk_widget_get_window (PLUGIN (root_window));
#else
    gdkwin = PLUGIN (root_window)->window;
#endif

    old_r.x = MIN (_x1, _x2);
    old_r.y = MIN (_y1, _y2);
    old_r.width = ABS (_x1 - _x2) + 1;
    old_r.height = ABS (_y1 - _y2) + 1;

    new_r.width = CAPTURE_WIDTH (plugin);
    new_r.height = CAPTURE_HEIGHT (plugin);
    new_r.x = CAPTURE_X0 (plugin);
    new_r.y = CAPTURE_Y0 (plugin);

    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) != InvertOnly) {
        old_region = gdk_region_rectangle (&old_r);
        new_region = gdk_region_rectangle (&new_r);

        border_old = gdk_region_rectangle (&old_r);
        border_new = gdk_region_rectangle (&new_r);

        gdk_region_shrink (old_region, BORDER_WIDTH, BORDER_WIDTH);
        gdk_region_shrink (new_region, BORDER_WIDTH, BORDER_WIDTH);

        gdk_region_subtract (border_old, old_region);
        gdk_region_subtract (border_new, new_region);

        gdk_region_destroy (old_region);
        gdk_region_destroy (new_region);

        old_region = NULL;
        new_region = NULL;

        border_inter = gdk_region_copy (border_old);
        gdk_region_intersect (border_inter, border_new);
        gdk_region_subtract (border_old, border_inter);
        gdk_region_subtract (border_new, border_inter);
    }

    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) != BordersOnly) {
        GdkRegion *inter;

        old_region = gdk_region_rectangle (&old_r);
        new_region = gdk_region_rectangle (&new_r);

        inter = gdk_region_copy (old_region);
        gdk_region_intersect (inter, new_region);

        gdk_region_subtract (old_region, inter);
        gdk_region_subtract (new_region, inter);
        gdk_region_destroy (inter);
    }

    /* remove old borders */
    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) != InvertOnly)
        paint_region (border_old, gdkwin, PLUGIN (root_pixbuf_orig), plugin);

    /* clear old selection */
    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) == InvertOnly)
        paint_region (old_region, gdkwin, PLUGIN (root_pixbuf_orig), plugin);
    else if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) != BordersOnly)
        paint_region (old_region, gdkwin, PLUGIN (root_pixbuf_x), plugin);

    /* draw selection */
    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) == InvertOnly) {
        gdk_gc_set_function (PLUGIN (gc), GDK_COPY_INVERT);
        paint_region (new_region, gdkwin, PLUGIN (root_pixbuf_orig), plugin);
        gdk_gc_set_function (PLUGIN (gc), GDK_COPY);

    }
    else if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) != BordersOnly)
        paint_region (new_region, gdkwin, PLUGIN (root_pixbuf_orig), plugin);

    /* add selection borders */
    if (purple_prefs_get_int (PREF_HIGHLIGHT_MODE) != InvertOnly) {
        gdk_gc_set_function (PLUGIN (gc), GDK_COPY_INVERT);
        paint_region (border_new, gdkwin, PLUGIN (root_pixbuf_orig), plugin);
        gdk_gc_set_function (PLUGIN (gc), GDK_COPY);
    }

    if (old_region != NULL) {
        gdk_region_destroy (old_region);
        old_region = NULL;
    }
    if (border_old != NULL) {
        gdk_region_destroy (border_old);
        border_old = NULL;
    }
    if (new_region != NULL) {
        gdk_region_destroy (new_region);
        new_region = NULL;
    }
    if (border_new != NULL) {
        gdk_region_destroy (border_new);
        border_new = NULL;
    }

    if (border_inter != NULL)
        gdk_region_destroy (border_inter);

}

static gboolean
on_root_window_motion_notify_cb (PurplePlugin * plugin,
                                 GdkEventMotion * event) {
    g_assert (plugin != NULL && plugin->extra != NULL);
    /* select approriate mouse cursors */
    if (PLUGIN (resize_allow) && (event->state & GDK_BUTTON1_MASK) == 0)
        maybe_change_cursor (event->x, event->y, plugin);

    /* mouse button pressed */
    if ((event->state & GDK_BUTTON1_MASK) &&
        selection_defined (plugin) && PLUGIN (resize_mode) != ResizeNone) {
        gint _x1, _y1, _x2, _y2;

        /* remember previous coordinates */
        _x1 = PLUGIN (x1);
        _y1 = PLUGIN (y1);
        _x2 = PLUGIN (x2);
        _y2 = PLUGIN (y2);

        if (PLUGIN (select_mode) != SELECT_MOVE) {
            /* allow horizontal resizing */
            if (PLUGIN (resize_mode) != ResizeTop &&
                PLUGIN (resize_mode) != ResizeBottom)
                PLUGIN (x2) = (gint) event->x;
            /* allow vertical resizing */
            if (PLUGIN (resize_mode) != ResizeLeft &&
                PLUGIN (resize_mode) != ResizeRight)
                PLUGIN (y2) = (gint) event->y;

            if (event->state & GDK_CONTROL_MASK) {
                GdkScreen *screen;
                gint xdiff, ydiff;

                screen = gdk_screen_get_default ();

                xdiff = (gint) event->x - _x2;
                ydiff = (gint) event->y - _y2;

                if (PLUGIN (resize_mode) != ResizeTop &&
                    PLUGIN (resize_mode) != ResizeBottom)
                    PLUGIN (x1) =
                        MIN (MAX (PLUGIN (x1) - xdiff, 0),
                             gdk_screen_get_width (screen) - 1);

                if (PLUGIN (resize_mode) != ResizeLeft &&
                    PLUGIN (resize_mode) != ResizeRight)
                    PLUGIN (y1) =
                        MIN (MAX (PLUGIN (y1) - ydiff, 0),
                             gdk_screen_get_height (screen) - 1);
            }
        }
        else {
            GdkScreen *screen;

            screen = gdk_screen_get_default ();

            gint dx, dy;

            dx = ((gint) event->x) - PLUGIN (mouse_x);
            dy = ((gint) event->y) - PLUGIN (mouse_y);

            if (PLUGIN (x1) + dx >= 0 &&
                PLUGIN (x1) + dx < gdk_screen_get_width (screen) &&
                PLUGIN (x2) + dx >= 0 &&
                PLUGIN (x2) + dx < gdk_screen_get_width (screen)) {
                PLUGIN (x1) += dx;
                PLUGIN (x2) += dx;
            }

            if (PLUGIN (y1) + dy >= 0 &&
                PLUGIN (y1) + dy < gdk_screen_get_height (screen) &&
                PLUGIN (y2) + dy >= 0 &&
                PLUGIN (y2) + dy < gdk_screen_get_height (screen)) {
                PLUGIN (y1) += dy;
                PLUGIN (y2) += dy;
            }
        }

        /* */

        draw_selection (_x1, _x2, _y1, _y2, plugin);
    }

    if (purple_prefs_get_bool (PREF_SHOW_VISUAL_CUES)) {
        PLUGIN (mouse_x) = event->x;
        PLUGIN (mouse_y) = event->y;
    }

    return TRUE;
}

static void
clear_selection (PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);
    if (selection_defined (plugin)) {
        GdkRectangle area;
        GdkWindow *gdkwin = PLUGIN (root_window)->window;
        GdkCursor *cursor = gdk_cursor_new (GDK_CROSSHAIR);

        /* cancel current selection to try a new one */
        area.x = CAPTURE_X0 (plugin);
        area.y = CAPTURE_Y0 (plugin);
        area.width = CAPTURE_WIDTH (plugin);
        area.height = CAPTURE_HEIGHT (plugin);

        /* draw background */
        gdk_draw_pixbuf (gdkwin, PLUGIN (gc), BACKGROUND_PIXBUF,        /* src */
                         area.x, area.y,
                         area.x, area.y,
                         area.width, area.height, GDK_RGB_DITHER_NONE, 0, 0);

        RESET_SELECTION (plugin);

        gdk_window_set_cursor (gdkwin, cursor);
        gdk_cursor_unref (cursor);

        if (purple_prefs_get_bool (PREF_SHOW_VISUAL_CUES)) {
            g_assert (PLUGIN (timeout_source) == 0);
            draw_cues (TRUE, plugin);
        }
    }
}

#define SWAP_X()\
  {gint tmp =  PLUGIN (x1);\
   PLUGIN (x1) =  PLUGIN (x2);\
   PLUGIN (x2) = tmp;}

#define SWAP_Y()\
  {gint tmp =  PLUGIN (y1);\
   PLUGIN (y1) =  PLUGIN (y2);\
   PLUGIN (y2) = tmp;}

static gboolean
on_root_window_button_press_cb (GtkWidget * root_window,
                                GdkEventButton * event,
                                PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);

    if (event->button == 1) {
        if ((PLUGIN (resize_mode) == ResizeLeft && PLUGIN (x1) < PLUGIN (x2))
            || (PLUGIN (resize_mode) == ResizeRight
                && PLUGIN (x2) < PLUGIN (x1))
            || (PLUGIN (resize_mode) == ResizeTop
                && PLUGIN (y1) < PLUGIN (y2))
            || (PLUGIN (resize_mode) == ResizeBottom
                && PLUGIN (y2) < PLUGIN (y1))) {
            SWAP_X ();
            SWAP_Y ();
        }
        else if (PLUGIN (resize_mode) == ResizeTopLeft) {
            if (PLUGIN (x1) < PLUGIN (x2))
                SWAP_X ();

            if (PLUGIN (y1) < PLUGIN (y2))
                SWAP_Y ();
        }
        else if (PLUGIN (resize_mode) == ResizeBottomLeft) {
            if (PLUGIN (x1) < PLUGIN (x2))
                SWAP_X ();

            if (PLUGIN (y1) > PLUGIN (y2))
                SWAP_Y ();
        }
        else if (PLUGIN (resize_mode) == ResizeTopRight) {
            if (PLUGIN (x1) > PLUGIN (x2))
                SWAP_X ();

            if (PLUGIN (y1) < PLUGIN (y2))
                SWAP_Y ();
        }
        else if (PLUGIN (resize_mode) == ResizeBottomRight) {
            if (PLUGIN (x1) > PLUGIN (x2))
                SWAP_X ();
            if (PLUGIN (y1) > PLUGIN (y2))
                SWAP_Y ();
        }
        else if (PLUGIN (resize_mode) == ResizeAny &&
                 PLUGIN (select_mode) == SELECT_MOVE) {
            PLUGIN (mouse_x) = (gint) event->x;
            PLUGIN (mouse_y) = (gint) event->y;
        }
        else if (PLUGIN (x1) == -1) {   /* start defining the capture area */
            GdkRegion *region = NULL;
            GdkRectangle rect;

            if (purple_prefs_get_bool (PREF_SHOW_VISUAL_CUES))
	      erase_cues (plugin);

            PLUGIN (x1) = (gint) event->x;
            PLUGIN (y1) = (gint) event->y;
            PLUGIN (x2) = (gint) event->x;
            PLUGIN (y2) = (gint) event->y;

            /* draw upper-left pixel */
            rect.x = PLUGIN (x1);
            rect.y = PLUGIN (y1);
            rect.width = 1;
            rect.height = 1;

            region = gdk_region_rectangle (&rect);

            gdk_gc_set_function (PLUGIN (gc), GDK_COPY_INVERT);
            paint_region (region, root_window->window,
                          PLUGIN (root_pixbuf_orig), plugin);
            gdk_gc_set_function (PLUGIN (gc), GDK_COPY);

            gdk_region_destroy (region);
            region = NULL;  
        }
    }
    else if (event->button == 2 &&     /* hide the current conversation window  */
	     get_receiver_window (plugin) != NULL &&
             !receiver_window_is_iconified (plugin) &&
	     !selection_defined(plugin)) {

      if (purple_prefs_get_bool (PREF_SHOW_VISUAL_CUES))
	erase_cues (plugin);
      THAW_DESKTOP ();
        if (PLUGIN (root_pixbuf_x) != NULL) {
            g_object_unref (PLUGIN (root_pixbuf_x));
            PLUGIN (root_pixbuf_x) = NULL;
        }
        if (G_LIKELY (PLUGIN (root_pixbuf_orig) != NULL)) {
            g_object_unref (PLUGIN (root_pixbuf_orig));
            PLUGIN (root_pixbuf_orig) = NULL;
        }
        gtk_window_iconify (GTK_WINDOW (get_receiver_window (plugin)));
        freeze_desktop (plugin, TRUE);
    }
    else if (event->button == 3) {
        if (event->type == GDK_2BUTTON_PRESS)
            plugin_cancel (plugin);
        else if (selection_defined (plugin)) 
	    clear_selection (plugin);
    }
    return TRUE;
}

static GdkPixbuf *
extract_capture (PurplePlugin * plugin) {
    GdkPixbuf *capture = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    /* 1/ create our new pixbuf */
    g_assert (PLUGIN (root_pixbuf_orig) != NULL);
    capture =
        gdk_pixbuf_new_subpixbuf (PLUGIN (root_pixbuf_orig),
                                  CAPTURE_X0 (plugin),
                                  CAPTURE_Y0 (plugin),
                                  CAPTURE_WIDTH (plugin),
                                  CAPTURE_HEIGHT (plugin));

    if (capture != NULL) {
        /* 2/ make invisible areas black */
        mask_monitors (capture,
                       gdk_get_default_root_window (),
                       CAPTURE_X0 (plugin), CAPTURE_Y0 (plugin));

        /* 3/ add a signature to the bottom-right corner */
        if (purple_prefs_get_bool (PREF_ADD_SIGNATURE)) {
            GdkPixbuf *sign_pixbuf =
                gdk_pixbuf_new_from_file (purple_prefs_get_string
                                          (PREF_SIGNATURE_FILENAME),
                                          NULL);
            if (sign_pixbuf != NULL) {
                if (!mygdk_pixbuf_check_maxsize
                    (sign_pixbuf, SIGN_MAXWIDTH, SIGN_MAXHEIGHT)) {
                    NotifyError (PLUGIN_SIGNATURE_TOOBIG_ERROR, SIGN_MAXWIDTH,
                                 SIGN_MAXHEIGHT);
                    purple_prefs_set_string (PREF_SIGNATURE_FILENAME, "");
                }
                else
                    mygdk_pixbuf_compose (capture, sign_pixbuf);

                g_object_unref (sign_pixbuf);
                sign_pixbuf = NULL;
            }
        }
    }
    return capture;
}

static gboolean
save_capture (PurplePlugin * plugin, GdkPixbuf * capture) {
    gchar *basename = NULL;
    gchar *param_name = NULL;
    gchar *param_value = NULL;
    GError *error = NULL;
    GTimeVal g_tv;
    const gchar *const extension = purple_prefs_get_string (PREF_IMAGE_TYPE);

    g_assert (plugin != NULL && plugin->extra != NULL);

    if (PLUGIN (capture_path_filename) != NULL)
        g_free (PLUGIN (capture_path_filename));

    if (!strcmp (extension, "png")) {
        param_name = g_strdup ("compression");
        param_value =
            g_strdup_printf ("%d",
                             purple_prefs_get_int (PREF_PNG_COMPRESSION));
    }
    else if (!strcmp (extension, "jpg")) {
        param_name = g_strdup ("quality");
        param_value =
            g_strdup_printf ("%d", purple_prefs_get_int (PREF_JPEG_QUALITY));
    }

    /* create default name */
    g_get_current_time (&g_tv);
    basename = g_strdup_printf ("%s_%ld.%s", CAPTURE, g_tv.tv_sec, extension);

    /* eventually ask the user for a new name */
    screenshot_maybe_rename (plugin, &basename);

    PLUGIN (capture_path_filename) =
        g_build_filename (need_save ()?
                          purple_prefs_get_string (PREF_STORE_FOLDER) :
                          g_get_tmp_dir (), basename, NULL);
    g_free (basename);
    basename = NULL;

    /* store capture in a file */
    gdk_pixbuf_save (capture, PLUGIN (capture_path_filename), extension,
                     &error, param_name, param_value, NULL);

    if (param_name != NULL)
        g_free (param_name);
    if (param_value != NULL)
        g_free (param_value);

    if (error != NULL) {
        gchar *errmsg_saveto = g_strdup_printf (PLUGIN_SAVE_TO_FILE_ERROR,
                                                PLUGIN
                                                (capture_path_filename));

        NotifyError ("%s\n\n\%s", errmsg_saveto, error->message);

        g_free (errmsg_saveto);
        plugin_stop (plugin);
        g_error_free (error);
        return FALSE;
    }
    return TRUE;
}

static void
fetch_capture (PurplePlugin * plugin) {
    GdkPixbuf *capture = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    /* do nothing if there's no selection yet */
    if (!(selection_defined (plugin)))
        return;

    THAW_DESKTOP ();

    if (receiver_window_is_iconified (plugin))
        gtk_window_deiconify (GTK_WINDOW (get_receiver_window (plugin)));

    /* process screenshot */
    if ((capture = extract_capture (plugin)) == NULL) {
        purple_notify_error (plugin, PLUGIN_NAME, PLUGIN_ERROR,
                             PLUGIN_MEMORY_ERROR);
        plugin_stop (plugin);
    }
    else {
        /* capture was successfully stored in file */
        if (save_capture (plugin, capture)) {
            g_object_unref (capture);
            capture = NULL;

            switch (PLUGIN (send_as)) {
            case SEND_AS_FILE:
                {
                    serv_send_file
                        (purple_account_get_connection
                         (PLUGIN (account)), PLUGIN (name),
                         PLUGIN (capture_path_filename));
                    plugin_stop (plugin);
                    break;
                }
            case SEND_AS_IMAGE:
                {
                    gchar *filedata = NULL;
                    size_t size;
                    GError *error = NULL;

                    if (g_file_get_contents
                        (PLUGIN (capture_path_filename), &filedata, &size,
                         &error) == FALSE) {
                        gchar *errmsg_getdata;
                        if (filedata != NULL)
                            g_free (filedata);

                        errmsg_getdata =
                            g_strdup_printf (PLUGIN_GET_DATA_ERROR,
                                             PLUGIN (capture_path_filename));
                        NotifyError ("%s\n\n\%s", errmsg_getdata,
                                     error->message);
                        g_free (errmsg_getdata);
                        g_error_free (error);
                    }
                    else {
                        gchar *basename = NULL;
                        GtkTextIter iter;
                        GtkTextMark *ins = NULL;
                        gint purple_tmp_id;
                        GtkIMHtml *imhtml = get_receiver_imhtml (plugin);

                        basename =
                            g_path_get_basename (PLUGIN
                                                 (capture_path_filename));

                        ins =
                            gtk_text_buffer_get_insert
                            (gtk_text_view_get_buffer
                             (GTK_TEXT_VIEW (imhtml)));

                        gtk_text_buffer_get_iter_at_mark
                            (gtk_text_view_get_buffer
                             (GTK_TEXT_VIEW (imhtml)), &iter, ins);

                        purple_tmp_id =
                            purple_imgstore_add_with_id (filedata, size,
                                                         basename);
                        g_free (basename);

                        if (purple_tmp_id == 0) {
                            NotifyError ("%s\n\n\%s",
                                         PLUGIN_STORE_FAILED_ERROR,
                                         PLUGIN (capture_path_filename));
                            g_free (filedata);
                        }
                        else {
                            gtk_imhtml_insert_image_at_iter
                                (GTK_IMHTML (imhtml), purple_tmp_id, &iter);
                            /* filedata is freed here */
                            purple_imgstore_unref_by_id (purple_tmp_id);
                            gtk_widget_grab_focus (GTK_WIDGET (imhtml));
                        }
                    }
                    plugin_stop (plugin);
                    break;
                }
#ifdef ENABLE_UPLOAD
            case SEND_AS_HTTP_LINK:
                {
                    http_upload_prepare (plugin);
                    break;
                }
            case SEND_AS_FTP_LINK:
                {
                    ftp_upload_prepare (plugin);
                    break;
                }
#endif
            }
        }
    }
}

static gboolean
on_root_window_button_release_cb (PurplePlugin * plugin,
                                  GdkEventButton * event) {
    g_assert (plugin != NULL && plugin->extra != NULL);
    /* capture not yet defined */
    if (event->button != 1 || PLUGIN (x2) == -1)
        return TRUE;

    /* press shift to hold the selection */
    if ((event->state & GDK_SHIFT_MASK) || PLUGIN (resize_allow))
        PLUGIN (resize_allow) = TRUE;
    else
        fetch_capture (plugin);

    return TRUE;
}

static void
on_root_window_map_event_cb (GtkWidget * root_window, GdkEvent * event,
                             PurplePlugin * plugin) {
    GdkWindow *gdkwin;

    g_assert (plugin != NULL && plugin->extra != NULL);

    gtk_window_move (GTK_WINDOW (root_window), 0, 0);

#if GTK_CHECK_VERSION(2,14,0)
    gdkwin = gtk_widget_get_window (root_window);
#else
    gdkwin = root_window->window;
#endif

    (void) event;
}

static gboolean
on_root_window_expose_cb (GtkWidget * root_window,
                          GdkEventExpose * event, PurplePlugin * plugin) {

    g_assert (plugin != NULL && plugin->extra != NULL);

    /* This function should be called only one time,
       by the gtk_widget_show() function. The drawing work is
       actually done by on_root_window_motion_notify_cb().

       If it called twice, it means that an other process
       or program has requested user attention on top of
       our screenshot plugin... Generally this will mess up
       the display so we'd better terminate and let the user
       do what is wanting to do.       
     */
    if (PLUGIN (root_exposed)) {
        plugin_cancel (plugin);
        purple_notify_info (plugin,
                            PLUGIN_NAME, PLUGIN_INFO, FORCE_CANCEL_MSG);
        return TRUE;
    }

    /* no area is selected */
    if (!PLUGIN (root_exposed) && !selection_defined (plugin)) {
        GdkWindow *gdkwin;

        PLUGIN (root_exposed) = TRUE;
#if GTK_CHECK_VERSION(2,14,0)
        gdkwin = gtk_widget_get_window (root_window);
#else
        gdkwin = root_window->window;
#endif
        if (PLUGIN (gc) == NULL)
            PLUGIN (gc) = gdk_gc_new (gdkwin);

        gdk_draw_pixbuf (gdkwin, PLUGIN (gc), BACKGROUND_PIXBUF,        /* src */
                         event->area.x, event->area.y,
                         event->area.x, event->area.y,
                         event->area.width, event->area.height,
                         GDK_RGB_DITHER_NONE, 0, 0);

        gtk_window_move (GTK_WINDOW (root_window), 0, 0);

        /* draw cues if not already present and if necessary */
        if (PLUGIN (timeout_source) == 0 &&
            purple_prefs_get_bool (PREF_SHOW_VISUAL_CUES)) {
            GdkDisplay *display = gdk_display_get_default ();

            gdk_display_get_pointer (display, NULL, &PLUGIN (mouse_x),
                                     &PLUGIN (mouse_y), NULL);
            /* don't use double-buffering from here, otherwise display will be
               messed up, gdk bug ? */
            draw_cues (FALSE, plugin);
        }
    }
    return TRUE;
}

static gboolean
on_root_window_key_press_event_cb (GtkWidget * root_events,
                                   GdkEventKey * event,
                                   PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);
    switch (event->keyval) {
    case GDK_f:case GDK_F:               /* select full screen */
        {
            GdkWindow *gdkroot;
            gint _x1, _y1, _x2, _y2, width, height;

            if (!selection_defined (plugin) &&
                purple_prefs_get_bool (PREF_SHOW_VISUAL_CUES)) {
                erase_cues (plugin);
                PLUGIN (resize_allow) = TRUE;
            }

            /* remember previous coordinates */
            _x1 = PLUGIN (x1);
            _y1 = PLUGIN (y1);
            _x2 = PLUGIN (x2);
            _y2 = PLUGIN (y2);

            gdkroot = gdk_get_default_root_window ();

            gdk_drawable_get_size (gdkroot, &width, &height);

            PLUGIN (x1) = 0;
            PLUGIN (y1) = 0;
            PLUGIN (x2) = width - 1;
            PLUGIN (y2) = height - 1;

            draw_selection (_x1, _x2, _y1, _y2, plugin);
            maybe_change_cursor (PLUGIN (mouse_x), PLUGIN (mouse_y), plugin);
            break;
        }
    case GDK_Escape:
        plugin_cancel (plugin);
        break;
    case GDK_Return:
        fetch_capture (plugin);
        break;
    case GDK_BackSpace:
        clear_selection (plugin);
        break;
    }
    (void) root_events;
    return FALSE;
}

static void
on_screen_monitors_changed_cb (GdkScreen * screen, PurplePlugin * plugin) {
    g_assert (plugin != NULL && plugin->extra != NULL);
    gtk_widget_set_size_request (PLUGIN (root_window),
                                 gdk_screen_get_width (screen),
                                 gdk_screen_get_height (screen));
    clear_selection (plugin);
}

void
prepare_root_window (PurplePlugin * plugin) {
    GdkScreen *screen = NULL;

    g_assert (plugin != NULL && plugin->extra != NULL);

    /* From gdk API :
     *   "a screen may consist of multiple monitors which a
     *   re merged to form a large screen area".
     */
    screen = gdk_screen_get_default ();

    g_assert (PLUGIN (root_window) == NULL);
    g_assert (PLUGIN (root_events) == NULL);

    /* No toplevel otherwise some desktops (say Xfce4),
     * won't allow us to cover the entire screen. */
    PLUGIN (root_window) = gtk_window_new (GTK_WINDOW_POPUP);

    /* Seems like GTK_WINDOW_POPUP cannot receive key-press events,
     * so we create an "invisible" GTK_WINDOW_TOPLEVEL to catch them.
     */
    PLUGIN (root_events) = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated (GTK_WINDOW (PLUGIN (root_events)), FALSE);
    gtk_widget_set_size_request (PLUGIN (root_events), 1, 1);

    /* Here, gtk_window_fullscreen() won't work 'cos most (every)
       WMs do this on the current monitor only, while we want our
       invisible window to cover the entire GdkScreen. */
    gtk_widget_set_size_request (PLUGIN (root_window),
                                 gdk_screen_get_width (screen),
                                 gdk_screen_get_height (screen));

#if GTK_CHECK_VERSION(2,4,0)
    gtk_window_set_keep_above (GTK_WINDOW (PLUGIN (root_window)), TRUE);
#endif

    /* install callbacks */
    g_signal_connect_swapped (GTK_OBJECT (PLUGIN (root_window)), "realize",
                              G_CALLBACK (on_root_window_realize_cb), plugin);
    g_signal_connect (GTK_OBJECT (PLUGIN (root_window)),
                      "button-press-event",
                      G_CALLBACK (on_root_window_button_press_cb), plugin);
    g_signal_connect_swapped (GTK_OBJECT (PLUGIN (root_window)),
                              "button-release-event",
                              G_CALLBACK (on_root_window_button_release_cb),
                              plugin);

    g_signal_connect (GTK_OBJECT (PLUGIN (root_window)),
                      "expose-event",
                      G_CALLBACK (on_root_window_expose_cb), plugin);
    g_signal_connect_swapped (GTK_OBJECT (PLUGIN (root_window)),
                              "motion-notify-event",
                              G_CALLBACK (on_root_window_motion_notify_cb),
                              plugin);
    g_signal_connect (GTK_OBJECT (PLUGIN (root_window)), "map-event",
                      G_CALLBACK (on_root_window_map_event_cb), plugin);
    g_signal_connect (GTK_OBJECT (PLUGIN (root_events)), "key-press-event",
                      G_CALLBACK (on_root_window_key_press_event_cb), plugin);
#ifdef G_OS_WIN32
    /* waiting for signal "monitors-changed" to be implemented
       under Win32 */
    g_signal_connect (G_OBJECT (screen), "size-changed",
                      G_CALLBACK (on_screen_monitors_changed_cb), plugin);
#else
    g_signal_connect (G_OBJECT (screen),
                      "monitors-changed",
                      G_CALLBACK (on_screen_monitors_changed_cb), plugin);
#endif
}

/* end of screencap.c */
