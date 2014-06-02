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

// A bit of a hack to swap x,y arguments around if using vertical orientation,
// helps avoid a lot of switch statements and having multiple copies of code.
// Also makes it possible to move the scale bar to the top in horizontal mode.
#define wf_transpose(x, y)						\
    ((wf->orientation==WF_O_VERTICAL) ? (x) : (y)+SCALE_WIDTH),		\
	((wf->orientation==WF_O_VERTICAL) ? (y) : wf->width-(x))
#define wf_swap(x, y)							\
    ((wf->orientation==WF_O_VERTICAL) ? (x) : (y)),			\
	((wf->orientation==WF_O_VERTICAL) ? (y) : (x))
#define wf_remap(x, y)							\
    ((wf->orientation==WF_O_VERTICAL) ? (x) : (y)+SCALE_WIDTH),		\
	((wf->orientation==WF_O_VERTICAL) ? (y) : (x))
#define wf_rectangle(x, y, xsize, ysize)				\
    wf_transpose((x)+((wf->orientation==WF_O_VERTICAL)?0:(xsize)),y),	\
	((wf->orientation==WF_O_VERTICAL) ? (xsize) : (ysize)),			\
	((wf->orientation==WF_O_VERTICAL) ? (ysize) : (xsize))	\

static GtkWidgetClass *parent_class = NULL;
G_DEFINE_TYPE (SDRWaterfall, sdr_waterfall, GTK_TYPE_DRAWING_AREA);

static gboolean sdr_waterfall_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event);
static gboolean sdr_waterfall_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean sdr_waterfall_button_release(GtkWidget *widget, GdkEventButton *event);
static gboolean sdr_waterfall_scroll(GtkWidget *widget, GdkEventScroll *event);
static void sdr_waterfall_realize(GtkWidget *widget);
static void sdr_waterfall_unrealize(GtkWidget *widget);
void sdr_waterfall_set_lowpass(SDRWaterfall *wf, gdouble value);
void sdr_waterfall_set_highpass(SDRWaterfall *wf, gdouble value);

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
    widget_class->scroll_event = sdr_waterfall_scroll;

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

void sdr_waterfall_filter_cursors(SDRWaterfall *wf) {
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    gint width = wf->width;

    // FIXME - work out best place to put the enum
    // FIXME - use accessors for filters rather than gtk_adjustment_get_value
    switch(wf->mode) {
        case 0:
            priv->lp_pos = priv->cursor_pos - (width*(gtk_adjustment_get_value(wf->lp_tune)/wf->sample_rate));
            priv->hp_pos = priv->cursor_pos - (width*(gtk_adjustment_get_value(wf->hp_tune)/wf->sample_rate));
            break;
        case 1:
            priv->lp_pos = priv->cursor_pos + (width*(gtk_adjustment_get_value(wf->lp_tune)/wf->sample_rate));
            priv->hp_pos = priv->cursor_pos + (width*(gtk_adjustment_get_value(wf->hp_tune)/wf->sample_rate));
            break;
    }

}

static void sdr_waterfall_realize(GtkWidget *widget) {
    // here we handle things that must happen once the widget has a size
    SDRWaterfall *wf;
//    GtkAllocation *allocation;
    cairo_t *cr;
    gint i, j, scale, width;
    char s[10];

    g_return_if_fail(SDR_IS_WATERFALL(widget));

    wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);

    // chain up so we even *have* the size;
    GTK_WIDGET_CLASS(parent_class)->realize(widget);

    // save width and height to clamp rendering size
    // this just segfaults
    //gtk_widget_get_allocation(widget, allocation);
    //width = allocation->width;
    //wf->width = width;
    //wf->wf_height = allocation->height - SCALE_HEIGHT;

    // do it the non-Gtk3-friendly way
    switch (wf->orientation) {
    case WF_O_VERTICAL:
	width = widget->allocation.width;
	wf->wf_height = widget->allocation.height - SCALE_HEIGHT;
	break;
    case WF_O_HORIZONTAL:
	width = widget->allocation.height;
	wf->wf_height = widget->allocation.width - SCALE_WIDTH;
	break;
    }
    wf->width = width;

    // FIXME we do this a lot, maybe it should be a function
    // maybe we don't need it, since we poke the tuning adjustment
    priv->cursor_pos = width * (0.5+(gtk_adjustment_get_value(wf->tuning)/wf->sample_rate));
    // FIXME investigate cairo surfaces and speed
    wf->pixmap = gdk_pixmap_new(gtk_widget_get_window(widget), wf_swap(width, wf->wf_height), -1);

    // clear the waterfall pixmap to black
    // not sure if there's a better way to do this
    cr = gdk_cairo_create (wf->pixmap);
    //cairo_rectangle(cr, 0, 0, wf_remap(width, wf->wf_height));
    cairo_rectangle(cr, wf_rectangle(0, 0, width, wf->wf_height));
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    sdr_waterfall_set_scale(widget, wf->centre_freq);

    g_mutex_init(&priv->mutex);
    gtk_adjustment_value_changed(wf->tuning);
}

