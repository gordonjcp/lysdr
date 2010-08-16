/* gui.c */

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

static gboolean gui_update_waterfall(GtkWidget *widget) {

    int i, j, p, hi;
    gdouble y;
    fftw_complex z;
    guchar data[FFT_SIZE*4];
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
        y = 20 * log10(cabs(z) + 1e-10)-20;
        y = CLAMP((y / 100), -1.0, 0.0);
        data[j++] = 255+255*y;
        data[j++] = 255+255*y;
        data[j++] = 255+255*y;
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
    float tune = GTK_ADJUSTMENT(widget)->value;

    sdr->loPhase = cexp((I * -2.0 * 3.14159 * tune) / sdr->sample_rate);
    sprintf(l, "<tt>%4.5f</tt>",7.056+(tune/1000000));
    gtk_label_set_markup(GTK_LABEL(label), l);
}

static void filter_clicked(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr = (sdr_data_t *) psdr;
    gint state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state == 0) {
        gtk_button_set_label(GTK_BUTTON(widget), "WIDE");
        filter_fir_set_response(sdr->filter, sdr->sample_rate, 3100, 1850);
        gtk_adjustment_set_value(sdr->lp_tune, 3400);
                gtk_adjustment_set_value(sdr->hp_tune, 300);
    } else {
        gtk_button_set_label(GTK_BUTTON(widget), "NARROW");
        filter_fir_set_response(sdr->filter, sdr->sample_rate, 1500, 1650);
        gtk_adjustment_set_value(sdr->lp_tune, 2400);
                gtk_adjustment_set_value(sdr->hp_tune, 900);
    }
}

static void ssb_clicked(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr = (sdr_data_t *) psdr;
    
    gint state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state == 0) {
        gtk_button_set_label(GTK_BUTTON(widget), "LSB");
        sdr->mode = SDR_LSB;
    } else {
        gtk_button_set_label(GTK_BUTTON(widget), "USB");
        sdr->mode = SDR_USB;
    }
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
    SDR_WATERFALL(wfdisplay)->centre_freq = 7056000;
    gtk_widget_set_size_request(wfdisplay, FFT_SIZE, 250);
    gtk_box_pack_start(GTK_BOX(vbox), wfdisplay, TRUE, TRUE, 0);


    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    
    gtk_widget_show_all(mainWindow);

    // connect handlers
    // FIXME - determine minimum update rate from jack latency
    g_timeout_add(25,  (GSourceFunc)gui_update_waterfall, (gpointer)wfdisplay);
    gtk_signal_connect(GTK_OBJECT(sdr->tuning), "value-changed", G_CALLBACK(tuning_changed), sdr);
    gtk_signal_connect(GTK_OBJECT(filter_button), "clicked", G_CALLBACK(filter_clicked), sdr);
    gtk_signal_connect(GTK_OBJECT(ssb_button), "clicked", G_CALLBACK(ssb_clicked), sdr);
    gtk_signal_connect(GTK_OBJECT(agc_button), "clicked", G_CALLBACK(agc_clicked), sdr);
}

