#ifndef __AUDIO_JACK_H
#define __AUDIO_JACK_H
#include "sdr.h"

extern int audio_connect(sdr_data_t *sdr);
extern int audio_start(sdr_data_t *sdr);
extern int audio_stop(sdr_data_t *sdr);

#endif
