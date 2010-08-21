/*  lysdr Software Defined Radio
    (C) 2010 Gordon JC Pearce MM0YEQ
    
    audio_jack.h
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef __AUDIO_JACK_H
#define __AUDIO_JACK_H
#include "sdr.h"

extern int audio_connect(sdr_data_t *sdr, gboolean ci, gboolean co);
extern int audio_start(sdr_data_t *sdr);
extern int audio_stop(sdr_data_t *sdr);

#endif
