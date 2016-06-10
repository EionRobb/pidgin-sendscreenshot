
#ifndef __PIXBUF_UTILS_H__
#define __PIXBUF_UTILS_H__ 1

#include "main.h"

typedef guchar (*PixbufOpFunc) (guchar, gpointer);

void mygdk_pixbuf_lighten (GdkPixbuf * pixbuf, gint val);

void mygdk_pixbuf_darken (GdkPixbuf * pixbuf, gint val);

void mygdk_pixbuf_grey (GdkPixbuf * pixbuf);

void mygdk_pixbuf_compose (GdkPixbuf * pixbuf1, GdkPixbuf * pixbuf);

gboolean
mygdk_pixbuf_check_maxsize (GdkPixbuf * pixbuf,
                            gint max_width, gint max_height);

void
  mask_monitors (GdkPixbuf * pixbuf, GdkWindow * root_window, gint x, gint y);

#define BEGIN_PIXBUF_FORALL(pixbuf)\
  guchar *pixels;\
  gint width, height, rowstride, n_channels;\
  gint x, y;\
  pixels = gdk_pixbuf_get_pixels (pixbuf);\
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);\
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);\
  width = gdk_pixbuf_get_width (pixbuf);\
  height = gdk_pixbuf_get_height (pixbuf);\
  for (x = 0; x < width; x++)\
    for (y = 0; y < height; y++)\
      {\
	guchar *p =\
	  pixels + y * rowstride + x * n_channels
#define END_PIXBUF_FORALL()\
  }


#endif
/* end of pixbuf_utils.h */