static void sdr_waterfall_unrealize(GtkWidget *widget) {
    // ensure that the pixel buffer is freed
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);

    g_object_unref(wf->pixmap); // we should definitely have a pixmap
    if (wf->scale) // we might not have a scale
        g_object_unref(wf->scale);

    g_mutex_clear(&priv->mutex);
    GTK_WIDGET_CLASS(parent_class)->unrealize(widget);
}

static void sdr_waterfall_tuning_changed(GtkWidget *widget, gpointer *p) {
    // if the tuning adjustment changes, ensure the pointer is recalculated
    SDRWaterfall *wf = SDR_WATERFALL(p);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    gdouble value = gtk_adjustment_get_value(wf->tuning);   // FIXME - get from *widget?
    priv->cursor_pos = width * (0.5+(value/wf->sample_rate));
    // need to update the filter positions too
    //priv->lp_pos = priv->cursor_pos - (width*(wf->lp_tune->value/wf->sample_rate));
    //priv->hp_pos = priv->cursor_pos - (width*(wf->hp_tune->value/wf->sample_rate));
    sdr_waterfall_filter_cursors(wf);
    gtk_widget_queue_draw(GTK_WIDGET(wf));
}

static void sdr_waterfall_lowpass_changed(GtkWidget *widget, gpointer *p) {
    SDRWaterfall *wf = SDR_WATERFALL(p);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    gdouble value = gtk_adjustment_get_value(wf->lp_tune);
    sdr_waterfall_filter_cursors(wf);
    //priv->lp_pos = priv->cursor_pos - (width*(value/wf->sample_rate));
    gtk_widget_queue_draw(GTK_WIDGET(wf));
}

static void sdr_waterfall_highpass_changed(GtkWidget *widget, gpointer *p) {
    SDRWaterfall *wf = SDR_WATERFALL(p);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    gdouble value = gtk_adjustment_get_value(wf->hp_tune);
    sdr_waterfall_filter_cursors(wf);
    //priv->hp_pos = priv->cursor_pos - (width*(value/wf->sample_rate));
    gtk_widget_queue_draw(GTK_WIDGET(wf));
}

SDRWaterfall *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune, gint sample_rate, gint fft_size) {
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
    g_signal_connect (lp_tune, "value-changed",
        G_CALLBACK (sdr_waterfall_lowpass_changed), wf);
    g_signal_connect (hp_tune, "value-changed",
        G_CALLBACK (sdr_waterfall_highpass_changed), wf);
    return wf;

}

