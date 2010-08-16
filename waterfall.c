/* waterfall.c */
/* Copyright 2010 Gordon JC Pearce <gordon@gjcp.net> */


// http://cairographics.org/threaded_animation_with_cairo/
// http://audidude.com/?p=470
// http://audidude.com/?p=404

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
    
    g_type_class_add_private (class, sizeof (SDRWaterfallPrivate));

}

static void sdr_waterfall_init (SDRWaterfall *wf) {
    
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    gtk_widget_set_events(GTK_WIDGET(wf), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    priv->prelight = P_NONE;
    priv->drag = P_NONE;
    priv->scroll_pos = 0;
}

static void sdr_waterfall_realize(GtkWidget *widget) {
    // here we handle things that must happen once the widget has a size
    SDRWaterfall *wf;
    cairo_t *cr;
    gint i, j, scale, width;
    char s[10];
    
    g_return_if_fail(SDR_IS_WATERFALL(widget));

    wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);

    // chain up so we even *have* the size;
    GTK_WIDGET_CLASS(parent_class)->realize(widget);
    
    
    // save width and height to clamp rendering size
    width = widget->allocation.width;
    wf->width = width;
    wf->wf_height = widget->allocation.height - SCALE_HEIGHT;
    
    priv->cursor_pos = width * (0.5+(wf->tuning->value/wf->sample_rate)); 
    wf->pixmap = gdk_pixmap_new(widget->window, width, wf->wf_height, -1);
    
    // clear the waterfall pixmap to black
    cr = gdk_cairo_create (wf->pixmap);
    cairo_rectangle(cr, 0, 0, width, wf->wf_height);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    // draw the scale to a handy pixmap
    wf->scale = gdk_pixmap_new(widget->window, width, SCALE_HEIGHT, -1);
    cr = gdk_cairo_create(wf->scale);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, width, 0);
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1);
    
    // zero is width/2
    // ends are sample_rate/2

    scale = (trunc(wf->sample_rate/SCALE_TICK)+1)*SCALE_TICK;
    
    for (i=-scale; i<scale; i+=SCALE_TICK) {  // FIXME hardcoded
        j = width * (0.5+((double)i/wf->sample_rate));
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_move_to(cr, 0.5+j, 0);
        cairo_line_to(cr, 0.5+j, 8);
        cairo_stroke(cr);
        cairo_move_to(cr, j-10, 18);
        cairo_set_source_rgb(cr, .75, .75, .75);
        sprintf(s, "%4.3f", (wf->centre_freq/1000000.0f)+(i/1000000.0f));
        cairo_show_text(cr,s);
    }
    cairo_destroy(cr);    

    g_assert(priv->mutex == NULL);
    priv->mutex = g_mutex_new();

}

static void sdr_waterfall_unrealize(GtkWidget *widget) {
    // ensure that the pixel buffer is freed
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    //g_free(wf->pixels);
    g_object_unref(wf->pixmap);

    GTK_WIDGET_CLASS(parent_class)->unrealize(widget);
    g_mutex_free(priv->mutex);
}

static void sdr_waterfall_tuning_changed(GtkWidget *widget, gpointer *p) {
    // if the tuning adjustment changes, ensure the pointer is recalculated
    SDRWaterfall *wf = SDR_WATERFALL(p);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    gdouble value = gtk_adjustment_get_value(wf->tuning);
    priv->cursor_pos = width * (0.5+(value/wf->sample_rate));
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

    // assign the GtkAdjustments
    wf->tuning = tuning;
    wf->lp_tune = lp_tune;
    wf->hp_tune = hp_tune;
    // internal parameters
    wf->sample_rate = sample_rate;
    wf->fft_size = fft_size;
    
    // signals for when the adjustments change
    g_signal_connect (tuning, "value-changed",
        G_CALLBACK (sdr_waterfall_tuning_changed), wf);
    
    return GTK_WIDGET(wf);
    
}

