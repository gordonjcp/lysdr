/* waterfall.c */
/* Copyright 2010 Gordon JC Pearce <gordon@gjcp.net> */

#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkmain.h>

#include "waterfall.h"

static GtkWidgetClass *parent_class = NULL;
G_DEFINE_TYPE (SDRWaterfall, sdr_waterfall, GTK_TYPE_DRAWING_AREA);

static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event);
static gboolean sdr_waterfall_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean sdr_waterfall_button_release(GtkWidget *widget, GdkEventButton *event);
static void sdr_waterfall_realize(GtkWidget *widget);
static void sdr_waterfall_unrealize(GtkWidget *widget);

static void sdr_waterfall_class_init (SDRWaterfallClass *class) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    parent_class = gtk_type_class(GTK_TYPE_DRAWING_AREA);

    widget_class->realize = sdr_waterfall_realize;
    widget_class->unrealize = sdr_waterfall_unrealize; // hate american spelling
    widget_class->expose_event = sdr_waterfall_expose;
    widget_class->button_press_event = sdr_waterfall_button_press;
    widget_class->button_release_event = sdr_waterfall_button_release;
    widget_class->motion_notify_event = sdr_waterfall_motion_notify;
}

static void sdr_waterfall_init (SDRWaterfall *wf) {
    
    gtk_widget_set_events(GTK_WIDGET(wf), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    wf->prelight = P_NONE;
    wf->drag = P_NONE;
    wf->scroll_pos = 0;
}

static void sdr_waterfall_realize(GtkWidget *widget) {
    // here we handle things that must happen once the widget has a size
    SDRWaterfall *wf;
    int i, j;

    g_return_if_fail(SDR_IS_WATERFALL(widget));

    // chain up so we even *have* the size;
    GTK_WIDGET_CLASS(parent_class)->realize(widget);
    
    wf = SDR_WATERFALL(widget);
    wf->cursor_pos = widget->allocation.width * (0.5+(wf->tuning->value/wf->sample_rate)); 
    if (!wf->pixels) wf->pixels=g_new0(guchar, wf->fft_size*4*widget->allocation.height);

}

static void sdr_waterfall_unrealize(GtkWidget *widget) {

    SDRWaterfall *wf = SDR_WATERFALL(widget);

    g_free(wf->pixels);

    GTK_WIDGET_CLASS(parent_class)->unrealize(widget);
}

static void sdr_waterfall_tuning_changed(GtkWidget *widget, gpointer *p) {
    // if the tuning adjustment changes, ensure the pointer is recalculated
    SDRWaterfall *wf = SDR_WATERFALL(p);
    int width = GTK_WIDGET(p)->allocation.width;
    gdouble value = gtk_adjustment_get_value(wf->tuning);
    wf->cursor_pos = width * (0.5+(value/wf->sample_rate));
    gtk_widget_queue_draw(GTK_WIDGET(wf));
}


GtkWidget *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune, gint sample_rate, gint fft_size) {
    // call this with three Adjustments, for tuning, lowpass filter and highpass filter
    // the tuning Adjustment should have its upper and lower bounds set to half the sample rate
    SDRWaterfall *wf;

    wf = g_object_new(
        SDR_TYPE_WATERFALL,
        NULL
    );

    wf->tuning = tuning;
    wf->lp_tune = lp_tune;
    wf->hp_tune = hp_tune;
    wf->sample_rate = sample_rate;
    wf->fft_size = fft_size;
    
    g_signal_connect (tuning, "value-changed",
        G_CALLBACK (sdr_waterfall_tuning_changed), wf);
    
    return GTK_WIDGET(wf);
    
}

static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event) {

    gint prelight = P_NONE; 
    SDRWaterfall *wf = SDR_WATERFALL(widget);

    gdouble value;
    gint offset;
    gboolean dirty = FALSE;

    gint width = widget->allocation.width;    
    gint x = CLAMP(event->x, 0, width);
    if (wf->drag == P_NONE) {    
        //if (x == wf->cursor_pos) {
        if (WITHIN(x, wf->cursor_pos)) {
            prelight = P_TUNING;
            dirty = TRUE;
        }
    }
    if (wf->drag == P_TUNING) {
        // drag cursor to tune
        value = ((float)x/width-0.5)*wf->sample_rate;
        sdr_waterfall_set_tuning(wf, value);
        prelight = P_TUNING;
        dirty = TRUE;
    }
    
    if (wf->drag == P_BANDSPREAD) {
        // right-drag for fine tuning (1Hz steps)
        offset = x - wf->click_pos;
        value = wf->bandspread + (float)offset;
        sdr_waterfall_set_tuning(wf, value);
        prelight = P_TUNING;
        dirty = TRUE;
    }
    
    // redraw if the prelight has changed
    if (prelight != wf->prelight) {
        dirty = TRUE;
    }
    if (dirty) {
        gtk_widget_queue_draw(widget);
        wf->prelight = prelight;
    }
}

