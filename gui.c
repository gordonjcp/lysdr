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

#include "sdr.h"
#include "waterfall.h"
#include "smeter.h"

extern sdr_data_t *sdr;

// these are global so that the gui_update routine can fiddle with them
static GtkWidget *label;
static GtkWidget *wfdisplay;
static GtkWidget *meter;

/*
// red hot
gint32 colourmap[] = {
0x00000000, 0x00000000, 0x00000000, 0x02000000, 0x03000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 0x07000000, 0x09000000, 0x09000000, 0x0b000000, 0x0b000000, 0x0c000000, 0x0d000000, 0x0e000000, 0x0f000000, 0x0f000000, 0x11000000, 0x11000000, 0x12000000, 0x13000000, 0x13000000, 0x15000000, 0x15000000, 0x16000000, 0x17000000, 0x18000000, 0x18000000, 0x1a000000, 0x1b000000, 0x1b000000, 0x1c000000, 0x1d000000, 0x1e000000, 0x1e000000, 0x1f000000, 0x20000000, 0x21000000, 0x22000000, 0x22000000, 0x24000000, 0x25000000, 0x25000000, 0x26000000, 0x27000000, 0x27000000, 0x29000000, 0x2a000000, 0x2a000000, 0x2b000000, 0x2c000000, 0x2d000000, 0x2e000000, 0x2e000000, 0x30000000, 0x31000000, 0x32000000, 0x32000000, 0x32000000, 0x34000000, 0x35000000, 0x36000000, 0x36000000, 0x38000000, 0x38000000, 0x39000000, 0x3a000000, 0x3b000000, 0x3b000000, 0x3c000000, 0x3d000000, 0x3e000000, 0x3f000000, 0x40000000, 0x41000000, 0x41000000, 0x42000000, 0x43000000, 0x44000000, 0x45000000, 0x46000000, 0x46000000, 0x47000000, 0x48000000, 0x49000000, 0x49000000, 0x4a000000, 0x4c000000, 0x4d000000, 0x4d000000, 0x4e000000, 0x4f000000, 0x50000000, 0x51000000, 0x52000000, 0x52000000, 0x53000000, 0x54000000, 0x55000000, 0x56000000, 0x57000000, 0x57000000, 0x58000000, 0x59000000, 0x5a000000, 0x5b000000, 0x5b000000, 0x5d000000, 0x5e000000, 0x60000000, 0x64000000, 0x66000000, 0x6a000000, 0x6d000000, 0x6f000000, 0x72000000, 0x75000000, 0x78000000, 0x7b000000, 0x7d000000, 0x80000000, 0x83000000, 0x86000000, 0x89000000, 0x8c000000, 0x8e000000, 0x92000000, 0x94000000, 0x98000000, 0x9a000000, 0x9d000000, 0xa0000000, 0xa3000000, 0xa6000000, 0xa8000000, 0xab000000, 0xae000000, 0xb1000000, 0xb4000000, 0xb7000000, 0xba000000, 0xbc030100, 0xbe070300, 0xbf0b0300, 0xc10e0500, 0xc2110600, 0xc4150800, 0xc6190800, 0xc71b0900, 0xc91f0b00, 0xcb230d00, 0xcd260e00, 0xcf290f00, 0xd12d1100, 0xd2311200, 0xd4341300, 0xd5371400, 0xd83c1500, 0xd93e1600, 0xdb421800, 0xdd461900, 0xde481a00, 0xdf4a1b00, 0xe04c1c00, 0xe14e1c00, 0xe2511d00, 0xe3531d00, 0xe4551f00, 0xe5571f00, 0xe7592000, 0xe75c2100, 0xe95e2100, 0xe9602200, 0xea632400, 0xec642400, 0xed662500, 0xee692500, 0xef6b2700, 0xf16d2800, 0xf16f2800, 0xf2722900, 0xf3732a00, 0xf5762b00, 0xf5792b00, 0xf77b2c00, 0xf87d2c00, 0xf97f2d00, 0xfa812e00, 0xfb843000, 0xfd853000, 0xfd883100, 0xfe8a3100, 0xff8e3200, 0xff923000, 0xfe972f00, 0xfe9c2d00, 0xfda12b00, 0xfda62a00, 0xfdaa2900, 0xfcaf2600, 0xfcb42600, 0xfcb82400, 0xfcbd2200, 0xfbc22100, 0xfbc62000, 0xfbca1e00, 0xfacf1d00, 0xfad41c00, 0xfad81a00, 0xfadd1900, 0xf9e11800, 0xf9e61500, 0xf8ea1400, 0xf8ef1300, 0xf8ef1b00, 0xf8f02400, 0xf8f02b00, 0xf8f03400, 0xf9f13d00, 0xf9f14400, 0xf8f14d00, 0xf9f25500, 0xf8f25e00, 0xf9f36600, 0xf8f26e00, 0xf8f37600, 0xf9f47f00, 0xf9f48600, 0xf9f48d00, 0xf9f49600, 0xf9f59e00, 0xf9f6a600, 0xf9f6ad00, 0xf8f5b500, 0xf9f6bd00, 0xf9f6c400, 0xf9f7cc00, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400
};
*/
gint32 colourmap[] = {
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08052800, 0x0b073800, 0x0e084400, 0x0f0a4f00, 0x110a5700, 0x120b5e00, 0x140c6500, 0x150d6a00, 0x150d6f00, 0x170d7400, 0x170e7800, 0x180e7b00, 0x190e7d00, 0x180f7f00, 0x190f7e00, 0x190f7f00, 0x180f8000, 0x190f8000, 0x190f8000, 0x190f8100, 0x19108200, 0x190f8200, 0x19108200, 0x19108400, 0x19108300, 0x1a0f8400, 0x1a0f8400, 0x1a108500, 0x1a108500, 0x1a0f8600, 0x1a108700, 0x1a108700, 0x1a0f8700, 0x1a108800, 0x1a108700, 0x1a108800, 0x1a108900, 0x1b108900, 0x1b108900, 0x1b108900, 0x1a108a00, 0x1b118a00, 0x1a108a00, 0x1b118b00, 0x1b108a00, 0x1b108b00, 0x1b108c00, 0x1b118c00, 0x1b108c00, 0x1b118c00, 0x1b108d00, 0x1b108c00, 0x1b118d00, 0x1b108d00, 0x1c108d00, 0x1c108e00, 0x1c118e00, 0x1b118e00, 0x1c118e00, 0x1b118f00, 0x1c108f00, 0x1b118e00, 0x1b108f00, 0x1c118f00, 0x1c108f00, 0x1c119000, 0x1c119000, 0x1c119000, 0x1c119000, 0x1c118f00, 0x1c119000, 0x1c119000, 0x1c119100, 0x1c119100, 0x1c119000, 0x1c119100, 0x1c119100, 0x1c119000, 0x1c119000, 0x1c119100, 0x1c119100, 0x1c119100, 0x1c119100, 0x1c119000, 0x1c119100, 0x1c119100, 0x1c119100, 0x1c119100, 0x1c119100, 0x1e118f00, 0x1f118f00, 0x21118d00, 0x23118c00, 0x26118b00, 0x27118a00, 0x29118800, 0x2a118700, 0x2c108600, 0x2e118500, 0x30118400, 0x31108300, 0x33108200, 0x35108000, 0x37117f00, 0x39107e00, 0x3b107d00, 0x3c107c00, 0x3e107a00, 0x40107900, 0x42107900, 0x440f7700, 0x45107600, 0x47107500, 0x49107400, 0x4b107200, 0x4c107200, 0x4e0f7000, 0x510f6f00, 0x52106e00, 0x54106d00, 0x560f6b00, 0x58106b00, 0x59106900, 0x5b0f6800, 0x5c106700, 0x5e0f6600, 0x600f6400, 0x620f6400, 0x640f6200, 0x660f6100, 0x670f6000, 0x690f5e00, 0x6a0f5d00, 0x6d0f5d00, 0x6f0f5b00, 0x710f5a00, 0x720f5900, 0x740e5800, 0x760e5700, 0x770f5600, 0x790e5500, 0x7b0e5300, 0x7d0e5200, 0x7e0e5100, 0x810e5000, 0x820e4f00, 0x840e4d00, 0x870e4b00, 0x8a0e4900, 0x8d0e4700, 0x900d4500, 0x930e4300, 0x960d4200, 0x990e4000, 0x9c0d3e00, 0x9f0e3c00, 0xa10e3a00, 0xa40d3800, 0xa70d3700, 0xaa0d3500, 0xad0d3300, 0xb00d3100, 0xb30d2f00, 0xb60d2d00, 0xb90d2b00, 0xbc0d2a00, 0xbe0c2800, 0xc10c2600, 0xc40d2400, 0xc70c2200, 0xca0c2000, 0xcd0c1e00, 0xd00c1c00, 0xd30b1a00, 0xd50c1800, 0xd80c1600, 0xdb0b1500, 0xde0c1300, 0xe10c1100, 0xe40b0f00, 0xe70b0d00, 0xea0b0b00, 0xeb100a00, 0xec170a00, 0xed1d0900, 0xee240800, 0xef2a0800, 0xf1310700, 0xf1370700, 0xf23d0600, 0xf4440600, 0xf44a0600, 0xf54e0500, 0xf5500500, 0xf6540500, 0xf7570400, 0xf75b0400, 0xf85f0400, 0xf9620300, 0xfa650300, 0xfa690200, 0xfa6c0300, 0xfb700200, 0xfb730200, 0xfc770200, 0xfc7a0200, 0xfe7d0100, 0xfd800100, 0xfe840000, 0xff870000, 0xfe8e0300, 0xfd940700, 0xfc9a0b00, 0xfba10d00, 0xfaa81100, 0xf9ae1500, 0xf8b51900, 0xf7bb1c00, 0xf6c11f00, 0xf5c62200, 0xf4cb2500, 0xf4d12700, 0xf3d52b00, 0xf2db2e00, 0xf1e03100, 0xf0e63300, 0xefeb3700, 0xf0ed4500, 0xf2ef5700, 0xf3f16900, 0xf4f37a00, 0xf6f48b00, 0xf7f59c00, 0xf8f7a700, 0xf9f8b000, 0xf9f9ba00, 0xfaf9c300, 0xfbface00, 0xfcfbd800, 0xfcfce100, 0xfdfdeb00, 0xfefff500, 0xffffff00, 0xffffff00, 
};
static gboolean gui_update_waterfall(GtkWidget *widget) {

	int i, j, p, hi;
	gdouble y;
	fftw_complex z;
	guchar data[FFT_SIZE*4];
	gint32 colour;
	fft_data_t *fft= sdr->fft;

	fftw_execute(fft->plan);
	fft->status=EMPTY;
	fft->index=0;

	hi = FFT_SIZE/2;
	j=0;
	for(i=0; i<FFT_SIZE; i++) {
		p=i;
		if (p<hi) p=p+hi; else p=p-hi;
		z = fft->out[p];	 // contains the FFT data 
		y=6*cabs(z);
		y = CLAMP(y , 0, 1.0);
		colour = colourmap[(int)(255*y)];
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
	sdr_smeter_set_level(SDR_SMETER(meter), y);
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
	sdr_data_t *sdr = (sdr_data_t *) psdr;
	gint state = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	switch (state) {
		case 0:
			sdr_waterfall_set_lowpass(wfdisplay, 3400.0f);
			sdr_waterfall_set_highpass(wfdisplay, 300.0f);
			break;
		case 1:
			sdr_waterfall_set_lowpass(wfdisplay, 1500.0f);
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
	sdr_waterfall_filter_cursors(SDR_WATERFALL(wfdisplay)); // hacky
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
	GtkWidget *waterfall;
	GtkWidget *vbox;
	GtkWidget *tuneslider;
	GtkWidget *hbox;
	GtkWidget *lpslider;
	GtkWidget *hpslider;
	GtkWidget *filter_combo;
	GtkWidget *mode_combo;
	GtkWidget *agc_combo;
	
	float tune_max;
	
	gtk_window_set_title(GTK_WINDOW(mainWindow), "lysdr");
	gtk_signal_connect(GTK_OBJECT(mainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	vbox = gtk_vbox_new(FALSE,1);
	gtk_container_add(GTK_CONTAINER(mainWindow), vbox);

	// tuning scale
	tune_max = (float)sdr->sample_rate;
	sdr->tuning = gtk_adjustment_new(-1, -tune_max/2, tune_max/2, 10, 100, 0);
	
	
	sdr->lp_tune = gtk_adjustment_new(3400, 300, 9000, 10, 100, 0); // pretty arbitrary limits
	sdr->hp_tune = gtk_adjustment_new(300, 25, 3400, 10, 100, 0); // pretty arbitrary limits

	// buttons etc
	hbox = gtk_hbox_new(FALSE,1);
	// S meter
	meter = sdr_smeter_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), meter, FALSE, TRUE, 0);

	agc_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(agc_combo), "Fast");
	gtk_combo_box_append_text(GTK_COMBO_BOX(agc_combo), "Slow");
	gtk_combo_box_append_text(GTK_COMBO_BOX(agc_combo), "Lock");
	gtk_combo_box_set_active(GTK_COMBO_BOX(agc_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox), agc_combo, TRUE, TRUE, 0);

	// VFO readout
	label = gtk_label_new (NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_label_set_markup(GTK_LABEL(label), "<tt>VFO</tt>");

	filter_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(filter_combo), "Wide");
	gtk_combo_box_append_text(GTK_COMBO_BOX(filter_combo), "Narrow");
	gtk_combo_box_set_active(GTK_COMBO_BOX(filter_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox), filter_combo, TRUE, TRUE, 0);

	mode_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(mode_combo), "LSB");
	gtk_combo_box_append_text(GTK_COMBO_BOX(mode_combo), "USB");
	gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
	gtk_box_pack_start(GTK_BOX(hbox), mode_combo, TRUE, TRUE, 0);

	wfdisplay = sdr_waterfall_new(GTK_ADJUSTMENT(sdr->tuning), GTK_ADJUSTMENT(sdr->lp_tune), GTK_ADJUSTMENT(sdr->hp_tune), sdr->sample_rate, FFT_SIZE);
	// common softrock frequencies
	// 160m =  1844250
	// 80m  =  3528000
	// 40m  =  7056000
	// 30m  = 10125000
	// 20m  = 14075000
	// 15m  = 21045000
	SDR_WATERFALL(wfdisplay)->centre_freq = sdr->centre_freq;
	gtk_widget_set_size_request(wfdisplay, FFT_SIZE, 250);
	gtk_box_pack_start(GTK_BOX(vbox), wfdisplay, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	
	gtk_widget_show_all(mainWindow);

	// connect handlers
	// FIXME - determine minimum update rate from jack latency
	g_timeout_add(25,  (GSourceFunc)gui_update_waterfall, (gpointer)wfdisplay);
	gtk_signal_connect(GTK_OBJECT(sdr->tuning), "value-changed", G_CALLBACK(tuning_changed), sdr);
	gtk_signal_connect(GTK_OBJECT(sdr->lp_tune), "value-changed", G_CALLBACK(filter_changed), sdr);
	gtk_signal_connect(GTK_OBJECT(sdr->hp_tune), "value-changed", G_CALLBACK(filter_changed), sdr);
	gtk_signal_connect(GTK_OBJECT(filter_combo), "changed", G_CALLBACK(filter_clicked), sdr);
	gtk_signal_connect(GTK_OBJECT(mode_combo), "changed", G_CALLBACK(mode_changed), sdr);
	gtk_signal_connect(GTK_OBJECT(agc_combo), "changed", G_CALLBACK(agc_changed), sdr);
}

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */

