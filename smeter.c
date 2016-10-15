/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others

	smeter.c
	draw a simple signal strength meter

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
#include <gtk/gtk.h>
#include "smeter.h"

static GtkWidgetClass *parent_class = NULL;
G_DEFINE_TYPE (SDRSMeter, sdr_smeter, GTK_TYPE_DRAWING_AREA);

static gboolean sdr_smeter_expose(GtkWidget *widget, GdkEventExpose *event);
static void sdr_smeter_size_request(GtkWidget *widget, GtkRequisition *requisition);

static void sdr_smeter_class_init (SDRSMeterClass *class) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	//parent_class = gtk_type_class(GTK_TYPE_DRAWING_AREA);

	//widget_class->expose_event = sdr_smeter_expose;
	//widget_class->size_request = sdr_smeter_size_request;
}

static void sdr_smeter_init(SDRSMeter *sm) {

}

static void sdr_smeter_size_request(GtkWidget *widget, GtkRequisition *requisition) {
	// width doesn't seem to be obeyed.
	requisition->width = 265;
	requisition->height = 20;
}

GtkWidget *sdr_smeter_new() {
	SDRSMeter *sm;
	sm = g_object_new(SDR_TYPE_SMETER, NULL);
	return GTK_WIDGET(sm);
}

static gboolean sdr_smeter_expose(GtkWidget *widget, GdkEventExpose *event) {
	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	gint pos = SDR_SMETER(widget)->level*255;

	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_source_rgb(cr, 0.93, 1, 0.93);
	cairo_move_to(cr, 5, 15);
	cairo_line_to(cr, 5, 10);
	cairo_line_to(cr, 200, 10);
	cairo_line_to(cr, 200, 15);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 1, 0.63, 0.63);
	cairo_move_to(cr, 203, 15);
	cairo_line_to(cr, 203, 10);
	cairo_line_to(cr, 258, 10);
	cairo_line_to(cr, 258, 15);
	cairo_stroke(cr);

	// draw bargraph
	cairo_set_source_rgb(cr, 0.2, 0.4, 0);
	cairo_rectangle(cr, 4, 4, 255, 4);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 0.6, 1.0, 0);
	cairo_rectangle(cr, 4, 4, pos, 4);
	cairo_fill(cr);

	cairo_destroy(cr);
	return TRUE;
}

void sdr_smeter_set_level(SDRSMeter *sm, gdouble level) {
	sm->level = level;
	gtk_widget_queue_draw(GTK_WIDGET(sm));
}

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