static gboolean sdr_waterfall_button_press(GtkWidget *widget, GdkEventButton *event) {
    // detect button presses
    // there are four distinct cases to check for
    // click off the whole cursor = jump tuning
    // click on the cursor bounding box but not on a cursor = do nothing but allow drag
    // click on cursor hairline = allow drag (done)
    // right-click = bandspread (done)
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    switch (event->button) {
        case 1:
            // clicking off the cursor should jump the tuning to wherever we clicked
            if(wf->prelight == P_NONE) {
                wf->prelight = P_TUNING;
                sdr_waterfall_set_tuning(wf, ((float)event->x/widget->allocation.width-0.5)*wf->sample_rate);
            }
            wf->drag = wf->prelight;    // maybe the cursor is on something
            wf->click_pos = event->x;
            break;
        case 3:
            wf->prelight = P_TUNING;
            wf->drag = P_BANDSPREAD;
            wf->click_pos = event->x;
            wf->bandspread = gtk_adjustment_get_value(wf->tuning);
            gtk_widget_queue_draw(widget);
            break;
    }
}

static gboolean sdr_waterfall_button_release(GtkWidget *widget, GdkEventButton *event) {
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    wf->click_pos=0;
    if (wf->drag == P_BANDSPREAD) {
        wf->prelight = P_NONE;
        gtk_widget_queue_draw(widget);
    }
    wf->drag = P_NONE;
    return FALSE;
}

static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event) {

    SDRWaterfall *wf = SDR_WATERFALL(widget);
    int width = widget->allocation.width;
    int height = widget->allocation.height;
    
    int cursor;
    
    cairo_t *cr = gdk_cairo_create (widget->window);
    cairo_surface_t *pix;

    pix = cairo_image_surface_create_for_data (wf->pixels,
        CAIRO_FORMAT_RGB24,
        width,
        height,
        (4*wf->fft_size)); // FIXME there's a function for stride
                                               
    cairo_set_source_surface (cr, pix, 0, 0);
    cairo_paint(cr);
    // cursor is translucent when "off", opaque when prelit
    cursor = wf->cursor_pos;
    if (wf->prelight == P_TUNING) {
        cairo_set_source_rgba(cr, 1, 0, 0, 1); // red for cursor
    } else {
        cairo_set_source_rgba(cr, 1, 0, 0, 0.5); // red for cursor
    }
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, 0.5f+cursor, 0); // must be offset by a half-pixel for a single line
    cairo_line_to(cr, 0.5f+cursor, height);
    cairo_stroke(cr);
    
    // horizontal "scan bar" in lieu of scrolling all those pixels
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgba(cr, 1, 1, 0, 1);
    cairo_move_to(cr, 0, 0.5+wf->scroll_pos);
    cairo_line_to(cr, width, 0.5+wf->scroll_pos);
    cairo_stroke(cr);
    
    cairo_destroy (cr);    
    return FALSE;
}

void sdr_waterfall_update(GtkWidget *widget, guchar *row) {
    // bang a bunch of pixels onto the current row, update and wrap if need be
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    memcpy(wf->pixels+4*wf->fft_size*wf->scroll_pos, row, 4*wf->fft_size);
    wf->scroll_pos++;
    if (wf->scroll_pos >= widget->allocation.height) wf->scroll_pos = 0;
    gtk_widget_queue_draw(widget);
    
}

/* accessor functions */

float sdr_waterfall_get_tuning(SDRWaterfall *wf) {
    return wf->tuning->value;
}
float sdr_waterfall_get_lowpass(SDRWaterfall *wf) {
    return wf->lp_tune->value;
}
float sdr_waterfall_get_highpass(SDRWaterfall *wf) {
    return wf->hp_tune->value;
}

void sdr_waterfall_set_tuning(SDRWaterfall *wf, gdouble value) {
    gtk_adjustment_set_value(wf->tuning, value);
}


