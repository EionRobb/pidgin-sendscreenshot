 /*
  * Pidgin SendScreenshot third-party plugin - pixbuf manipulation utilities.
  *
  * Pixbuf manipulation utilities.
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

#include "pixbuf_utils.h"

static guchar
op_lighten (guchar val, gpointer data) {
    return MIN (val + GPOINTER_TO_INT(data), 255);
}


static guchar
op_darken (guchar val, gpointer data) {
    return MAX (val - GPOINTER_TO_INT(data), 0);
}


static void
mygdk_pixbuf_apply_op (GdkPixbuf * pixbuf,
                       PixbufOpFunc op_func, gpointer data) {
    BEGIN_PIXBUF_FORALL (pixbuf);
    p[0] = op_func (p[0], data);
    p[1] = op_func (p[1], data);
    p[2] = op_func (p[2], data);
    END_PIXBUF_FORALL ();
}

void
mygdk_pixbuf_lighten (GdkPixbuf * pixbuf, gint val) {
    mygdk_pixbuf_apply_op (pixbuf, op_lighten, GINT_TO_POINTER(val));
}

void
mygdk_pixbuf_darken (GdkPixbuf * pixbuf, gint val) {
    mygdk_pixbuf_apply_op (pixbuf, op_darken, GINT_TO_POINTER(val));
}

void
mygdk_pixbuf_grey (GdkPixbuf * pixbuf) {
    BEGIN_PIXBUF_FORALL (pixbuf);
    guchar val = p[0] * 0.299 + p[1] * 0.587 + p[2] * 0.114;
    p[0] = val;
    p[1] = val;
    p[2] = val;
    END_PIXBUF_FORALL ();
}

void
mygdk_pixbuf_compose (GdkPixbuf * pixbuf1, GdkPixbuf * pixbuf) {
    guchar *pixels1;
    gint rowstride1, n_channels1;
    gint width1, height1;

    pixels1 = gdk_pixbuf_get_pixels (pixbuf1);
    n_channels1 = gdk_pixbuf_get_n_channels (pixbuf1);
    rowstride1 = gdk_pixbuf_get_rowstride (pixbuf1);
    width1 = gdk_pixbuf_get_width (pixbuf1);
    height1 = gdk_pixbuf_get_height (pixbuf1);


    if (gdk_pixbuf_get_has_alpha (pixbuf)) {
        BEGIN_PIXBUF_FORALL (pixbuf);
        gdouble alpha;
        guchar *p1;

        /* do nothing if pixbuf cannot fit in pixbuf1 */
        if (width1 < width || height1 < height)
            return;

        p1 = pixels1 + (y + (height1 - height)) * rowstride1 +
            (x + (width1 - width)) * n_channels1;

        alpha = p[3] / (gdouble) 255.;

        p1[0] += alpha * (p[0] - p1[0]);
        p1[1] += alpha * (p[1] - p1[1]);
        p1[2] += alpha * (p[2] - p1[2]);
        END_PIXBUF_FORALL ();
    }
    else {
        BEGIN_PIXBUF_FORALL (pixbuf);
        guchar *p1 = pixels1 + (y + (height1 - height)) * rowstride1 +
            (x + (width1 - width)) * n_channels1;

        p1[0] = p[0];
        p1[1] = p[1];
        p1[2] = p[2];
        END_PIXBUF_FORALL ();
    }
}

gboolean
mygdk_pixbuf_check_maxsize (GdkPixbuf * pixbuf,
                            gint max_width, gint max_height) {
    return
        gdk_pixbuf_get_width (pixbuf) <= max_width &&
        gdk_pixbuf_get_height (pixbuf) <= max_height;
}


/* =============================================================== 
   
   All the above is stolen from gnome-screenshot code, part of
   GnomeUtils.
   See http://live.gnome.org/GnomeUtils

   Copyright (C) 2001 Jonathan Blandford <jrb@alum.mit.edu>
   Copyright (C) 2006 Emmanuele Bassi <ebassi@gnome.org>
   Copyright (C) 2008 Cosimo Cecchi <cosimoc@gnome.org> 

   =============================================================== */

static GdkRegion *
make_region_with_monitors (GdkScreen * screen) {
    GdkRegion *region;
    int num_monitors;
    int i;

    num_monitors = gdk_screen_get_n_monitors (screen);

    region = gdk_region_new ();

    for (i = 0; i < num_monitors; i++) {
        GdkRectangle rect;

        gdk_screen_get_monitor_geometry (screen, i, &rect);
        gdk_region_union_with_rect (region, &rect);
    }

    return region;
}

static void
blank_rectangle_in_pixbuf (GdkPixbuf * pixbuf, GdkRectangle * rect) {
    int x, y;
    int x2, y2;
    guchar *pixels;
    int rowstride;
    int n_channels;
    guchar *row;
    gboolean has_alpha;


    x2 = rect->x + rect->width;
    y2 = rect->y + rect->height;

    pixels = gdk_pixbuf_get_pixels (pixbuf);
    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
    n_channels = gdk_pixbuf_get_n_channels (pixbuf);

    for (y = rect->y; y < y2; y++) {
        guchar *p;

        row = pixels + y * rowstride;
        p = row + rect->x * n_channels;

        for (x = rect->x; x < x2; x++) {
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;

            if (has_alpha)
                *p++ = 255;     /* opaque black */
        }
    }
}


/* Modified for pidgin-sendscreenshot to handle custom selected area. */
static void
blank_region_in_pixbuf (GdkPixbuf * pixbuf, GdkRegion * region,
                        gint x, gint y) {
    GdkRectangle *rects;
    int n_rects;
    int i;
    GdkRectangle pixbuf_rect;

    gdk_region_get_rectangles (region, &rects, &n_rects);

    pixbuf_rect.x = x;
    pixbuf_rect.y = y;
    pixbuf_rect.width = gdk_pixbuf_get_width (pixbuf);
    pixbuf_rect.height = gdk_pixbuf_get_height (pixbuf);

    for (i = 0; i < n_rects; i++) {
        GdkRectangle dest;

        if (gdk_rectangle_intersect (rects + i, &pixbuf_rect, &dest)) {
            dest.x -= x;
            dest.y -= y;
            blank_rectangle_in_pixbuf (pixbuf, &dest);
        }
    }

    g_free (rects);
}

/* When there are multiple monitors with different resolutions, the visible area
 * within the root window may not be rectangular (it may have an L-shape, for
 * example). In that case, mask out the areas of the root window which would
 * not be visible in the monitors, so that screenshot do not end up with content
 * that the user won't ever see.
 *
 * Modified for pidgin-sendscreenshot to handle custom selected area.
 *
 */
void
mask_monitors (GdkPixbuf * pixbuf, GdkWindow * root_window, gint x, gint y) {
    GdkScreen *screen;
    GdkRegion *region_with_monitors;
    GdkRegion *invisible_region;
    GdkRectangle rect;

    screen = gdk_drawable_get_screen (GDK_DRAWABLE (root_window));

    region_with_monitors = make_region_with_monitors (screen);

    rect.x = x;
    rect.y = y;
    rect.width = gdk_pixbuf_get_width (pixbuf);
    rect.height = gdk_pixbuf_get_height (pixbuf);

    invisible_region = gdk_region_rectangle (&rect);
    gdk_region_subtract (invisible_region, region_with_monitors);

    blank_region_in_pixbuf (pixbuf, invisible_region, x, y);

    gdk_region_destroy (region_with_monitors);
    gdk_region_destroy (invisible_region);
}

/* end of pixbuf_utils.c */
