/*  lysdr Software Defined Radio
    (C) 2010 Gordon JC Pearce MM0YEQ
    
    gui.c
    set up and draw the GUI elements
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
#include <math.h>
#include <complex.h> 
#include <gtk/gtk.h>

#include "sdr.h"
#include "waterfall.h"
#include "smeter.h"

extern sdr_data_t *sdr;

// these are global so that the gui_update routine can fiddle with them
GtkWidget *label;
GtkWidget *wfdisplay;
GtkWidget *meter;

// red hot
gint32 colourmap[] = {
0x00000000, 0x00000000, 0x00000000, 0x02000000, 0x03000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 0x07000000, 0x09000000, 0x09000000, 0x0b000000, 0x0b000000, 0x0c000000, 0x0d000000, 0x0e000000, 0x0f000000, 0x0f000000, 0x11000000, 0x11000000, 0x12000000, 0x13000000, 0x13000000, 0x15000000, 0x15000000, 0x16000000, 0x17000000, 0x18000000, 0x18000000, 0x1a000000, 0x1b000000, 0x1b000000, 0x1c000000, 0x1d000000, 0x1e000000, 0x1e000000, 0x1f000000, 0x20000000, 0x21000000, 0x22000000, 0x22000000, 0x24000000, 0x25000000, 0x25000000, 0x26000000, 0x27000000, 0x27000000, 0x29000000, 0x2a000000, 0x2a000000, 0x2b000000, 0x2c000000, 0x2d000000, 0x2e000000, 0x2e000000, 0x30000000, 0x31000000, 0x32000000, 0x32000000, 0x32000000, 0x34000000, 0x35000000, 0x36000000, 0x36000000, 0x38000000, 0x38000000, 0x39000000, 0x3a000000, 0x3b000000, 0x3b000000, 0x3c000000, 0x3d000000, 0x3e000000, 0x3f000000, 0x40000000, 0x41000000, 0x41000000, 0x42000000, 0x43000000, 0x44000000, 0x45000000, 0x46000000, 0x46000000, 0x47000000, 0x48000000, 0x49000000, 0x49000000, 0x4a000000, 0x4c000000, 0x4d000000, 0x4d000000, 0x4e000000, 0x4f000000, 0x50000000, 0x51000000, 0x52000000, 0x52000000, 0x53000000, 0x54000000, 0x55000000, 0x56000000, 0x57000000, 0x57000000, 0x58000000, 0x59000000, 0x5a000000, 0x5b000000, 0x5b000000, 0x5d000000, 0x5e000000, 0x60000000, 0x64000000, 0x66000000, 0x6a000000, 0x6d000000, 0x6f000000, 0x72000000, 0x75000000, 0x78000000, 0x7b000000, 0x7d000000, 0x80000000, 0x83000000, 0x86000000, 0x89000000, 0x8c000000, 0x8e000000, 0x92000000, 0x94000000, 0x98000000, 0x9a000000, 0x9d000000, 0xa0000000, 0xa3000000, 0xa6000000, 0xa8000000, 0xab000000, 0xae000000, 0xb1000000, 0xb4000000, 0xb7000000, 0xba000000, 0xbc030100, 0xbe070300, 0xbf0b0300, 0xc10e0500, 0xc2110600, 0xc4150800, 0xc6190800, 0xc71b0900, 0xc91f0b00, 0xcb230d00, 0xcd260e00, 0xcf290f00, 0xd12d1100, 0xd2311200, 0xd4341300, 0xd5371400, 0xd83c1500, 0xd93e1600, 0xdb421800, 0xdd461900, 0xde481a00, 0xdf4a1b00, 0xe04c1c00, 0xe14e1c00, 0xe2511d00, 0xe3531d00, 0xe4551f00, 0xe5571f00, 0xe7592000, 0xe75c2100, 0xe95e2100, 0xe9602200, 0xea632400, 0xec642400, 0xed662500, 0xee692500, 0xef6b2700, 0xf16d2800, 0xf16f2800, 0xf2722900, 0xf3732a00, 0xf5762b00, 0xf5792b00, 0xf77b2c00, 0xf87d2c00, 0xf97f2d00, 0xfa812e00, 0xfb843000, 0xfd853000, 0xfd883100, 0xfe8a3100, 0xff8e3200, 0xff923000, 0xfe972f00, 0xfe9c2d00, 0xfda12b00, 0xfda62a00, 0xfdaa2900, 0xfcaf2600, 0xfcb42600, 0xfcb82400, 0xfcbd2200, 0xfbc22100, 0xfbc62000, 0xfbca1e00, 0xfacf1d00, 0xfad41c00, 0xfad81a00, 0xfadd1900, 0xf9e11800, 0xf9e61500, 0xf8ea1400, 0xf8ef1300, 0xf8ef1b00, 0xf8f02400, 0xf8f02b00, 0xf8f03400, 0xf9f13d00, 0xf9f14400, 0xf8f14d00, 0xf9f25500, 0xf8f25e00, 0xf9f36600, 0xf8f26e00, 0xf8f37600, 0xf9f47f00, 0xf9f48600, 0xf9f48d00, 0xf9f49600, 0xf9f59e00, 0xf9f6a600, 0xf9f6ad00, 0xf8f5b500, 0xf9f6bd00, 0xf9f6c400, 0xf9f7cc00, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400, 0xf9f7d400
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
      	z = fft->out[p];     // contains the FFT data 
        y = 30 * log10(cabs(z) + 1e-10)-20;
        y = CLAMP((y / 100), -1.0, 0.0);
        
        colour = colourmap[255+(int)(255*y)];
        //colour = colourmap[j&0xff];
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
    //gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), y);
    sdr_smeter_set_level(SDR_SMETER(meter), y);
  return TRUE;
}


static void tuning_changed(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr;
    char l[20];
    sdr = (sdr_data_t *) psdr;
    float tune = gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));

    sdr->loPhase = cexp((I * -2.0 * 3.14159 * tune) / sdr->sample_rate);
    sprintf(l, "<span size=\"large\">%4.5f</span>",(sdr->centre_freq/1000000.0f)+(tune/1000000));
    gtk_label_set_markup(GTK_LABEL(label), l);
}

static void filter_clicked(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr = (sdr_data_t *) psdr;
    gint state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state == 0) {
        gtk_button_set_label(GTK_BUTTON(widget), "WIDE");
        filter_fir_set_response(sdr->filter, sdr->sample_rate, 3100, 1850);
        sdr_waterfall_set_lowpass(wfdisplay, 3400.0f);
        sdr_waterfall_set_highpass(wfdisplay, 300.0f);
    } else {
        gtk_button_set_label(GTK_BUTTON(widget), "NARROW");
        filter_fir_set_response(sdr->filter, sdr->sample_rate, 1500, 1650);
        sdr_waterfall_set_lowpass(wfdisplay, 2400.0f);
        sdr_waterfall_set_highpass(wfdisplay, 900.0f);
    }
}

static void filter_changed(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr = (sdr_data_t *) psdr;
    gdouble lowpass = gtk_adjustment_get_value(GTK_ADJUSTMENT(sdr->lp_tune));
    gdouble highpass = gtk_adjustment_get_value(GTK_ADJUSTMENT(sdr->hp_tune));
    filter_fir_set_response(sdr->filter, sdr->sample_rate, highpass-lowpass, lowpass+(highpass-lowpass)/2);
}

static void ssb_clicked(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr = (sdr_data_t *) psdr;
    
    gint state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state == 0) {
        gtk_button_set_label(GTK_BUTTON(widget), "LSB");
        sdr->mode = SDR_LSB;
        SDR_WATERFALL(wfdisplay)->mode = SDR_LSB;
    } else {
        gtk_button_set_label(GTK_BUTTON(widget), "USB");
        sdr->mode = SDR_USB;
        SDR_WATERFALL(wfdisplay)->mode = SDR_USB;
    }
    sdr_waterfall_filter_cursors(SDR_WATERFALL(wfdisplay)); // hacky
}

static void agc_clicked(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr = (sdr_data_t *) psdr;
    
    gint state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state == 0) {
        gtk_button_set_label(GTK_BUTTON(widget), "FAST");
        sdr->agc_speed = 0.005;
    } else {
        gtk_button_set_label(GTK_BUTTON(widget), "SLOW");
        sdr->agc_speed = 0.001;
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
    GtkWidget *filter_button;
    GtkWidget *ssb_button;
    GtkWidget *agc_button;
    
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

    agc_button = gtk_toggle_button_new_with_label("FAST");
    gtk_box_pack_start(GTK_BOX(hbox), agc_button, TRUE, TRUE, 0);

    // VFO readout
    label = gtk_label_new (NULL);
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
    gtk_label_set_markup(GTK_LABEL(label), "<tt>VFO</tt>");

    filter_button = gtk_toggle_button_new_with_label("WIDE");
    gtk_box_pack_start(GTK_BOX(hbox), filter_button, TRUE, TRUE, 0);
    ssb_button = gtk_toggle_button_new_with_label("LSB");
    gtk_box_pack_start(GTK_BOX(hbox), ssb_button, TRUE, TRUE, 0);


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
    gtk_signal_connect(GTK_OBJECT(filter_button), "clicked", G_CALLBACK(filter_clicked), sdr);
    gtk_signal_connect(GTK_OBJECT(ssb_button), "clicked", G_CALLBACK(ssb_clicked), sdr);
    gtk_signal_connect(GTK_OBJECT(agc_button), "clicked", G_CALLBACK(agc_clicked), sdr);
}

