/* lysdr.c */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"

extern void gui_display();


SDR_DATA *sdr;
guchar data[2048]; //FIXME
int main(int argc, char *argv[]) {
    printf("lysdr starting\n");
    
    sdr = malloc(sizeof(SDR_DATA));
    audio_start(sdr);
    
    sdr->loPhase = cexp((I * -2.0 * 3.14159) / sdr->samplerate);
    make_filter(sdr->samplerate, 250, 3100, 1850);
    sdr->loVector = 1;
    sdr->agcGain = 0;
    sdr->agcPeak = 0;
        
    audio_connect(sdr);
    fft_setup(sdr);
    
    gtk_init(&argc, &argv);
    
    gui_display(sdr);
        
    gtk_main();
    fft_teardown(sdr);
    audio_stop(sdr);
    free(sdr);
}
