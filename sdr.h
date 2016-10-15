/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others

	sdr.h

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

#ifndef __SDR_H
#define __SDR_H

#include <complex.h>
#include <gtk/gtk.h>
#include <fftw3.h>
#include "filter.h"

#define FIR_SIZE 1024
#define MAX_FIR_LEN 8*4096

enum fft_status {EMPTY,			// fft_data is currently unused
	FILLING,			// now writing samples to this fft
	READY};	// ready to perform fft

enum rx_mode { SDR_LSB, SDR_USB };

typedef struct {
	fftw_complex *windowed;
	fftw_complex *samples;		// complex data for fft
	fftw_complex *out;
	fftw_complex *filter;
	fftw_plan plan;			// fft plan for fftw
	fftw_plan htplan;			// fft plan for fftw
	fftw_plan htbplan;			// fft plan for fftw
	int index;			// position of next fft sample
	enum fft_status status;		// whether the fft is busy
} fft_data_t;

typedef struct {
	double complex *iqSample;  // the array of incoming samples
	double complex loVector;   // local oscillator vector
	double complex loPhase;	// local oscillator phase angle (sets tuning)
	gdouble *output;	 // pointer to output samples

	GtkAdjustment *tuning;  // adjustment for tuning
	GtkAdjustment *lp_tune; // adjustment for filter lowpass
	GtkAdjustment *hp_tune; // adjustment for filter highpass
	gint mode;		  // demodulator mode
	gint centre_freq;

	fft_data_t *fft;
	gint fft_size;

	filter_fir_t *filter;

	// things to keep track of between callbacks
	double complex dc_remove;
	gfloat agc_gain;
	gfloat agc_speed;
	// jack parameters
	guint size;  // periodsize
	guint sample_rate;	// samplerate
} sdr_data_t;

sdr_data_t *sdr_new(gint fft_size);
int sdr_process(sdr_data_t *sdr);
void sdr_destroy(sdr_data_t *sdr);
void fft_setup(sdr_data_t *sdr);
void fft_teardown(sdr_data_t *sdr);
#endif

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
