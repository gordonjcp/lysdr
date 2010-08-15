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
    
    if(!g_thread_supported())
        g_thread_init(NULL);

    gdk_threads_init();
    gdk_threads_enter();

    sdr = malloc(sizeof(sdr_data_t));
    audio_start(sdr);
    
    sdr->loPhase = cexp((I * -2.0 * 3.14159) / sdr->sample_rate);
    
    sdr->loVector = 1;
    sdr->agcGain = 0;
    sdr->filter = filter_fir_new(250, sdr->size);
    filter_fir_set_response(sdr->filter, sdr->sample_rate, 3100, 1850);
       
    audio_connect(sdr);

    fft_setup(sdr);
    
    gtk_init(&argc, &argv);
    
    gui_display(sdr);
    
    gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->tuning), 1015);
       
    gtk_main();
    audio_stop(sdr);
    filter_fir_destroy(sdr->filter);
    fft_teardown(sdr);
    
    free(sdr);
    gdk_threads_leave();
}
