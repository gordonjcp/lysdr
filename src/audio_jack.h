/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others
	
	audio_jack.h
	
	This file is part of lysdr.

	lysdr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	any later version.

	lysdr is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with lysdr.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __AUDIO_JACK_H
#define __AUDIO_JACK_H
#include "sdr.h"

extern int audio_connect(sdr_data_t *sdr, gboolean ci, gboolean co);
extern int audio_start(sdr_data_t *sdr);
extern int audio_stop(sdr_data_t *sdr);

#endif

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