void sdr_waterfall_set_scale(GtkWidget *widget, gint centre_freq) {
    // draw the scale to a handy pixmap
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    gint width = wf->width;
    cairo_t *cr;
    gint i, j, scale;
    gchar s[10];

    wf->centre_freq = centre_freq;

    if (!wf->scale) {
	switch (wf->orientation) {
	case WF_O_VERTICAL:
	    wf->scale = gdk_pixmap_new(gtk_widget_get_window(widget), width, SCALE_HEIGHT, -1);
	    break;
	case WF_O_HORIZONTAL:
	    wf->scale = gdk_pixmap_new(gtk_widget_get_window(widget), SCALE_WIDTH, width, -1);
	    break;
	}
    }

    cr = gdk_cairo_create(wf->scale);
    switch (wf->orientation) {
    case WF_O_VERTICAL:
	cairo_rectangle(cr, 0, 0, width, SCALE_HEIGHT);
	cairo_clip(cr);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
	cairo_set_source_rgb(cr, 1, 0, 0);
	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, width, 0);
	cairo_stroke(cr);
	cairo_set_line_width(cr, 1);

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
	break;
    case WF_O_HORIZONTAL:
	cairo_rectangle(cr, 0, 0, SCALE_WIDTH, width);
	cairo_clip(cr);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
	cairo_set_source_rgb(cr, 1, 0, 0);
	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, 0, width);
	cairo_stroke(cr);
	cairo_set_line_width(cr, 1);

	scale = (trunc(wf->sample_rate/SCALE_TICK)+1)*SCALE_TICK;

	for (i=-scale; i<scale; i+=SCALE_TICK) {  // FIXME hardcoded
	    j = width * (0.5+((double)i/wf->sample_rate));
	    cairo_set_source_rgb(cr, 1, 0, 0);
	    cairo_move_to(cr, 0, j);
	    cairo_line_to(cr, 8, j);
	    cairo_stroke(cr);
	    cairo_move_to(cr, 12, j+4);
	    cairo_set_source_rgb(cr, .75, .75, .75);
	    sprintf(s, "%4.3f", (wf->centre_freq/1000000.0f)+(i/1000000.0f));
	    cairo_show_text(cr,s);
	}
	break;
    }
    cairo_destroy(cr);

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
        sdr_waterfall_set_lowpass(wf, (float)value);
        prelight = P_LOWPASS;
    }

    if (priv->drag == P_HIGHPASS) {
        if (wf->mode == 0) {
            offset = priv->cursor_pos - x;
        } else {
            offset = x - priv->cursor_pos;
        }
        value = ((float)offset/width)*wf->sample_rate;
        sdr_waterfall_set_highpass(wf, (float)value);
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
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
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

static gboolean sdr_waterfall_scroll(GtkWidget *widget, GdkEventScroll *event) {
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    float tuning = gtk_adjustment_get_value(wf->tuning);
    float step;

    if (event->state & GDK_MOD1_MASK) {
        step = 10000.0;
    } else if (event->state & GDK_SHIFT_MASK) {
        step = 1000.0;
    } else {
        step = 100.0;
    }

    if (wf->orientation == WF_O_HORIZONTAL)
	    step = -step;

    switch (event->direction) {
        case GDK_SCROLL_UP:
            tuning -= step;
            break;
        case GDK_SCROLL_DOWN:
            tuning += step;
            break;
        default:
            break;
    }

    sdr_waterfall_set_tuning(wf, tuning);

    return FALSE;
}

static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event) {

    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);
    int width = wf->width;
    int height = wf->wf_height;
    int cursor;

    cairo_t *cr = gdk_cairo_create (gtk_widget_get_window(widget));

    if (wf->scale) {    // might not have a scale
	switch (wf->orientation) {
	case WF_O_VERTICAL:
	    gdk_cairo_set_source_pixmap(cr, wf->scale, 0, height);
	    break;
	case WF_O_HORIZONTAL:
	    gdk_cairo_set_source_pixmap(cr, wf->scale, 0, 0);
	    break;
	}
        cairo_paint(cr);
    }

    // clip region is waterfall size
    cairo_rectangle(cr, wf_rectangle(0, 0, width, height));
    cairo_clip(cr);

    g_mutex_lock(&priv->mutex);
        gdk_cairo_set_source_pixmap(cr, wf->pixmap, wf_remap(0, -priv->scroll_pos));
        cairo_paint(cr);
        gdk_cairo_set_source_pixmap(cr, wf->pixmap, wf_remap(0, height-priv->scroll_pos));
	cairo_paint(cr);
    g_mutex_unlock(&priv->mutex);

    // cursor is translucent when "off", opaque when prelit
    cursor = priv->cursor_pos;
    if (priv->prelight == P_TUNING) {
        cairo_set_source_rgba(cr, 1, 0, 0, 1); // red for cursor
    } else {
        cairo_set_source_rgba(cr, 1, 0, 0, 0.5); // red for cursor
    }
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, wf_transpose(0.5f+cursor, 0)); // must be offset by a half-pixel for a single line
    cairo_line_to(cr, wf_transpose(0.5f+cursor, height));
    cairo_stroke(cr);

    // filter cursor
    cairo_set_source_rgba(cr, 0.5, 0.5, 0, 0.25);
    cairo_rectangle(cr,
		    wf_rectangle(MIN(priv->hp_pos, priv->lp_pos), 0,
				 abs(priv->lp_pos - priv->hp_pos), height));
    cairo_fill(cr);

    // side rails
    // lowpass
    cairo_set_line_width(cr, 1);
    if (priv->prelight == P_LOWPASS) {
        cairo_set_source_rgba(cr, 1, 1, 0.5, 0.75);
    } else  {
        cairo_set_source_rgba(cr, 1, 1, 0.5, 0.25);

    }

    cairo_move_to(cr, wf_transpose(0.5 + priv->lp_pos, 0));
    cairo_line_to(cr, wf_transpose(0.5 + priv->lp_pos, height));
    cairo_stroke(cr);

    // highpass
    if (priv->prelight == P_HIGHPASS) {
        cairo_set_source_rgba(cr, 1, 1, 0.5, 0.75);
    } else  {
        cairo_set_source_rgba(cr, 1, 1, 0.5, 0.25);
    }
    cairo_move_to(cr, wf_transpose(0.5 + priv->hp_pos-1, 0));
    cairo_line_to(cr, wf_transpose(0.5 + priv->hp_pos-1, height));
    cairo_stroke(cr);

    cairo_destroy (cr);

    return FALSE;
}

