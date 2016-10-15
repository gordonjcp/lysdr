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
static gboolean sdr_waterfall_scroll(GtkWidget *widget, GdkEventScroll *event);
void sdr_waterfall_set_lowpass(SDRWaterfall *wf, gdouble value);
void sdr_waterfall_set_highpass(SDRWaterfall *wf, gdouble value);
*/

static void sdr_waterfall_realize(GtkWidget *widget);
static void sdr_waterfall_unrealize(GtkWidget *widget);
static gboolean sdr_waterfall_draw(GtkWidget *widget, cairo_t *cr);

static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gboolean sdr_waterfall_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean sdr_waterfall_button_release(GtkWidget *widget, GdkEventButton *event);


static void sdr_waterfall_class_init (SDRWaterfallClass *class) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
    // GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    // parent_class = gtk_type_class(GTK_TYPE_DRAWING_AREA);

    widget_class->realize = sdr_waterfall_realize;
    widget_class->unrealize = sdr_waterfall_unrealize; // hate american spelling
    widget_class->draw = sdr_waterfall_draw;
    widget_class->motion_notify_event = sdr_waterfall_motion_notify;


    widget_class->button_press_event = sdr_waterfall_button_press;
    widget_class->button_release_event = sdr_waterfall_button_release;
/*    widget_class->motion_notify_event = sdr_waterfall_motion_notify;
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
    cairo_fill(cr);
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

static void sdr_waterfall_tuning_changed(GtkWidget *widget, gpointer *p) {
    // if the tuning adjustment changes, ensure the pointer is recalculated
    SDRWaterfall *wf = SDR_WATERFALL(p);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    gdouble value =  gtk_adjustment_get_value(wf->tuning);   // FIXME - get from *widget?

    priv->cursor_pos = width * (0.5+(value/wf->sample_rate));

    // need to update the filter positions too
    //priv->lp_pos = priv->cursor_pos - (width*(wf->lp_tune->value/wf->sample_rate));
    //priv->hp_pos = priv->cursor_pos - (width*(wf->hp_tune->value/wf->sample_rate));
    //sdr_waterfall_filter_cursors(wf);
    gtk_widget_queue_draw(GTK_WIDGET(wf));
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

        // signals for when the adjustments change
        g_signal_connect (tuning, "value-changed",
            G_CALLBACK (sdr_waterfall_tuning_changed), wf);
/*        g_signal_connect (lp_tune, "value-changed",
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

    unsigned int height = wf->wf_height;
    unsigned int cursor;

    cairo_rectangle(cr, 0, 0, wf->width, wf->wf_height);
    cairo_clip(cr);

    //g_mutex_lock(&priv->mutex);
    cairo_set_source_surface(cr, wf->pixels, 0, -priv->scroll_pos);
    cairo_paint(cr);
    cairo_set_source_surface(cr, wf->pixels, 0, wf->wf_height-priv->scroll_pos);
	  cairo_paint(cr);
    //g_mutex_unlock(&priv->mutex);

    // draw the tuning cursor
    // cursor bar is translucent when "off", opaque when prelit
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


}




void sdr_waterfall_update(GtkWidget *widget, guchar *row) {
    // bang a bunch of pixels onto the current row, update and wrap if need be
      SDRWaterfall *wf;
      SDRWaterfallPrivate *priv;
      wf = SDR_WATERFALL(widget);
      priv = G_TYPE_INSTANCE_GET_PRIVATE(widget, SDR_TYPE_WATERFALL, SDRWaterfallPrivate);

    cairo_t *cr = cairo_create (wf->pixels);

  //  printf("stride=%d\n", cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, wf->fft_size));

    cairo_surface_t *s_row = cairo_image_surface_create_for_data(row,
      CAIRO_FORMAT_RGB24, wf->fft_size, 1, 4096);

    //g_mutex_lock(&priv->mutex);

    //cairo_rectangle(cr, 0, 0, wf->width, wf->wf_height);

    unsigned char *data;
    //cairo_surface_flush(s_row);
    data = cairo_image_surface_get_data(s_row);

    cairo_set_source_surface (cr, s_row, 0, priv->scroll_pos);
    //cairo_set_source_rgb(cr, row[0]/256.0, .2, .3);
    cairo_fill(cr);
    cairo_paint(cr);
  //  g_mutex_unlock(&priv->mutex);

    priv->scroll_pos++;
    if (priv->scroll_pos >= wf->wf_height) priv->scroll_pos = 0;

    cairo_surface_destroy(s_row);
    cairo_destroy(cr);
    gtk_widget_queue_draw(widget);
}

static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event) {

    gint prelight = P_NONE;
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);

    gdouble value;
    gint offset;
    gboolean dirty = FALSE;

    gint width = wf->width;
    gint x = CLAMP((wf->orientation == WF_O_VERTICAL) ? event->x : (wf->width-event->y), 0, width);
    if (priv->drag == P_NONE) {
        if (WITHIN(x, priv->cursor_pos)) {
            prelight = P_TUNING;
            dirty = TRUE;
        }
        if (WITHIN(x, priv->lp_pos)) {
            prelight = P_LOWPASS;
            dirty = TRUE;
        }
        if (WITHIN(x, priv->hp_pos)) {
            prelight = P_HIGHPASS;
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

    if (priv->drag == P_LOWPASS) {
        if (wf->mode == 0) {
            offset = priv->cursor_pos - x;
        } else {
            offset = x - priv->cursor_pos;
        }
        value = ((float)offset/width)*wf->sample_rate;
        //sdr_waterfall_set_lowpass(wf, (float)value);
        prelight = P_LOWPASS;
    }

    if (priv->drag == P_HIGHPASS) {
        if (wf->mode == 0) {
            offset = priv->cursor_pos - x;
        } else {
            offset = x - priv->cursor_pos;
        }
        value = ((float)offset/width)*wf->sample_rate;
        //sdr_waterfall_set_highpass(wf, (float)value);
        prelight = P_HIGHPASS;
    }

    // redraw if the prelight has changed
    if (prelight != priv->prelight) {
        dirty = TRUE;
    }
    if (dirty) {
        gtk_widget_queue_draw(widget);
        priv->prelight = prelight;
    }
    return TRUE;
}


static gboolean sdr_waterfall_button_press(GtkWidget *widget, GdkEventButton *event) {
    // detect button presses
    // there are four distinct cases to check for
    // click off the whole cursor = jump tuning
    // click on the cursor bounding box but not on a cursor = do nothing but allow drag
    // click on cursor hairline = allow drag (done)
    // right-click = bandspread (done)
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE(widget,
      SDR_TYPE_WATERFALL, SDRWaterfallPrivate);

    gint x = (wf->orientation == WF_O_VERTICAL) ? event->x : (wf->width-event->y);
    switch (event->button) {
        case 1:
            // clicking off the cursor should jump the tuning to wherever we clicked
            if (priv->prelight == P_NONE) {
                priv->prelight = P_TUNING;
                sdr_waterfall_set_tuning(wf, ((float)x/wf->width-0.5)*wf->sample_rate);
            }
            priv->drag = priv->prelight;    // maybe the cursor is on something
            priv->click_pos = x;
            break;
        case 3:
            // right-click for bandspread tuning
            priv->prelight = P_TUNING;
            priv->drag = P_BANDSPREAD;
            priv->click_pos = x;
            priv->bandspread = gtk_adjustment_get_value(wf->tuning);
            gtk_widget_queue_draw(widget);
            break;
    }
    return TRUE;
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


// accessor functions
void sdr_waterfall_set_tuning(SDRWaterfall *wf, gdouble value) {
    gtk_adjustment_set_value(wf->tuning, value);
}