static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event) {

    gint prelight = P_NONE; 
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    
    gdouble value;
    gint offset;
    gboolean dirty = FALSE;

    gint width = wf->width;    
    gint x = CLAMP(event->x, 0, width);
    if (priv->drag == P_NONE) {    
        if (WITHIN(x, priv->cursor_pos)) {
            prelight = P_TUNING;
            dirty = TRUE;
        }
    }
    if (priv->drag == P_TUNING) {
        // drag cursor to tune
        value = ((float)x/width-0.5)*wf->sample_rate;
        sdr_waterfall_set_tuning(wf, value);
        prelight = P_TUNING;
        dirty = TRUE;
    }
    
    if (priv->drag == P_BANDSPREAD) {
        // right-drag for fine tuning (1Hz steps)
        offset = x - priv->click_pos;
        value = priv->bandspread + (float)offset;
        sdr_waterfall_set_tuning(wf, value);
        prelight = P_TUNING;
        dirty = TRUE;
    }
    
    // redraw if the prelight has changed
    if (prelight != priv->prelight) {
        dirty = TRUE;
    }
    if (dirty) {
        gtk_widget_queue_draw(widget);
        priv->prelight = prelight;
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
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    switch (event->button) {
        case 1:
            // clicking off the cursor should jump the tuning to wherever we clicked
            if (priv->prelight == P_NONE) {
                priv->prelight = P_TUNING;
                sdr_waterfall_set_tuning(wf, ((float)event->x/wf->width-0.5)*wf->sample_rate);
            }
            priv->drag = priv->prelight;    // maybe the cursor is on something
            priv->click_pos = event->x;
            break;
        case 3:
            // right-click for bandspread tuning
            priv->prelight = P_TUNING;
            priv->drag = P_BANDSPREAD;
            priv->click_pos = event->x;
            priv->bandspread = gtk_adjustment_get_value(wf->tuning);
            gtk_widget_queue_draw(widget);
            break;
    }
}

static gboolean sdr_waterfall_button_release(GtkWidget *widget, GdkEventButton *event) {
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    priv->click_pos=0;
    if (priv->drag == P_BANDSPREAD) {
        priv->prelight = P_NONE;
        gtk_widget_queue_draw(widget);
    }
    priv->drag = P_NONE;
    return FALSE;
}

static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event) {

    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    int height = wf->wf_height;
    
   

    int cursor;
    
    cairo_t *cr = gdk_cairo_create (widget->window);
    
    gdk_cairo_set_source_pixmap(cr, wf->scale, 0, height);
    cairo_paint(cr);
    
    // clip region is waterfall size
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_clip(cr);

    g_mutex_lock(priv->mutex);
	gdk_cairo_set_source_pixmap(cr, wf->pixmap, 0, -priv->scroll_pos);
	cairo_paint(cr);
	gdk_cairo_set_source_pixmap(cr, wf->pixmap, 0, height-priv->scroll_pos);
	cairo_paint(cr);
    g_mutex_unlock(priv->mutex);

    // cursor is translucent when "off", opaque when prelit
    cursor = priv->cursor_pos;
    if (priv->prelight == P_TUNING) {
        cairo_set_source_rgba(cr, 1, 0, 0, 1); // red for cursor
    } else {
        cairo_set_source_rgba(cr, 1, 0, 0, 0.5); // red for cursor
    }
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, 0.5f+cursor, 0); // must be offset by a half-pixel for a single line
    cairo_line_to(cr, 0.5f+cursor, height);
    cairo_stroke(cr);
    
    // filter cursor
    cairo_set_source_rgba(cr, 0.5, 0.5, 0, 0.25);
    cairo_rectangle(cr, cursor - wf->lp_tune->value / 48.875 , 0, (wf->lp_tune->value - wf->hp_tune->value) / 48.875, height);
    cairo_fill(cr);
    
    // side rails
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgba(cr, 1, 1, 0.5, 0.25);
    cairo_move_to(cr, 0.5 + (int)(cursor - wf->lp_tune->value / 48.875), 0);
    cairo_line_to(cr, 0.5 + (int)(cursor - wf->lp_tune->value / 48.875), height);
    cairo_stroke(cr);
    cairo_move_to(cr, 0.5 + (int)(cursor - wf->hp_tune->value / 48.875), 0);
    cairo_line_to(cr, 0.5 + (int)(cursor - wf->hp_tune->value / 48.875), height);
    cairo_stroke(cr);
    

    cairo_destroy (cr);

    return FALSE;
}

void sdr_waterfall_update(GtkWidget *widget, guchar *row) {
    // bang a bunch of pixels onto the current row, update and wrap if need be
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    
    cairo_t *cr = gdk_cairo_create (wf->pixmap);
    cairo_surface_t *s_row = cairo_image_surface_create_for_data (
        row,
        CAIRO_FORMAT_RGB24,
        wf->fft_size,
        1,
        (4*wf->fft_size)
    );
    
    g_mutex_lock(priv->mutex);
    cairo_set_source_surface (cr, s_row, 0, priv->scroll_pos);
    cairo_paint(cr);
    g_mutex_unlock(priv->mutex);

    priv->scroll_pos++;
    if (priv->scroll_pos >= wf->wf_height) priv->scroll_pos = 0;

    cairo_surface_destroy(s_row);
    cairo_destroy(cr);
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


