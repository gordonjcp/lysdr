/*
 sdr.c
 
 */

#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

#include "sdr.h"

#define MAX_FILTER_SIZE 10000
 
static int indexFilter;
static int sizeFilter = 250;

static double cFilterI[MAX_FILTER_SIZE];	// Digital filter coefficients for receive
static double cFilterQ[MAX_FILTER_SIZE];	// Digital filter coefficients
static double bufFilterI[MAX_FILTER_SIZE];	// Digital filter sample buffer
static double bufFilterQ[MAX_FILTER_SIZE];	// Digital filter sample buffer

void make_filter(float rate, int N, float bw, float centre) {
    // rate is currently 48000 by default
    // N - filter length
    // bw = bandwidth
    // centre = Fc

    float K = bw * N / rate;
    float w;
    complex z;
    int i=0, k;
    float tune = 2.0 * M_PI * centre / rate;
    
    for (k=-N/2; k<N/2; k++) {
        if (k==0) z=(float)K/N;
        else z=1.0/N*sin(M_PI*k*K/N)/sin(M_PI*k/N);
        // apply a windowing function.  I can't hear any difference...
        //w = 0.5 + 0.5 * cos(2.0 * M_PI * k / N); // Hanning window
        w = 0.42 + 0.5 * cos(2.0 * M_PI * k / N) + 0.08 * cos(4. * M_PI * k / N); // Blackman window
        //w=1; // No window
        z *= w; 
        z *= 2*cexp(-1*I * tune * k);
        cFilterI[i] = creal(z);
        cFilterQ[i] = cimag(z);
        i++;
    }   
}

int sdr_process(SDR_DATA *sdr) {
    // actually do the SDR bit
    int i, j, k;
    double y, accI, accQ;
    complex c;
    
    float agcGain = sdr->agcGain;
    float agcPeak = 0;//sdr->agcPeak;
    

    // remove DC
    for (i = 0; i < sdr->size; i++) {       // DC removal; R.G. Lyons page 553
                c = sdr->iqSample[i] + sdr->dc_remove * 0.95;
                sdr->iqSample[i] = c - sdr->dc_remove;
                sdr->dc_remove = c;
    }

    // copy this frame to FFT
    if (fft->status != READY) {
        j = fft->index;

        for (i = 0; i < sdr->size; i++) {
            fft->samples[j] = sdr->iqSample[i];
	    	if (++j >= sdr->fft_size) {	// check sample count
	    		fft->status = READY;				// ready to run fft
	    	}
		}
		fft->index=j;
	}


    // shift frequency
    for (i = 0; i < sdr->size; i++) {
		sdr->iqSample[i] *= sdr->loVector;
    	sdr->loVector *= sdr->loPhase;
	}
	

	// apply the FIR filter
    for (i = 0; i < sdr->size; i++) {
        c = sdr->iqSample[i];
		bufFilterI[indexFilter] = creal(c);
		bufFilterQ[indexFilter] = cimag(c);
		accI = accQ = 0;
		j = indexFilter;
		for (k = 0; k < sizeFilter; k++) {
			accI += bufFilterI[j] * cFilterI[k];
			accQ += bufFilterQ[j] * cFilterQ[k];
			if (++j >= sizeFilter)
				j = 0;
		}
		sdr->iqSample[i] = accI + I * accQ;
		if (++indexFilter >= sizeFilter) indexFilter = 0;
	}

    // this demodulates LSB
    for (i=0; i<sdr->size; i++) {
	    y = creal(sdr->iqSample[i])+cimag(sdr->iqSample[i]);
        sdr->output[i] = y;
    }

    // agc
    for (i = 0; i < sdr->size; i++) {
        y = fabs(sdr->output[i]);
        if (agcPeak < y) agcPeak = y;
    }
    if (agcPeak == 0) agcPeak = 0.00001;
    //printf("%f %f\n", agcPeak, agcGain);
    y = agcPeak * agcGain; // Current level if not changed

    if (y <= 1) {       // Current level is below the soundcard max, increase gain
        agcGain += (1/ agcPeak - agcGain) * 0.005;
        //printf("increase %f\n",agcGain);
    } else {                   // decrease gain
        agcGain += (1 / agcPeak - agcGain) * 1;
        //printf("decrease %f\n",agcGain);
    }
    y = agcGain * 0.5;             // change volume
    for (i = 0; i < sdr->size; i++){
        sdr->output[i] *= y;
    }
 
    sdr->agcPeak = agcPeak;
    sdr->agcGain = agcGain;
}

void fft_setup(SDR_DATA *sdr) {
    sdr->fft = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    sdr->fft_size = 512; // FIXME
    FFT_DATA *fft = sdr->fft;
    
    fft->samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sdr->fft_size);
    fft->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * sdr->fft_size);
    fft->plan = fftw_plan_dft_1d(sdr->fft_size, fft->samples, fft->out, FFTW_FORWARD, FFTW_ESTIMATE);
	fft->status = EMPTY;
	fft->index = 0;
}

void fft_teardown(SDR_DATA *sdr) {
    FFT_DATA *fft = sdr->fft;
    fftw_destroy_plan(fft->plan);
    fftw_free(fft->samples);
    fftw_free(fft->out);
    free(sdr->fft);
}