void sdr_waterfall_update(GtkWidget *widget, guchar *row) {
    // bang a bunch of pixels onto the current row, update and wrap if need be
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    SDRWaterfallPrivate *priv = SDR_WATERFALL_GET_PRIVATE(wf);

    //return;
    cairo_t *cr = gdk_cairo_create (wf->pixmap);
    cairo_surface_t *s_row = cairo_image_surface_create_for_data (
        row,
        CAIRO_FORMAT_RGB24,
	wf_swap(wf->fft_size, 1),
	(wf->orientation == WF_O_VERTICAL) ? (4*wf->fft_size) : 4
    );

    g_mutex_lock(&priv->mutex);
	switch (wf->orientation) {
	case WF_O_VERTICAL:
	    cairo_set_source_surface (cr, s_row, 0, priv->scroll_pos);
	    break;
	case WF_O_HORIZONTAL:
	    cairo_set_source_surface (cr, s_row, priv->scroll_pos, 0);
	    break;
	}
    cairo_paint(cr);
    g_mutex_unlock(&priv->mutex);

    priv->scroll_pos++;
    if (priv->scroll_pos >= wf->wf_height) priv->scroll_pos = 0;

    cairo_surface_destroy(s_row);
    cairo_destroy(cr);
    gtk_widget_queue_draw(widget);

}

/* accessor functions */
float sdr_waterfall_get_tuning(SDRWaterfall *wf) {
    return gtk_adjustment_get_value(wf->tuning);
}
float sdr_waterfall_get_lowpass(SDRWaterfall *wf) {
    return gtk_adjustment_get_value(wf->lp_tune);
}
float sdr_waterfall_get_highpass(SDRWaterfall *wf) {
    return gtk_adjustment_get_value(wf->hp_tune);
}

void sdr_waterfall_set_tuning(SDRWaterfall *wf, gdouble value) {
    gtk_adjustment_set_value(wf->tuning, value);
}

void sdr_waterfall_set_lowpass(SDRWaterfall *wf, gdouble value) {
    gtk_adjustment_set_value(wf->lp_tune, value);
    gtk_adjustment_set_upper(wf->hp_tune, value);
}
void sdr_waterfall_set_highpass(SDRWaterfall *wf, gdouble value) {
    gtk_adjustment_set_value(wf->hp_tune, value);
    gtk_adjustment_set_lower(wf->lp_tune, value);
}

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
