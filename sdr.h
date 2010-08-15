/*
 sdr.h
 */
 
#ifndef __SDR_H
#define __SDR_H

#include <complex.h>
#include <gtk/gtk.h>
#include <fftw3.h>
#include "filter.h"

#define FFT_SIZE 1024
#define FIR_SIZE 1024
#define MAX_FIR_LEN 8*4096

enum fft_status {EMPTY,			// fft_data is currently unused
	FILLING,			// now writing samples to this fft
	READY};				// ready to perform fft

enum rx_mode { LSB, USB };

typedef struct {
	fftw_complex *samples;		// complex data for fft
	fftw_complex *out;
	fftw_plan plan;			// fft plan for fftW
	int index;			// position of next fft sample
	enum fft_status status;		// whether the fft is busy
} FFT_DATA;

typedef struct {
    complex *iqSample;  // the array of incoming samples
    complex loVector;   // local oscillator vector
    complex loPhase;    // local oscillator phase angle (sets tuning)
    double *output;     // pointer to output samples

    GtkObject *tuning;  // adjustment for tuning
    GtkObject *lp_tune; // adjustment for filter lowpass
    GtkObject *hp_tune; // adjustment for filter highpass
    gint mode;          // demodulator mode

    FFT_DATA *fft;
    int fft_size;
    
    filter_fir_t *filter;

    // things to keep track of between callbacks
    complex dc_remove;
    float agcGain;
    // jack parameters
    unsigned int size;  // periodsize
    unsigned int sample_rate;    // samplerate
} SDRData;

int sdr_process(SDRData *sdr);
#endif
