/* gui.c */

#include <math.h>
#include <complex.h> 
#include <gtk/gtk.h>

#include "sdr.h"
#include "waterfall.h"

extern sdr_data_t *sdr;

// these are global so that the gui_update routine can fiddle with them
GtkWidget *label;
GtkWidget *wfdisplay;
GtkWidget *progress;

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

    y = (2000-sdr->agcGain)/2000;
    if (y<0) y = 0;
    if (y>1) y = 1;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), y);

  return TRUE;
}


static void tuning_changed(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr;
    char l[20];
    sdr = (sdr_data_t *) psdr;
    float tune = GTK_ADJUSTMENT(widget)->value;

    sdr->loPhase = cexp((I * -2.0 * 3.14159 * tune) / sdr->sample_rate);
    sdr->loVector = 1;
    sprintf(l, "<tt>%f</tt>",7.056+(tune/1000000));
    gtk_label_set_markup(GTK_LABEL(label), l);
}

static void lowpass_changed(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr;
    sdr = (sdr_data_t *) psdr;    // void* cast back to sdr_data_t*
    float tune = GTK_ADJUSTMENT(widget)->value;
    float hp_tune = GTK_ADJUSTMENT(sdr->hp_tune)->value;
    float lp_tune = GTK_ADJUSTMENT(sdr->lp_tune)->value;

    if (tune < GTK_ADJUSTMENT(sdr->hp_tune)->value) {
        gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->hp_tune), tune);
    }
    gtk_adjustment_set_upper(GTK_ADJUSTMENT(sdr->hp_tune), tune);
    gtk_adjustment_changed(GTK_ADJUSTMENT(sdr->hp_tune));
    //make_filter(sdr->samplerate, 256, (lp_tune-hp_tune), (lp_tune-hp_tune)/2 + hp_tune);
    gtk_widget_queue_draw(GTK_WIDGET(wfdisplay));
}

static void highpass_changed(GtkWidget *widget, gpointer psdr) {
    sdr_data_t *sdr;
    sdr = (sdr_data_t *) psdr;    // void* cast back to sdr_data_t*
    float tune = GTK_ADJUSTMENT(widget)->value;
    float hp_tune = GTK_ADJUSTMENT(sdr->hp_tune)->value;
    float lp_tune = GTK_ADJUSTMENT(sdr->lp_tune)->value;
    
    if (tune > GTK_ADJUSTMENT(sdr->lp_tune)->value) {
        gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->lp_tune), tune);
    }
    gtk_adjustment_set_lower(GTK_ADJUSTMENT(sdr->lp_tune), tune);
    gtk_adjustment_changed(GTK_ADJUSTMENT(sdr->lp_tune));
    //make_filter(sdr->samplerate, 256, (lp_tune-hp_tune), (lp_tune-hp_tune)/2 + hp_tune);
    gtk_widget_queue_draw(GTK_WIDGET(wfdisplay));
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
    
    float tune_max;
    
	gtk_window_set_title(GTK_WINDOW(mainWindow), "lysdr");
	gtk_signal_connect(GTK_OBJECT(mainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE,1);
    gtk_container_add(GTK_CONTAINER(mainWindow), vbox);

    // tuning scale
    tune_max = (float)sdr->sample_rate;
    sdr->tuning = gtk_adjustment_new(0, -tune_max/2, tune_max/2, 10, 100, 0);
    
    // filter sliders
    hbox = gtk_hbox_new(FALSE,1);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    
    sdr->lp_tune = gtk_adjustment_new(3400, 300, 9000, 10, 100, 0); // pretty arbitrary limits
    sdr->hp_tune = gtk_adjustment_new(300, 25, 3400, 10, 100, 0); // pretty arbitrary limits
    lpslider = gtk_hscale_new(GTK_ADJUSTMENT(sdr->lp_tune));
    hpslider = gtk_hscale_new(GTK_ADJUSTMENT(sdr->hp_tune));
    
    //gtk_box_pack_start(GTK_BOX(hbox), hpslider, TRUE, TRUE, 0);
    //gtk_box_pack_start(GTK_BOX(hbox), lpslider, TRUE, TRUE, 0);

    // VFO readout
    label = gtk_label_new (NULL);
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    gtk_label_set_markup(GTK_LABEL(label), "<tt>VFO</tt>");

    wfdisplay = sdr_waterfall_new(GTK_ADJUSTMENT(sdr->tuning), GTK_ADJUSTMENT(sdr->lp_tune), GTK_ADJUSTMENT(sdr->hp_tune), sdr->sample_rate, FFT_SIZE);
    gtk_widget_set_size_request(wfdisplay, FFT_SIZE, 250);
    gtk_box_pack_start(GTK_BOX(vbox), wfdisplay, TRUE, TRUE, 0);

    // AGC
    progress = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress, TRUE, TRUE, 0);
    
    gtk_widget_show_all(mainWindow);
    
    // connect handlers
    // FIXME - determine minimum update rate from jack latency
    g_timeout_add(25,  (GSourceFunc)gui_update_waterfall, (gpointer)wfdisplay);
    gtk_signal_connect(GTK_OBJECT(sdr->tuning), "value-changed", G_CALLBACK(tuning_changed), sdr);
    gtk_signal_connect(GTK_OBJECT(sdr->lp_tune), "value-changed", G_CALLBACK(lowpass_changed), sdr);
    gtk_signal_connect(GTK_OBJECT(sdr->hp_tune), "value-changed", G_CALLBACK(highpass_changed), sdr);
}

