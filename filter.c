/*  vim: set noexpandtab ai ts=4 sw=4 tw=4:
	lysdr Software Defined Radio
	
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others
	Hilbert transform code from Steve Harris' swh-plugins
	
	filter.c
	contains all filter creation and processing code

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

#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include "filter.h"
#include "sdr.h"
#include "hilbert.h"

#define IS_ALMOST_DENORMAL(f) (fabs(f) < 3.e-34)

static complex delay[D_SIZE];
static gfloat alpha, w0, b0, b1, b2, a0, a1, a2;
static gfloat x1r, x2r, y1r, y2r, x1i, x2i, y1i, y2i;
    


static void make_impulse(complex fir_imp[], float sample_rate, int taps, float bw, float centre) {

	float K = bw * taps / sample_rate;
	float w;
	complex z;
	int k, i=0;

	float tune = 2.0 * M_PI * centre / sample_rate;
	
	for (k=-taps/2; k<taps/2; k++) {
		if (k==0) z=(float)K/taps;
		else z=1.0/taps*sin(M_PI*k*K/taps)/sin(M_PI*k/taps);
		// apply a windowing function.  I can't hear any difference...
		//w = 0.5 + 0.5 * cos(2.0 * M_PI * k / taps); // Hanning window
		w = 0.42 + 0.5 * cos(2.0f * M_PI * k / taps) + 0.08 * cos(4.0f * M_PI * k / taps); // Blackman window
		//w=1; // No window
		z *= w; 
		z *= 2*cexp(-1*I * tune * k);
	if (IS_ALMOST_DENORMAL(creal(z))) { z = I * cimag(z); }
	if (IS_ALMOST_DENORMAL(cimag(z))) { z = creal(z); }
		fir_imp[i] = z;
		i++;
	}
}

filter_fir_t *filter_fir_new(int taps, int size) {
	// create the structure for a new FIR filter
	
	w0 = 2 * M_PI * 3400.0/48000.0;
	alpha = sin(w0)/(2*1.707);
	b0 = (1-cos(w0))/2;
	b1 =  1-cos(w0);
	b2 = (1-cos(w0))/2;
	a0 = 1 + alpha;
	a1 = -2*cos(w0);
	a2 = 1 - alpha;
	
	filter_fir_t *filter = malloc(sizeof(filter_fir_t));
	filter->taps = taps;
	filter->size = size;
	filter->impulse = malloc(sizeof(complex)*taps);
	filter->imp_I = malloc(sizeof(double)*taps);
	filter->imp_Q = malloc(sizeof(double)*taps);
	filter->buf_I = malloc(sizeof(double)*taps);
	filter->buf_Q = malloc(sizeof(double)*taps);
	filter->index = 0;
	return filter;
}

void filter_fir_destroy(filter_fir_t *filter) {
	// destroy the FIR filter
	if (filter) {
		if (filter->impulse) free(filter->impulse);
		if (filter->imp_I) free(filter->imp_I);
		if (filter->imp_Q) free(filter->imp_Q);
		if (filter->buf_I) free(filter->buf_I);
		if (filter->buf_Q) free(filter->buf_Q);
	   free(filter);
	}
}

void filter_fir_set_response(filter_fir_t *filter, int sample_rate, float bw, float centre) {
	// plop an impulse into the appropriate array
	int i;
	make_impulse(filter->impulse, sample_rate, filter->taps, bw, centre);

	for (i=0; i<filter->taps; i++) {
		filter->imp_I[i] = creal(filter->impulse[i]);
		filter->imp_Q[i] = cimag(filter->impulse[i]);
	} 
}

void filter_fir_process(filter_fir_t *filter, complex *samples) {
	// Perform an FIR filter on the data "in place"
	// this routine is slow and has a horrible hack to avoid denormals
	int i, j, k;
	complex c;
	double accI, accQ;
	double *buf_I = filter->buf_I;
	double *buf_Q = filter->buf_Q;
	double *imp_I = filter->imp_I;
	double *imp_Q = filter->imp_Q;
	int index = filter->index;
	int taps = filter->taps;
		
	for (i = 0; i < filter->size; i++) {
		c = samples[i];
		buf_I[index] = creal(c);
		buf_Q[index] = cimag(c);
	// flush denormals
	if (IS_ALMOST_DENORMAL(buf_I[index])) { buf_I[index]=0; }
	if (IS_ALMOST_DENORMAL(buf_Q[index])) { buf_Q[index]=0; }



		accI = accQ = 0;
		j = index;
		for (k = 0; k < taps; k++) {
			accI += buf_I[j] * imp_I[k];
			accQ += buf_Q[j] * imp_Q[k];
			if (++j >= taps) j = 0;
		}
		samples[i] = accI + I * accQ;
		index++;
		if (index >= taps) index = 0;
	}
	filter->index = index;

}

void filter_hilbert(complex *samples, gint taps) {
	// Hilbert transform, shamelessly nicked from swh-plugins
	// taps needs to be a multiple of D_SIZE
	// returns I and Q, with Q rotated through 90 degrees
	// 100 samples delay
	gint i, j, dptr = 0;
	gfloat hilb;
	for (i = 0; i < taps; i++) {
	  delay[dptr] = samples[i];
	  hilb = 0.0f;
	  for (j = 0; j < NZEROS/2; j++) {
	    hilb += (-xcoeffs[j] * cimag(delay[(dptr - j*2) & (D_SIZE - 1)]));
	  }
	  samples[i] = creal(delay[(dptr-99)& (D_SIZE-1)]) + I * hilb;
	  dptr = (dptr + 1) & (D_SIZE - 1);
	}

}

void filter_iir_process(filter_fir_t *filter, complex *samples) {
	
	int i;
	gfloat yr, yi, x;

	for (i = 0; i < filter->size; i++) {
		x = creal(samples[i]);
		yr = (b0/a0)*x + (b1/a0)*x1r + (b2/a0)*x2r - (a1/a0)*y1r - (a2/a0)*y2r;
		y2r = y1r; y1r = yr;
		x2r = x1r; x1r = x;
		x = cimag(samples[i]);
		yi = (b0/a0)*x + (b1/a0)*x1i + (b2/a0)*x2i - (a1/a0)*y1i - (a2/a0)*y2i;
		y2i = y1i; y1i = yi;
		x2i = x1i; x1i = x;
		samples[i] = yr+I*yi;
	}
}

