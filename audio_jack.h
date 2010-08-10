#ifndef __AUDIO_JACK_H
#define __AUDIO_JACK_H
#include "sdr.h"

extern int audio_connect(SDRData *sdr);
extern int audio_start(SDRData *sdr);
extern int audio_stop(SDRData *sdr);

#endif
