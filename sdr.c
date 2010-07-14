/*
 sdr.c
 
 */

#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

#include "sdr.h"

#define MAX_FILTER_SIZE 10000
#define MAX_FIR_LEN 8*4096
 
static int indexFilter;
static int sizeFilter = FIR_SIZE;

static double cFilterI[MAX_FILTER_SIZE];	// Digital filter coefficients for receive
static double cFilterQ[MAX_FILTER_SIZE];	// Digital filter coefficients
static double bufFilterI[MAX_FILTER_SIZE];	// Digital filter sample buffer
static double bufFilterQ[MAX_FILTER_SIZE];	// Digital filter sample buffer

static double overlap[MAX_FIR_LEN];
static int          fft_len = 0;
static int          blk_len = 0;
static gint blk_pos=0;
static int n;

void make_filter(SDR_DATA *sdr, float rate, int N, float bw, float centre) {
    // rate is currently 48000 by default
    // N - filter length
    // bw = bandwidth
    // centre = Fc
    
    // blow away N
  
    int  fft_len = MAX_FIR_LEN;
    int fir_len = 512;  // number of FIR points
    int blk_len = 128; // Min block length.
    int n = 0;
    while ((int) pow(2.0,(double) n) < MAX_FIR_LEN) {
        if  ((int) pow(2.0, (double) n) >= fir_len + blk_len + 1) {
            fft_len = (int) pow(2, (double) n);
            break;
        }
    n++;
    }
    printf("blk_len = %d, fft_len = %d, fir_len=%d\n",blk_len, fft_len, fir_len);  



    N=fir_len;

    float K = bw * N / rate;
    float w;
    complex z;

    int i=0, k;
    float tune = 2.0 * M_PI * centre / rate;
    
    for (k=-N/2; k<=N/2; k++) {
        if (k==0) z=(float)K/N;
        else z=1.0/N*sin(M_PI*k*K/N)/sin(M_PI*k/N);
        // apply a windowing function.  I can't hear any difference...
        //w = 0.5 + 0.5 * cos(2.0 * M_PI * k / N); // Hanning window
        w = 0.42 + 0.5 * cos(2.0 * M_PI * k / N) + 0.08 * cos(4. * M_PI * k / N); // Blackman window
        //w=0.5; // No window
        z *= w; 
        z *= 2*cexp(-1*I * tune * k);
        cFilterI[i] = creal(z);
        cFilterQ[i] = cimag(z);
        sdr->fir_coeff->samples[i]= z;   // store the complex value, too
        i++;
    }
    // now for the clever bit
   fftw_execute(sdr->fir_coeff->plan);
   /*
   for (i=0; i<N; i++) {
       printf("%d %f\n",i,sdr->fir_coeff->out[i]);
   }
*/
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
  //blk_len = 128, fft_len = 1024, fir_len=512
  
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

    blk_len = 128, fft_len = 1024;
    int  fir_len=512;
    int length=sdr->size;
  
  
        blk_len = fft_len + 1 - fir_len;
      //blk_len = fft_len - fir_len;

      // Check that blk_len is smaller than fragment size (=length/4).
      if (length/4 < blk_len) { 
	blk_len = length/4;
      }
  
  printf("blk_len = %d\n",blk_len);
  
    // perform filtering
    for (blk_pos = 0; blk_pos<length; blk_pos += blk_len) {
        for (i=0, n=0; n < fft_len; i+=2, n++) {
            if (blk_pos+n < length  && n < blk_len ) {
             	sdr->fir_fwd->samples[n] = sdr->iqSample[blk_pos+n];
            } else { // Zero-pad
                sdr->fir_fwd->samples[n] = 0;
            } // if
        } // for


        // next DFT the samples
        fftw_execute(sdr->fir_fwd->plan);
    
       
        // perform the filtering
        for (n = 0; n < fft_len/2+1; n++) {
            sdr->fir_back->samples[n] *= sdr->fir_coeff->out[n] / (double) fft_len;
            //uf_r[n] *= hf_r[n] / (double) fft_len;
        }
        
        
        fftw_execute(sdr->fir_back->plan);
        // now the filtered processed samples are in sdr->fir_fwd->samples again
                
        
        if ( (length-blk_pos)/2 >= blk_len) // Have blk_len samples in buffer.
            k = blk_len;
        else		  // Have less than blk_len samples in buffer.
            k = (length-blk_pos)/2;
    
        // Add the last block's overlap to the current one.
        for (n = 0; n < fir_len-1; n++) {
          sdr->fir_fwd->samples[n] += overlap[n]; 
          //u_r[n] += olap_r[n];
        }

    // Save the samples that will overlap to the next block.
    for (n = k; n < k+fir_len; n++) {
      overlap[n - k] = sdr->iqSample[n]; 
      //olap_r[n - k] = u_r[n]; 
    }
    
    for (i = 0,n=0; n < blk_len; i+=2,n++) {

      if (blk_pos+i < length/2) {
	
	// Left channel.
	sdr->iqSample[blk_pos + n]   =  sdr->fir_fwd->samples[n];//CLAMP( (gint16) u_l[n], -32768, 32767);
	
	// Right channel.
//	data[blk_pos + i+1] =  CLAMP( (gint16) u_r[n], -32768, 32767);

      } // if
    } // for
        
        
        blk_len = fft_len + 1 - fir_len;
        
    
    }
    

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
	
	sdr->fir_fwd = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    sdr->fir_fwd->samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FIR_SIZE);
    sdr->fir_fwd->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FIR_SIZE);
    sdr->fir_fwd->plan = fftw_plan_dft_1d(FIR_SIZE, sdr->fir_fwd->samples, sdr->fir_fwd->out, FFTW_FORWARD, FFTW_ESTIMATE);

    printf("set up sdr->fir_fwd correctly\n");

	sdr->fir_back = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    //sdr->fir_back->samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FIR_SIZE);
    //sdr->fir_back->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FIR_SIZE);
    sdr->fir_back->samples = sdr->fir_fwd->out;
    sdr->fir_back->out = sdr->fir_fwd->samples;
    
    sdr->fir_back->plan = fftw_plan_dft_1d(FIR_SIZE, sdr->fir_back->samples, sdr->fir_back->out, FFTW_BACKWARD, FFTW_ESTIMATE);

	sdr->fir_coeff = (FFT_DATA *)malloc(sizeof(FFT_DATA));
    sdr->fir_coeff->samples = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FIR_SIZE);
    sdr->fir_coeff->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FIR_SIZE);
    sdr->fir_coeff->plan = fftw_plan_dft_1d(FIR_SIZE, sdr->fir_coeff->samples, sdr->fir_coeff->out, FFTW_FORWARD, FFTW_ESTIMATE);

}

void fft_teardown(SDR_DATA *sdr) {
    FFT_DATA *fft = sdr->fft;
    fftw_destroy_plan(fft->plan);
    fftw_free(fft->samples);
    fftw_free(fft->out);
    free(sdr->fft);
    
    fft = sdr->fir_fwd;
    fftw_destroy_plan(fft->plan);
    fftw_free(fft->samples);
    fftw_free(fft->out);
    free(sdr->fir_fwd);

    fft = sdr->fir_back;
    fftw_destroy_plan(fft->plan);
    //fftw_free(fft->samples);
   // fftw_free(fft->out);
    free(sdr->fir_back);

    fft = sdr->fir_coeff;
    fftw_destroy_plan(fft->plan);
    fftw_free(fft->samples);
    fftw_free(fft->out);
    free(sdr->fir_coeff);
    
    
}

