/* gui.c */

#include <math.h>
#include <complex.h> 
#include <gtk/gtk.h>

#include "sdr.h"


    GtkWidget *label;
    GtkWidget *progress;

static void tuning_changed(GtkWidget *widget, gpointer data) {
    g_print("%f\n", GTK_ADJUSTMENT(widget)->value);    

}
void gui_display(SDR_DATA *sdr)
{
	GtkWidget *mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *waterfall;
    GtkWidget *vbox;
    GtkWidget *tuneslider;
    
    float tune_max;
    
	gtk_window_set_title(GTK_WINDOW(mainWindow), "lysdr");
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_signal_connect(GTK_OBJECT(mainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE,1);
    gtk_container_add(GTK_CONTAINER(mainWindow), vbox);

    // tuning scale
    tune_max = (float)sdr->samplerate;
    sdr->tuning = gtk_adjustment_new(0, -tune_max/2, tune_max/2, 1, 0, 0);
    tuneslider = gtk_hscale_new(GTK_ADJUSTMENT(sdr->tuning));
    gtk_box_pack_start(GTK_BOX(vbox), tuneslider, TRUE, TRUE, 0);

    // VFO readout
    label = gtk_label_new (NULL);
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    gtk_label_set_markup(GTK_LABEL(label), "<tt>VFO</tt>");

    // AGC
    progress = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress, TRUE, TRUE, 0);
    gtk_widget_show_all(mainWindow);
    
    gtk_signal_connect(GTK_OBJECT(sdr->tuning), "value-changed", G_CALLBACK(tuning_changed), NULL);
    
}

