/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others

	gui.c
	set up and draw the GUI elements

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
#include <complex.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "sdr.h"
#include "waterfall.h"
#include "colourmap.h"

extern sdr_data_t *sdr;

// these are global so that the gui_update routine can fiddle with them
static GtkWidget *label;
static SDRWaterfall *wfdisplay;

static gboolean gui_update_waterfall(GtkWidget *widget) {
	// copy the current block of samples to a buffer
	// calculate the FFT and scale, eventually returning
	// an array of pixels to paint to the waterfall
	// FIXME sane variable names, fewer magic numbers
	int i, j, p, hi;
	gdouble wi, y;

	fftw_complex z;
	guchar data[sdr->fft_size*4];
	static gfloat oldy[8192];
	gfloat filt = 0.99;
	gint32 colour;
	fft_data_t *fft= sdr->fft;

	// copy the block of samples to a buffer
	// we can then apply a window function to it
	memmove(fft->windowed, fft->samples, sizeof(double complex)*(sdr->fft_size));

	// window it
	for (i=0; i<sdr->fft_size; i++) {
		// Hamming function
		wi = 0.54 - 0.46 * cos(2.0 * M_PI * i/sdr->fft_size);
		// Blackman function, better strong-signal performance but more computationally expensive
		//wi = 0.42 - 0.5 * cos(2.0f * M_PI * i / sdr->fft_size) + 0.08 * cos(4.0f * M_PI * i / sdr->fft_size);
		fft->windowed[i] *= wi + I * wi;
	}

	fftw_execute(fft->plan);
	fft->status=EMPTY;
	fft->index=0;

	hi = sdr->fft_size/2;
	j=0;

	for(i=0; i<sdr->fft_size; i++) {
		p=i;
		if (p<hi) p=p+hi; else p=p-hi;
		z = fft->out[p];	 // contains the FFT data
		y=10*cabs(z);
		y = (y*filt) + (oldy[i]*(1-filt));
		oldy[i]=y;
		//y = (float)rand()/(float)(RAND_MAX);
		y = CLAMP(y , 0, 1.0);
		colour = colourmap[(int)(255*y)];

		//colour = colourmap[i%256];

		data[j++] = (colour>>8)&0xff;
		data[j++] = (colour>>16)&0xff;
		data[j++] = colour>>24;
		data[j++] = 255;
	}

	sdr_waterfall_update(widget, data);

	y = (2000-sdr->agc_gain)/2000;
	if (y<0) y = 0;
	if (y>1) y = 1;
	y=y*y;
	//sdr_smeter_set_level(SDR_SMETER(meter), y);
	return TRUE;
}

static void tuning_changed(GtkWidget *widget, gpointer psdr) {
	sdr_data_t *sdr;
	char l[256];
	sdr = (sdr_data_t *) psdr;
	float tune = gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));

	sdr->loPhase = cexp((I * -2.0 * 3.14159 * tune) / sdr->sample_rate);
	sprintf(l, "<span size=\"large\">%4.5f</span>",(sdr->centre_freq/1000000.0f)+(tune/1000000));
	gtk_label_set_markup(GTK_LABEL(label), l);
}

static void filter_clicked(GtkWidget *widget, gpointer psdr) {
	// adjust filter settings in combobox

	// sdr_data_t *sdr = (sdr_data_t *) psdr;	// FIXME needed?
	gint state = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	switch (state) {
		case 0:
			sdr_waterfall_set_lowpass(wfdisplay, 3400.0f);
			sdr_waterfall_set_highpass(wfdisplay, 300.0f);
			break;
		case 1:
			sdr_waterfall_set_lowpass(wfdisplay, 1200.0f);
			sdr_waterfall_set_highpass(wfdisplay, 500.0f);
			break;
	}
}

static void filter_changed(GtkWidget *widget, gpointer psdr) {
	sdr_data_t *sdr = (sdr_data_t *) psdr;
	gdouble lowpass = gtk_adjustment_get_value(GTK_ADJUSTMENT(sdr->lp_tune));
	gdouble highpass = gtk_adjustment_get_value(GTK_ADJUSTMENT(sdr->hp_tune));
	filter_fir_set_response(sdr->filter, sdr->sample_rate, highpass-lowpass, lowpass+(highpass-lowpass)/2);
}

