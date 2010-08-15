/* lysdr.c */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"
#include "filter.h"

extern void gui_display();


sdr_data_t *sdr;

int main(int argc, char *argv[]) {
    printf("lysdr starting\n");
    
    // get the Gtk threads support going
    if(!g_thread_supported())
        g_thread_init(NULL);

    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    // create a new SDR, and set up the jack client
    sdr = sdr_new();
    audio_start(sdr);

    // define a filter and configure a default shape
    sdr->filter = filter_fir_new(250, sdr->size);
    filter_fir_set_response(sdr->filter, sdr->sample_rate, 3100, 1850);
    
    // hook up the jack ports and start the client  
    fft_setup(sdr);
    audio_connect(sdr);

    gui_display(sdr);
    
    gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->tuning), 1015);
       
    gtk_main();
    audio_stop(sdr);
    filter_fir_destroy(sdr->filter);
    fft_teardown(sdr);
    
    sdr_destroy(sdr);
    gdk_threads_leave();
}
