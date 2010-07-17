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
static int sizeFilter = FIR_SIZE;

static int          fft_len = 0;
static int          blk_len = 0;
static gint blk_pos=0;
static int n;

static complex fir_in[MAX_FIR_LEN];
static complex fir_fft[MAX_FIR_LEN];
static complex fir_imp[MAX_FIR_LEN];
complex fir_imp_fft[MAX_FIR_LEN];
static complex fir_overlay[MAX_FIR_LEN];

static fftw_plan fwd  = NULL;
static fftw_plan bwd  = NULL;
static fftw_plan imp  = NULL;

int     fir_len = 0;
static int length = 1024; // number of samples in a period

void make_filter(float rate, int N, float bw, float centre) {
    // N - filter length
    // bw = bandwidth
    // centre = Fc

    float K = bw * N / rate;
    float w;
    complex z;
    int k, i=0;

    float tune = 2.0 * M_PI * centre / rate;
    
    for (k=-N/2; k<=N/2; k++) {
        if (k==0) z=(float)K/N;
        else z=1.0/N*sin(M_PI*k*K/N)/sin(M_PI*k/N);
        // apply a windowing function.  I can't hear any difference...
        w = 0.5 + 0.5 * cos(2.0 * M_PI * k / N); // Hanning window
        //w = 0.42 + 0.5 * cos(2.0 * M_PI * k / N) + 0.08 * cos(4. * M_PI * k / N); // Blackman window
        //w=1; // No window
        z *= w; 
        z *= 2*cexp(-1*I * tune * k);
        fir_imp[i] = z;
        i++;
    }
    // at this point, fir_imp[] contains our desired impulse.  Now to FFT it ;-)
    
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
    //blk_len = fft_len - fir_len;

    // Check that blk_len is smaller than fragment size (=length/4).
    if (length/4 < blk_len) { 
        blk_len = length/4;
    }

    //printf(" fft_len = %d\n blk_len = %d\n fir_len = %d\n", fft_len,blk_len,fir_len);
    
    // okay, so we know how big to make the FFT blocks
    if (fwd) fftw_destroy_plan(fwd);
    if (bwd) fftw_destroy_plan(bwd);
    if (imp) fftw_destroy_plan(imp);
    fwd = fftw_plan_dft_1d(fft_len, fir_in, fir_fft, FFTW_FORWARD, FFTW_ESTIMATE);
    bwd = fftw_plan_dft_1d(fft_len, fir_fft, fir_in, FFTW_BACKWARD, FFTW_ESTIMATE); 
    imp = fftw_plan_dft_1d(fft_len, fir_imp, fir_imp_fft, FFTW_FORWARD, FFTW_ESTIMATE); 
    
    // now we can FFT the impulse
    fftw_execute(imp);   
}

int sdr_process(SDR_DATA *sdr) {
    // actually do the SDR bit
    int i, j, k;
    double y, accI, accQ;
    complex c;
    FFT_DATA *fft = sdr->fft;
    FFT_DATA *fft_fwd = sdr->fir_fwd;
    FFT_DATA *fft_back = sdr->fir_fwd;
    FFT_DATA *fft_coeff = sdr->fir_coeff;
    float agcGain = sdr->agcGain;
    float agcPeak = 0;//sdr->agcPeak;

    // remove DC
    for (i = 0; i < sdr->size; i++) {       // DC removal; R.G. Lyons page 553
                c = sdr->iqSample[i] + sdr->dc_remove * 0.95;
                sdr->iqSample[i] = c - sdr->dc_remove;
                sdr->dc_remove = c;
    }

    // copy this frame to FFT for display
    for (i=0; i<sdr->size; i++) {
        fft->samples[i] = sdr->iqSample[i];
    }
    fft->status = READY;
    fftw_execute(fft->plan);
  
    // shift frequency
    for (i = 0; i < sdr->size; i++) {
		sdr->iqSample[i] *= sdr->loVector;
    	sdr->loVector *= sdr->loPhase;
	}

  
  /*
  1. Compute N point FFT of a block of N-M input samples (zero pad to get
to length N)
2. Complex multiply resulting N points with N point FFT of the M point
FIR filter (again zero pad)
3. IFFT the result of step 2 to get a block of N time samples
4. Add the last M points from the prior iteration's IFFT to the first M
points of this IFFT result
5. Write the first N-M points from step 4 to the output
6. Save the last M points of this IFFT result for the next iteration's
OLA
7. Go back to step 1 and process the next N-M samples from the input

The N point FFT of the M filter coefficients is typically a constant
and only needs to be computed at initialization.


  */



    // this demodulates LSB
    for (i=0; i<sdr->size; i++) {
	    y = creal(sdr->iqSample[i])+cimag(sdr->iqSample[i]);
        sdr->output[i] = y*10;
    }

}

void fft_setup(SDR_DATA *sdr) {
    sdr->fft = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    sdr->fft_size = FFT_SIZE;
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
    
    if (fwd) fftw_destroy_plan(fwd);
    if (bwd) fftw_destroy_plan(bwd);
    if (imp) fftw_destroy_plan(imp);

}

