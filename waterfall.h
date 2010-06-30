/* waterfall.h */
/* gordon@gjcp.net */

#ifndef __WATERFALL_H
#define __WATERFALL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SDRWaterfall            SDRWaterfall;
typedef struct _SDRWaterfallClass       SDRWaterfallClass;
typedef struct _SDRWaterfallPrivate     SDRWaterfallPrivate;

struct _SDRWaterfall {
    GtkDrawingArea parent;
    
    GtkAdjustment *tuning;
    GtkAdjustment *lp_tune;
    GtkAdjustment *hp_tune;
    
    gdouble position;          // cursor position
    gint highpass, lowpass; // highpass and lowpass marker offset from cursor
    gint mode;
    gint max_lowpass;       // highest possible lowpass setting
    gint drag;              // what we're dragging (if anything)
    gint prelight;
    gint last_prelight;
    gint dragoffset;        // where we clicked when dragging the whole cursor
    gint fft_size;          // defines the maximum width
    // these can be moved out to private values
    gint c_left;
    gint c_right;
    gint c_lowpass;
    gint c_highpass;
    guchar *pixels;     // actual pixel data for the waterfall
};

struct _SDRWaterfallClass {
    GtkDrawingAreaClass parent_class;
    void (* tuning_changed) (SDRWaterfall *wf, float tuning);
    void (* filter_changed) (SDRWaterfall *wf, float lowpass);
};

struct _SDRWaterfallPrivate {
};

enum dragstate {
    NONE,
    LOWPASS,
    HIGHPASS,
    TUNE,
    TUNE_GRAB
};

enum {
    USB,
    LSB
};

enum {
    TUNING_CHANGED,
    FILTER_CHANGED,
    LAST_SIGNAL
};

#define SDR_TYPE_WATERFALL             (sdr_waterfall_get_type ())
#define SDR_WATERFALL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SDR_TYPE_WATERFALL, SDRWaterfall))
#define SDR_WATERFALL_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), SDR_WATERFALL,  SDRWaterfallClass))
#define SDR_IS_WATERFALL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SDR_TYPE_WATERFALL))
#define SDR_IS_WATERFALL_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), SDR_TYPE_WATERFALL))
#define SDR_WATERFALL_GET_CLASS        (G_TYPE_INSTANCE_GET_CLASS ((obj), SDR_TYPE_WATERFALL, SDRWaterfallClass))
#define SDR_WATERFALL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SDR_TYPE_WATERFALL, SDRWaterfallPrivate))

G_END_DECLS

GtkWidget *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune);
void sdr_waterfall_update(GtkWidget *widget);
#endif /* __WATERFALL_H */


