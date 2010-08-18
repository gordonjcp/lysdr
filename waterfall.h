/* waterfall.h */
/* Copyright 2010 Gordon JC Pearce <gordon@gjcp.net> */


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

struct _SDRWaterfall {
    GtkDrawingArea parent;
    
    GtkAdjustment *tuning;
    GtkAdjustment *lp_tune;
    GtkAdjustment *hp_tune;

    GdkPixmap *pixmap;
    GdkPixmap *scale;
    cairo_surface_t *pix;

    gint mode;

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
    GMutex *mutex;
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
#define SCALE_TICK 5000

GtkWidget *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune, gint sample_rate, gint fft_size);
float sdr_waterfall_get_tuning(SDRWaterfall *wf);
float sdr_waterfall_get_lowpass(SDRWaterfall *wf);
float sdr_waterfall_get_highpass(SDRWaterfall *wf);

void sdr_waterfall_set_tuning(SDRWaterfall *wf, gdouble value);
void sdr_waterfall_update(GtkWidget *widget, guchar *row);
void sdr_waterfall_set_scale(GtkWidget *widget, gint centre_freq);
void sdr_waterfall_filter_cursors(SDRWaterfall *wf);
#endif /* __WATERFALL_H */


