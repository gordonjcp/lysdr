/* lysdr.c */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"
#include "filter.h"

extern void gui_display();


SDRData *sdr;
guchar data[4*FFT_SIZE];
int main(int argc, char *argv[]) {
    printf("lysdr starting\n");
    
    if( ! g_thread_supported() )
        g_thread_init( NULL );

    gdk_threads_init();
    gdk_threads_enter();
   
    
    sdr = malloc(sizeof(SDRData));
    audio_start(sdr);
    
    sdr->loPhase = cexp((I * -2.0 * 3.14159) / sdr->sample_rate);
    
    sdr->loVector = 1;
    sdr->agcGain = 0;
    filter_fir_new(250, sdr->size);
    filter_fir_set_response(sdr->sample_rate, 3100, 1850);
       
    audio_connect(sdr);

    fft_setup(sdr);
    
    gtk_init(&argc, &argv);
    
    gui_display(sdr);
    
    gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->tuning), 1015);
       
    gtk_main();
    //filter_fir_destroy();
    fft_teardown(sdr);
    audio_stop(sdr);
    free(sdr);
        gdk_threads_leave();
}
