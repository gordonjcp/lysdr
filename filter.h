/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others
	
	filter.h
	
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

#include <math.h>
#include <complex.h>
#include "sdr.h"

#ifndef __FILTER_H
#define __FILTER_H

// IIR filter defs
typedef struct {
	// coefficients
	gfloat alpha, w0, b0, b1, b2, a0, a1, a2;
	// taps
	gfloat x1, x2, y1, y2;
	gint size;
} filter_iir_t;
    

// FIR filter defs
typedef struct {
	complex *impulse;
	double *buf_I;
	double *buf_Q;
	double *imp_I;
	double *imp_Q;
	int index;
	int size;
	int taps;
} filter_fir_t;

filter_fir_t *filter_fir_new(int taps, int size);
void filter_fir_destroy(filter_fir_t *filter);
void filter_fir_set_response(filter_fir_t *filter, int sample_rate, float bw, float centre);
void filter_fir_process(filter_fir_t *filter, complex *samples);
void filter_hilbert(gint phase, complex *samples, gint taps);
#endif

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
