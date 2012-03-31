/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others
	
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

#define IS_ALMOST_DENORMAL(f) (fabs(f) < 3.e-34)

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

/* scratchpad below here */
/*---------------------------------------------------------*/

#if 0

void fft_impulse() {
	// at this point, fir_imp[] contains our desired impulse.  Now to FFT it ;-)

#if 0
	/* this bit should be broken out to a function only called if the FFT size changes */
	fir_len = N;
	// Compute FFT and data block lengths.
	fft_len = MAX_FIR_LEN;
	blk_len = 128; // Min block length.
	n = 0;
	while ((int) pow(2.0,(double) n) < MAX_FIR_LEN) {
		if  ((int) pow(2.0, (double) n) >= fir_len + blk_len + 1) {
			fft_len = (int) pow(2, (double) n);
			break;
		}   
	 n++;
	}

	blk_len = fft_len + 1 - fir_len;

	// Check that blk_len is smaller than fragment size (=length/4).
	if (length/4 < blk_len) { 
		blk_len = length/4;
	}
	
	// okay, so we know how big to make the FFT blocks
	if (fwd) fftw_destroy_plan(fwd);
	if (bwd) fftw_destroy_plan(bwd);
	if (imp) fftw_destroy_plan(imp);
	fwd = fftw_plan_dft_1d(fft_len, fir_in, fir_fft, FFTW_FORWARD, FFTW_ESTIMATE);
	bwd = fftw_plan_dft_1d(fft_len, fir_fft, fir_in, FFTW_BACKWARD, FFTW_ESTIMATE); 
	imp = fftw_plan_dft_1d(fft_len, fir_imp, fir_imp_fft, FFTW_FORWARD, FFTW_ESTIMATE); 
	/* right up to here */
	
	// now we can FFT the impulse
	fftw_execute(imp);   
	
#endif
}

void fft_filter() {
	// FIXME - check if this is ever needed
	// Check if in-buffer is shorter than blk_len.
	if (length/4 < blk_len) { 
		blk_len = length/4;
	}
	
	// Process a new data block.
	for (blk_pos = 0; blk_pos<length; blk_pos += blk_len) {
  
	// Read in a new data block from input stream.
		for (i=0; i < fft_len; i++) {
			if (blk_pos+i < length  && i < blk_len ) {
				fir_in[i] = (double) sdr->iqSample[blk_pos + i]; // Get a new left channel sample.
			}
			else { 
			 fir_in[i] = 0.0;
		 } // if
		} // for

	// DFT, apply filter, IDFT
	fftw_execute(fwd);
	for (i=0; i<fft_len; i++) {
		fir_fft[i] *= fir_imp_fft[i]) / (double) fft_len;
	}
	fftw_execute(bwd);
	

	// Overlap-and-Add
	if ( (length-blk_pos) >= blk_len) // Have blk_len samples in buffer.
	  k = blk_len;
	else	// Have less than blk_len samples in buffer.
	  k = (length-blk_pos);
	
	// Add the last block's overlap to the current one.
	for (i = 0; i < fir_len-1; i++) {
	  fir_in[i] += fir_overlap[i];
	}

	// Save the samples that will overlap to the next block.
	for (i = k; i < k + fir_len; i++) {
		fir_overlap[i-k] = fir_in[i];
	}

  
	for (i=0; i<blk_len; i++) {
		if (blk_pos + i < length) sdr->iqSample[blk_pos+i] = fir_in[i];
	}
  
  } // for (blk_pos ...

  // Restore blk_len
  blk_len = fft_len + 1 - fir_len;



}

#endif

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
