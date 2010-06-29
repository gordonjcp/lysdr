/*
 gui.c
 */

#include <math.h>
#include <complex.h> 
#include <gtk/gtk.h>
/*#include "waterfall.h"
#include "sdr.h"
*/

//extern guchar data[FFT_SIZE*4];
//extern SDR_DATA *sdr;

    GtkWidget *label;
    GtkWidget *progress;
/*
static void tuning_changed (SDRWaterfall *wf, float tuning, gpointer data) {
    float f;
    char l[20];

    f = (sdr->samplerate / sdr->fft_size * tuning) - (sdr->samplerate / 2);
    sdr->loPhase = cexp((I * -2.0 * M_PI * f) / sdr->samplerate);
    sprintf(l, "<tt>%d</tt>",(int)(LO_FREQ+f));
    gtk_label_set_markup(GTK_LABEL(label), l);
}
*/

static void tuning_changed(GtkWidget *widget, gpointer data) {
    g_print("%f\n", GTK_ADJUSTMENT(widget)->value);    

}
void gui_display()
{
	GtkWidget *mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *waterfall;
    GtkWidget *vbox;
    GtkWidget *tuneslider;
    GtkAdjustment *tuning;

	gtk_window_set_title(GTK_WINDOW(mainWindow), "lysdr");
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_signal_connect(GTK_OBJECT(mainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE,1);
    gtk_container_add(GTK_CONTAINER(mainWindow), vbox);

    tuning = gtk_adjustment_new(0, -22050, 22050, 1, 0, 0);
    tuneslider = gtk_hscale_new(tuning);
    gtk_box_pack_start(GTK_BOX(vbox), tuneslider, TRUE, TRUE, 0);

    label = gtk_label_new (NULL);
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);
    gtk_label_set_markup(GTK_LABEL(label), "<tt>VFO</tt>");

    progress = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), progress, TRUE, TRUE, 0);
    gtk_widget_show_all(mainWindow);
    
    gtk_signal_connect(GTK_OBJECT(tuning), "value-changed", G_CALLBACK(tuning_changed), NULL);
    
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    gui_display();
    gtk_main();
}
