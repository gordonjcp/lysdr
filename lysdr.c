/* lysdr.c */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"
#include "filter.h"

extern void gui_display();


SDR_DATA *sdr;
guchar data[4*FFT_SIZE];
int main(int argc, char *argv[]) {
    printf("lysdr starting\n");
    
    sdr = malloc(sizeof(SDR_DATA));
    audio_start(sdr);
    
    sdr->loPhase = cexp((I * -2.0 * 3.14159) / sdr->samplerate);
    
    sdr->loVector = 1;
    sdr->agcGain = 0;
    sdr->filter = filter_fir_new(250, sdr->size);
    filter_fir_set_response(sdr->filter, sdr->samplerate, 3100, 1850);
       
    audio_connect(sdr);
    printf("about to do fft-setup\n");
    fft_setup(sdr);
    
    gtk_init(&argc, &argv);
    
    gui_display(sdr);
        
    gtk_main();
    filter_fir_destroy(sdr->filter);
    fft_teardown(sdr);
    audio_stop(sdr);
    free(sdr);
}
