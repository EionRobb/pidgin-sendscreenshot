/*
 * Pidgin SendScreenshot third-party plugin - MsWindows specific.
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

/* Code heavily stolen from the winsnap screenshot plugin in Gimp.
 * http://git.gnome.org/browse/gimp/tree/plug-ins/win-snap
 * 
 * WinSnap Win32 Window Capture Plug-in
 * Copyright (C) 1999 Craig Setera
 * Craig Setera <setera@home.com>
 * 07/14/1999
 *
 */

#include <gtk/gtk.h>
#include <windows.h>

#include "mswin-freeze.h"

/* File variables */
static guchar *capBytes = NULL;

/* We create a DIB section to hold the grabbed area. The scanlines in
 * DIB sections are aligned ona LONG (four byte) boundary. Its pixel
 * data is in RGB (BGR actually) format, three bytes per pixel.
 *
 * GIMP uses no alignment for its pixel regions. The GIMP image we
 * create is of type RGB, i.e. three bytes per pixel, too. Thus in
 * order to be able to quickly transfer all of the image at a time, we
 * must use a DIB section and pixel region the scanline width in
 * bytes of which is evenly divisible with both 3 and 4. I.e. it must
 * be a multiple of 12 bytes, or in pixels, a multiple of four pixels.
 */

#define ROUND4(width) (((width-1)/4+1)*4)

/*
 * flipRedAndBlueBytes
 *
 * Microsoft has chosen to provide us a very nice (not!)
 * interface for retrieving bitmap bits.  DIBSections have
 * RGB information as BGR instead.  So, we have to swap
 * the silly red and blue bytes before sending to the
 * GIMP.
 */
static void
flipRedAndBlueBytes (int width, int height) {
    int i, j;
    guchar *bufp;
    guchar temp;

    j = 0;
    while (j < height) {
        i = width;
        bufp = capBytes + j * ROUND4 (width) * 3;
        while (i--) {
            temp = bufp[2];
            bufp[2] = bufp[0];
            bufp[0] = temp;
            bufp += 3;
        }
        j++;
    }
}

/*
 * sendBMPToGIMP
 *
 * Take the captured data and send it across
 * to GIMP.
 */
static GdkPixbuf *
sendBMPToGimp (HBITMAP hBMP, HDC hDC, RECT rect) {
    gint width, height;
    GdkPixbuf *pixbuf;

    /* Our width and height */
    width = (rect.right - rect.left);
    height = (rect.bottom - rect.top);

    flipRedAndBlueBytes (width, height);
    /* Check that we got the memory */
    if (!capBytes) {
        g_message ("No data captured");
        return NULL;
    }

    pixbuf = gdk_pixbuf_new_from_data
        ((guchar *) capBytes,
         GDK_COLORSPACE_RGB, FALSE, 8, ROUND4 (width), height,
         ROUND4 (width) * 3, NULL, NULL);

    return pixbuf;
}

/*
 * primDoWindowCapture
 *
 * The primitive window capture functionality.  Accepts
 * the two device contexts and the rectangle to be
 * captured.
 */
static HBITMAP
primDoWindowCapture (HDC hdcWindow, HDC hdcCompat, RECT rect) {
    HBITMAP hbmCopy;
    HGDIOBJ oldObject;
    BITMAPINFO bmi;

    int width = (rect.right - rect.left);
    int height = (rect.bottom - rect.top);

    /* Create the bitmap info header */
    bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = ROUND4 (width);
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    /* Create the bitmap storage space */
    hbmCopy = CreateDIBSection (hdcCompat,
                                (BITMAPINFO *) & bmi,
                                DIB_RGB_COLORS, &capBytes, NULL, 0);
    if (!hbmCopy) {

        g_error ("Error creating DIB section");
        return NULL;
    }

    /* Select the bitmap into the compatible DC. */
    oldObject = SelectObject (hdcCompat, hbmCopy);
    if (!oldObject) {

        g_error ("Error selecting object");
        return NULL;
    }

    /* Copy the data from the application to the bitmap.  Even if we did
     * round up the width, BitBlt only the actual data.
     */
    if (!BitBlt (hdcCompat, 0, 0,
                 width, height, hdcWindow, rect.left, rect.top, SRCCOPY)) {

        g_error ("Error copying bitmap");
        return NULL;
    }

    /* Restore the original object */
    SelectObject (hdcCompat, oldObject);

    return hbmCopy;
}


static GdkPixbuf *
doCapture (GdkRectangle gdk_rect) {
    HDC hdcSrc;
    HDC hdcCompat;
    RECT gdi_rect;
    HBITMAP hbm;
    GdkPixbuf *pixbuf = NULL;

    /* Get the device context for the whole screen */
    hdcSrc = CreateDC (TEXT ("DISPLAY"), NULL, NULL, NULL);

    /* Get the screen's rectangle */
    gdi_rect.top = gdk_rect.y;
    gdi_rect.bottom = gdk_rect.y + gdk_rect.height;
    gdi_rect.left = gdk_rect.x;
    gdi_rect.right = gdk_rect.x + gdk_rect.width;

    if (!hdcSrc) {
        g_error ("Error getting device context");
        return NULL;
    }
    hdcCompat = CreateCompatibleDC (hdcSrc);
    if (!hdcCompat) {
        g_error ("ErRor getting compat device context");
        return NULL;
    }

    /* Do the window capture */
    hbm = primDoWindowCapture (hdcSrc, hdcCompat, gdi_rect);
    if (!hbm)
        return NULL;

    /* Release the device context */
    ReleaseDC (NULL, hdcSrc);

    /* Send the bitmap */
    if (hbm != NULL) {
        pixbuf = sendBMPToGimp (hbm, hdcCompat, gdi_rect);
    }
    return pixbuf;
}

gboolean
mswin_freeze_screen (PurplePlugin * plugin, GdkRectangle gdk_rect) {
    GdkWindow *root;
    HWND deskwin;

    g_assert (plugin != NULL && plugin->extra != NULL);

    deskwin = GetDesktopWindow ();
    root = gdk_window_foreign_new ((GdkNativeWindow) deskwin);

    return ((PLUGIN (root_pixbuf_orig) = doCapture (gdk_rect)) != NULL);
}

/* end of mswin-freeze.c */
