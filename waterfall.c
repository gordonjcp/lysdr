/*  lysdr Software Defined Radio
    (C) 2010-2011 Gordon JC Pearce MM0YEQ and others

    waterfall.c
    draw the waterfall display, and tuning cursors

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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "waterfall.h"

static GtkWidgetClass *parent_class = NULL;
G_DEFINE_TYPE (SDRWaterfall, sdr_waterfall, GTK_TYPE_DRAWING_AREA);
/*
static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event);
static gboolean sdr_waterfall_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean sdr_waterfall_button_release(GtkWidget *widget, GdkEventButton *event);
static gboolean sdr_waterfall_scroll(GtkWidget *widget, GdkEventScroll *event);
static void sdr_waterfall_realize(GtkWidget *widget);
static void sdr_waterfall_unrealize(GtkWidget *widget);
void sdr_waterfall_set_lowpass(SDRWaterfall *wf, gdouble value);
void sdr_waterfall_set_highpass(SDRWaterfall *wf, gdouble value);
*/

static void sdr_waterfall_realize(GtkWidget *widget);
static void sdr_waterfall_unrealize(GtkWidget *widget);

static void sdr_waterfall_class_init (SDRWaterfallClass *class) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
    // GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    // parent_class = gtk_type_class(GTK_TYPE_DRAWING_AREA);

    widget_class->realize = sdr_waterfall_realize;
    widget_class->unrealize = sdr_waterfall_unrealize; // hate american spelling

/*
    //widget_class->expose_event = sdr_waterfall_expose;
    widget_class->button_press_event = sdr_waterfall_button_press;
    widget_class->button_release_event = sdr_waterfall_button_release;
    widget_class->motion_notify_event = sdr_waterfall_motion_notify;
    widget_class->scroll_event = sdr_waterfall_scroll;
*/
    g_type_class_add_private (class, sizeof (SDRWaterfallPrivate));
}

static void sdr_waterfall_init (SDRWaterfall *wf) {

    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    gtk_widget_set_events(GTK_WIDGET(wf), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
    priv->prelight = P_NONE;
    priv->drag = P_NONE;
    priv->scroll_pos = 0;
    wf->centre_freq = 0;
}

static void sdr_waterfall_realize(GtkWidget *widget) {
    // Create various assets for the widget
    // for example, the pixel area for storing the waterfall

    SDRWaterfall *wf;
    SDRWaterfallPrivate *priv;
    GtkAllocation allocation;

    cairo_t *cr;

    g_return_if_fail(SDR_IS_WATERFALL(widget));

    // chain up to parent class, and get internal values
    GTK_WIDGET_CLASS(sdr_waterfall_parent_class)->realize(widget);
    wf = SDR_WATERFALL(widget);
    priv = G_TYPE_INSTANCE_GET_PRIVATE(widget, SDR_TYPE_WATERFALL, SDRWaterfallPrivate);

    gtk_widget_get_allocation(widget, &allocation);

    // FIXME make these more consistent
    // wf_height should be the height of the waterfall, with the overall
    // window being larger to allow for the scale
    wf->width = allocation.width;
    wf->wf_height = allocation.height;

    // create cairo surfaces for scale and waterfall buffer
    wf->pixels = cairo_image_surface_create(CAIRO_FORMAT_RGB24, wf->width,
       wf->wf_height);
    cr = cairo_create(wf->pixels);

    // black it out
    cairo_rectangle(cr, 0,0, wf->width, wf->wf_height);
    cairo_set_source_rgb(cr, 0, 0, 0); // solid black
    cairo_paint(cr);
    cairo_destroy(cr);

    g_mutex_init(&priv->mutex);
}

static void sdr_waterfall_unrealize(GtkWidget *widget) {

  SDRWaterfall *wf;
  SDRWaterfallPrivate *priv;

    wf = SDR_WATERFALL(widget);
    priv = G_TYPE_INSTANCE_GET_PRIVATE(widget, SDR_TYPE_WATERFALL, SDRWaterfallPrivate);

    // ensure that the pixel buffer is freed
    if (wf->pixels) {
      cairo_surface_destroy(wf->pixels);
    }

    if (wf->scale) {
      cairo_surface_destroy(wf->scale);
    }

    g_mutex_clear(&priv->mutex);
    GTK_WIDGET_CLASS(sdr_waterfall_parent_class)->unrealize(widget);

}

SDRWaterfall *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune,
   GtkAdjustment *hp_tune, gint sample_rate, gint fft_size) {
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
    /*
        // signals for when the adjustments change
        g_signal_connect (tuning, "value-changed",
            G_CALLBACK (sdr_waterfall_tuning_changed), wf);
        g_signal_connect (lp_tune, "value-changed",
            G_CALLBACK (sdr_waterfall_lowpass_changed), wf);
        g_signal_connect (hp_tune, "value-changed",
            G_CALLBACK (sdr_waterfall_highpass_changed), wf);

    */

        return wf;

}

static gboolean sdr_waterfall_draw(GtkWidget *widget, cairo_t *cr) {
  SDRWaterfall *wf;
  SDRWaterfallPrivate *priv;

    wf = SDR_WATERFALL(widget);
    priv = G_TYPE_INSTANCE_GET_PRIVATE(widget, SDR_TYPE_WATERFALL, SDRWaterfallPrivate);

    cairo_rectangle(cr, 0, 0, wf->width, wf->wf_height);
    cairo_set_source_surface(cr, wf->pixels, 0,0);

    cairo_paint(cr);

}
