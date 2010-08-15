/*
 sdr.c
 
 */

#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

#include "filter.h"
#include "sdr.h"

static gint blk_pos=0;
static int n;

int sdr_process(sdr_data_t *sdr) {
    // actually do the SDR bit
    int i, j, k;
    double y, accI, accQ;
    complex c;
    FFT_DATA *fft = sdr->fft;
    int size = sdr->size;
    
    float agcGain = sdr->agcGain;
    float agcPeak = 0;

    // remove DC with a highpass filter
    for (i = 0; i < size; i++) {       // DC removal; R.G. Lyons page 553
                c = sdr->iqSample[i] + sdr->dc_remove * 0.95;
                sdr->iqSample[i] = c - sdr->dc_remove;
                sdr->dc_remove = c;
    }

    // copy this period to FFT buffer
    
    for (i=0; i<FFT_SIZE-size; i++) {
        fft->samples[i] = fft->samples[i+size];
    }

    for (i=0; i<size; i++) {
        fft->samples[i+(FFT_SIZE-size)] = sdr->iqSample[i]; // segfault happened here
    }


    // shift frequency
    for (i = 0; i < sdr->size; i++) {
		sdr->iqSample[i] *= sdr->loVector;
    	sdr->loVector *= sdr->loPhase;
	}

    
    filter_fir_process(sdr->filter, sdr->iqSample);

    // apply some AGC here
    for (i = 0; i < sdr->size; i++) {
        y = cabs(sdr->iqSample[i]);
        if (agcPeak < y) agcPeak = y;
    }
    if (agcPeak == 0) agcPeak = 0.00001;    // don't be zero, in case we have digital silence
    y = agcPeak * agcGain;  // y is the peak level scaled by the current gain

    if (y <= 1) {       // Current level is below the soundcard max, increase gain
        agcGain += (1/ agcPeak - agcGain) * 0.005;
    } else {                   // decrease gain
        agcGain += (1 / agcPeak - agcGain);
    }
    y = agcGain * 0.5; // change volume
    for (i = 0; i < sdr->size; i++){
        sdr->iqSample[i] *= y;
    }
    
    sdr->agcGain = agcGain;

    // this demodulates LSB
    for (i=0; i < sdr->size; i++) {
	    y = creal(sdr->iqSample[i])+cimag(sdr->iqSample[i]);
        sdr->output[i] = y;
    }
}

void fft_setup(sdr_data_t *sdr) {
    sdr->fft = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    sdr->fft_size = FFT_SIZE;
    FFT_DATA *fft = sdr->fft;
    
    fft->samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sdr->fft_size);
    fft->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sdr->fft_size);
    fft->plan = fftw_plan_dft_1d(sdr->fft_size, fft->samples, fft->out, FFTW_FORWARD, FFTW_ESTIMATE);
	fft->status = EMPTY;
	fft->index = 0;
}

void fft_teardown(sdr_data_t *sdr) {
    FFT_DATA *fft = sdr->fft;
    fftw_destroy_plan(fft->plan);
    fftw_free(fft->samples);
    fftw_free(fft->out);
    free(sdr->fft);
}

