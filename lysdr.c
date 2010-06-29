/* lysdr.c */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"

extern void gui_display();


SDR_DATA *sdr;
int main(int argc, char *argv[]) {
    printf("lysdr starting\n");
    
    sdr = (SDR_DATA *)malloc(sizeof(SDR_DATA));
    audio_start(sdr);
    
    sdr->loPhase = cexp((I * -2.0 * 3.14159 * -16750) / sdr->samplerate);
    sdr->loVector = 1;
    sdr->agcGain = 0;
    sdr->agcPeak = 0;


        
    audio_connect(sdr);
    gtk_init(&argc, &argv);
    gui_display();
    gtk_main();
    audio_stop(sdr);
    free(sdr);
}
