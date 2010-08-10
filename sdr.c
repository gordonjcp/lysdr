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

int sdr_process(SDRData *sdr) {
    // actually do the SDR bit
    int i, j, k;
    double y, accI, accQ;
    complex c;
    FFT_DATA *fft = sdr->fft;
    float agcGain = sdr->agcGain;
    float agcPeak = 0;
    int size = sdr->size;
    
    
    int mvsmp = sdr->fft_size - size;

    // remove DC with a highpass filter
    for (i = 0; i < size; i++) {       // DC removal; R.G. Lyons page 553
                c = sdr->iqSample[i] + sdr->dc_remove * 0.95;
                sdr->iqSample[i] = c - sdr->dc_remove;
                sdr->dc_remove = c;
    }

    // copy this frame to FFT for display
#if 0
    if (fft->status != READY) {
        for (i=0; i<size; i++) {
            fft->samples[i+fft->index] = sdr->iqSample[i];
        }
        fft->index += size;
        if (fft->index > FFT_SIZE) {   
            fft->status = READY;
            fftw_execute(fft->plan);
        }
    }
#else
    //memmove(fft->samples, fft->samples+(size*sizeof(complex)), mvsmp*sizeof(complex));
    for (i=0; i<FFT_SIZE-size; i++) {
        fft->samples[i] = fft->samples[i+size];
    }
    for (i=0; i<size; i++) {
        fft->samples[i+(FFT_SIZE-size)] = sdr->iqSample[i];
    }
    fftw_execute(fft->plan);
    fft->status = READY;
#endif
    // shift frequency
    for (i = 0; i < sdr->size; i++) {
		sdr->iqSample[i] *= sdr->loVector;
    	sdr->loVector *= sdr->loPhase;
	}

    
    filter_fir_process(sdr);

    // this demodulates LSB
    for (i=0; i < sdr->size; i++) {
	    y = creal(sdr->iqSample[i])+cimag(sdr->iqSample[i]);
        sdr->output[i] = y*10; // FIXME level
    }

    // apply some AGC here
}

void fft_setup(SDRData *sdr) {
    sdr->fft = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    sdr->fft_size = FFT_SIZE;
    FFT_DATA *fft = sdr->fft;
    
    fft->samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sdr->fft_size);
    fft->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sdr->fft_size);
    fft->plan = fftw_plan_dft_1d(sdr->fft_size, fft->samples, fft->out, FFTW_FORWARD, FFTW_ESTIMATE);
	fft->status = EMPTY;
	fft->index = 0;
}

void fft_teardown(SDRData *sdr) {
    FFT_DATA *fft = sdr->fft;
    fftw_destroy_plan(fft->plan);
    fftw_free(fft->samples);
    fftw_free(fft->out);
    free(sdr->fft);
}