static void mode_changed(GtkWidget *widget, gpointer psdr) {
	sdr_data_t *sdr = (sdr_data_t *) psdr;
	gint state = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	switch(state) {
		case SDR_LSB:
		 sdr->mode = SDR_LSB;
		 SDR_WATERFALL(wfdisplay)->mode = SDR_LSB;
		 break;
		case SDR_USB:
		 sdr->mode = SDR_USB;
		 SDR_WATERFALL(wfdisplay)->mode = SDR_USB;
			break;
	}
	sdr_waterfall_filter_cursors(SDR_WATERFALL(wfdisplay));
}

static void agc_changed(GtkWidget *widget, gpointer psdr) {
	sdr_data_t *sdr = (sdr_data_t *) psdr;

	gint state = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	switch (state) {
		case 0:
			sdr->agc_speed = 0.005;
			break;
		case 1:
			sdr->agc_speed = 0.001;
			break;
		case 2:
			sdr->agc_speed = -1.0;
			break;
	}
}

void gui_display(sdr_data_t *sdr)
{
	GtkWidget *mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *filter_combo;
	GtkWidget *mode_combo;
	GtkWidget *agc_combo;

	float tune_max;

	gtk_window_set_title(GTK_WINDOW(mainWindow), "lysdr");
	g_signal_connect(mainWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(mainWindow), vbox);

	// tuning scale
	tune_max = (float)sdr->sample_rate;
	sdr->tuning = gtk_adjustment_new(-1, -tune_max/2, tune_max/2, 10, 100, 0);

	sdr->lp_tune = gtk_adjustment_new(3400, 300, 9000, 10, 100, 0); // pretty arbitrary limits
	sdr->hp_tune = gtk_adjustment_new(300, 25, 3400, 10, 100, 0); // pretty arbitrary limits

	// buttons etc
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

	agc_combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(agc_combo), NULL, "Fast");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(agc_combo), NULL, "Slow");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(agc_combo), NULL, "Lock");
	gtk_combo_box_set_active(GTK_COMBO_BOX(agc_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox), agc_combo, TRUE, TRUE, 0);

	// VFO readout
	label = gtk_label_new (NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_label_set_markup(GTK_LABEL(label), "<tt>VFO</tt>");

	filter_combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_combo), NULL, "Wide");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_combo), NULL, "Narrow");
	gtk_combo_box_set_active(GTK_COMBO_BOX(filter_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox), filter_combo, TRUE, TRUE, 0);

	mode_combo = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo), NULL, "LSB");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(mode_combo), NULL, "USB");
	gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox), mode_combo, TRUE, TRUE, 0);

	wfdisplay = sdr_waterfall_new(GTK_ADJUSTMENT(sdr->tuning), GTK_ADJUSTMENT(sdr->lp_tune), GTK_ADJUSTMENT(sdr->hp_tune), sdr->sample_rate, sdr->fft_size);
	filter_changed(GTK_WIDGET(wfdisplay), sdr);

	// common softrock frequencies
	// 160m =  1844250
	// 80m  =  3528000
	// 40m  =  7056000
	// 30m  = 10125000
	// 20m  = 14075000
	// 15m  = 21045000

	gtk_widget_set_size_request(GTK_WIDGET(wfdisplay), sdr->fft_size, 300);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(wfdisplay), TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	gtk_widget_show_all(mainWindow);

	// connect handlers
	// FIXME - determine minimum update rate from jack latency
	g_timeout_add(15,  (GSourceFunc)gui_update_waterfall, (gpointer)wfdisplay);

	g_signal_connect(sdr->tuning, "value-changed", G_CALLBACK(tuning_changed), sdr);
	g_signal_connect(sdr->lp_tune, "value-changed", G_CALLBACK(filter_changed), sdr);
	g_signal_connect(sdr->hp_tune, "value-changed", G_CALLBACK(filter_changed), sdr);

	g_signal_connect(filter_combo, "changed", G_CALLBACK(filter_clicked), sdr);
	g_signal_connect(mode_combo, "changed", G_CALLBACK(mode_changed), sdr);
	g_signal_connect(agc_combo, "changed", G_CALLBACK(agc_changed), sdr);
}

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
