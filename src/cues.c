
/*
  *  Pidgin SendScreenshot plugin - cues funcs -
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

#include "cues.h"
#include "screencap.h"

static gboolean
update_cue_offset (gpointer data) {
    PurplePlugin *plugin = (PurplePlugin *) data;

    PLUGIN (cue_offset)++;
    PLUGIN (cue_offset) %= CUE_LENGTH;

    draw_cues (TRUE, plugin);

    return TRUE;
}


/* draw visual cues */
static void
__private_f_cues (gboolean on, gboolean double_buff,    /* see expose-event handler */
                  PurplePlugin * plugin) {
    GdkWindow *gdkwin = NULL;
    GtkWidget *root_window = NULL;
    GdkRegion *union_r = NULL;
    gint width, height;
    GdkRectangle v, h;
    GdkRectangle _v, _h;

    g_assert (!selection_defined (plugin));

    root_window = PLUGIN (root_window);

#if GTK_CHECK_VERSION(2,14,0)
    gdkwin = gtk_widget_get_window (root_window);
#else
    gdkwin = root_window->window;
#endif



    gdk_drawable_get_size (gdkwin, &width, &height);

    /* clear previous lines */
    h.x = 0;
    h.y = PLUGIN (__mouse_y);
    h.width = width;
    h.height = 1;

    v.x = PLUGIN (__mouse_x);
    v.y = 0;
    v.width = 1;
    v.height = height;


    if (double_buff && on) {
        GdkRegion *_rv, *_rh, *rv, *rh;

        _v.x = PLUGIN (mouse_x);
        _v.y = 0;
        _v.width = 1;
        _v.height = height;

        _h.x = 0;
        _h.y = PLUGIN (mouse_y);
        _h.width = width;
        _h.height = 1;

        _rv = gdk_region_rectangle (&_v);
        _rh = gdk_region_rectangle (&_h);
        rv = gdk_region_rectangle (&v);
        rh = gdk_region_rectangle (&h);


        union_r = gdk_region_copy (_rv);

        gdk_region_union (union_r, _rh);
        gdk_region_union (union_r, rh);
        gdk_region_union (union_r, rv);

        gdk_region_destroy (rv);
        gdk_region_destroy (_rv);
        gdk_region_destroy (rh);
        gdk_region_destroy (_rh);
    }

    /* double buffering on */
    if (double_buff && on)
        gdk_window_begin_paint_region (gdkwin, union_r);


    if (PLUGIN (__mouse_y) >= 0 && PLUGIN (__mouse_x) >= 0) {

        gdk_draw_pixbuf (gdkwin, PLUGIN (gc), BACKGROUND_PIXBUF,        /* src */
                         h.x, h.y,
                         h.x, h.y, h.width, h.height, GDK_RGB_DITHER_NONE, 0,
                         0);
        gdk_draw_pixbuf (gdkwin, PLUGIN (gc), BACKGROUND_PIXBUF,        /* src */
                         v.x, v.y,
                         v.x, v.y, v.width, v.height, GDK_RGB_DITHER_NONE, 0,
                         0);
    }

    if (on) {
        gint k;
        /* draw cues */
        if (double_buff) {
            gdk_draw_pixbuf (gdkwin, PLUGIN (gc), BACKGROUND_PIXBUF,    /* src */
                             _h.x, _h.y,
                             _h.x, _h.y, _h.width, _h.height,
                             GDK_RGB_DITHER_NONE, 0, 0);

            gdk_draw_pixbuf (gdkwin, PLUGIN (gc), BACKGROUND_PIXBUF,    /* src */
                             _v.x, _v.y,
                             _v.x, _v.y, _v.width, _v.height,
                             GDK_RGB_DITHER_NONE, 0, 0);
        }

        gdk_gc_set_function (PLUGIN (gc), GDK_COPY_INVERT);
        /* horizontal */
        for (k = PLUGIN (cue_offset);
             k < width - (CUE_LENGTH / 2); k += CUE_LENGTH) {
            gdk_draw_pixbuf (gdkwin, PLUGIN (gc), PLUGIN (root_pixbuf_orig),    /* src */
                             k, PLUGIN (mouse_y),
                             k, PLUGIN (mouse_y),
                             CUE_LENGTH / 2, 1, GDK_RGB_DITHER_NONE, 0, 0);
        }


        /* vertical */
        for (k = PLUGIN (cue_offset);
             k < height - (CUE_LENGTH / 2); k += CUE_LENGTH) {
            gdk_draw_pixbuf (gdkwin, PLUGIN (gc),
                             PLUGIN (root_pixbuf_orig),
                             PLUGIN (mouse_x), k,
                             PLUGIN (mouse_x), k,
                             1, CUE_LENGTH / 2, GDK_RGB_DITHER_NONE, 0, 0);

        }
        gdk_gc_set_function (PLUGIN (gc), GDK_COPY);
        PLUGIN (__mouse_x) = PLUGIN (mouse_x);
        PLUGIN (__mouse_y) = PLUGIN (mouse_y);

        if (PLUGIN (timeout_source) == 0) {
	  PLUGIN (timeout_source) =
                g_timeout_add (30, update_cue_offset, plugin);
        }

    }
    /* double buffering off */
    if (double_buff && on) {
        gdk_window_end_paint (gdkwin);
        gdk_region_destroy (union_r);
    }
}

void
draw_cues (gboolean double_buff, PurplePlugin * plugin) {
    g_assert (plugin != NULL);
    __private_f_cues (TRUE, double_buff, plugin);
}

void
erase_cues (PurplePlugin * plugin) {
    g_assert (plugin != NULL);

    if (PLUGIN (timeout_source) != 0) {
      if (g_source_remove (PLUGIN (timeout_source)))      
        PLUGIN (timeout_source) = 0;
    }

    __private_f_cues (FALSE, TRUE, plugin);
}

/* end of cues.c */
