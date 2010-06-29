#ifndef __AUDIO_JACK_H
#define __AUDIO_JACK_H
#include "sdr.h"

extern int audio_connect(SDR_DATA *sdr);
extern int audio_start(SDR_DATA *sdr);
extern int audio_stop(SDR_DATA *sdr);

#endif
