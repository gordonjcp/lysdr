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
    
    audio_connect(sdr);
    gtk_init(&argc, &argv);
    gui_display();
    gtk_main();
    audio_stop(sdr);
    free(sdr);
}
