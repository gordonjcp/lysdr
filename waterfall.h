/*  lysdr Software Defined Radio
    (C) 2010-2011 Gordon JC Pearce MM0YEQ and others

    waterfall.h

	This file is part of lysdr.

	lysdr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	any later version.

	lysdr is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with lysdr.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef __WATERFALL_H
#define __WATERFALL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SDRWaterfall            SDRWaterfall;
typedef struct _SDRWaterfallClass       SDRWaterfallClass;
typedef struct _SDRWaterfallPrivate     SDRWaterfallPrivate;

enum {
    P_NONE,
    P_TUNING,
    P_HIGHPASS,
    P_LOWPASS,
    P_BANDSPREAD
};

enum {
    WF_O_VERTICAL,
    WF_O_HORIZONTAL
};

struct _SDRWaterfall {
    GtkDrawingArea parent;

    GtkAdjustment *tuning;
    GtkAdjustment *lp_tune;
    GtkAdjustment *hp_tune;

    GdkPixbuf *pixels;
    GdkPixbuf *scale;

    cairo_surface_t *pix;

    gint mode;
    gint orientation;

    gint width;
    gint wf_height;

    gint sample_rate;
    gint centre_freq;
    gint fft_size;
};

struct _SDRWaterfallClass {
    GtkDrawingAreaClass parent_class;
};

struct _SDRWaterfallPrivate {
    gint cursor_pos;    // pixel position for tuning cursor
    gint lp_pos;        // pixel position for lowpass cursor
    gint hp_pos;        // pixel position for highpass cursor
    gint scroll_pos;    // which line the scroller is on
    gint prelight;
    gint drag;
    gint click_pos;
    gdouble bandspread;
    GMutex mutex;
};

#define SDR_TYPE_WATERFALL             (sdr_waterfall_get_type ())
#define SDR_WATERFALL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SDR_TYPE_WATERFALL, SDRWaterfall))
#define SDR_WATERFALL_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), SDR_WATERFALL,  SDRWaterfallClass))
#define SDR_IS_WATERFALL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SDR_TYPE_WATERFALL))
#define SDR_IS_WATERFALL_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), SDR_TYPE_WATERFALL))
#define SDR_WATERFALL_GET_CLASS        (G_TYPE_INSTANCE_GET_CLASS ((obj), SDR_TYPE_WATERFALL, SDRWaterfallClass))
#define SDR_WATERFALL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SDR_TYPE_WATERFALL, SDRWaterfallPrivate))

G_END_DECLS

#define LOOSE 2
#define WITHIN(x, p) (x-1 > p-LOOSE) && (x-1 < p + LOOSE)

#define SCALE_HEIGHT 24
#define SCALE_WIDTH 50
#define SCALE_TICK 5000

SDRWaterfall *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune, gint sample_rate, gint fft_size);
float sdr_waterfall_get_tuning(SDRWaterfall *wf);
float sdr_waterfall_get_lowpass(SDRWaterfall *wf);
float sdr_waterfall_get_highpass(SDRWaterfall *wf);

void sdr_waterfall_set_tuning(SDRWaterfall *wf, gdouble value);
void sdr_waterfall_update(GtkWidget *widget, guchar *row);
void sdr_waterfall_set_scale(GtkWidget *widget, gint centre_freq);
void sdr_waterfall_filter_cursors(SDRWaterfall *wf);
void sdr_waterfall_set_lowpass(SDRWaterfall *wf, gdouble value);
void sdr_waterfall_set_highpass(SDRWaterfall *wf, gdouble value);
GType sdr_waterfall_get_type(void);
#endif /* __WATERFALL_H */

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
