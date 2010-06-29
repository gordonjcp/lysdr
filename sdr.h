/*
 sdr.h
 */
 
#ifndef __SDR_H
#define __SDR_H

#include <complex.h>
#include <gtk/gtk.h>

typedef struct {
    complex *iqSample;  // the array of incoming samples
    complex loVector;   // local oscillator vector
    complex loPhase;    // local oscillator phase angle (sets tuning)
    double *output;     // pointer to output samples
    
    GtkObject *tuning;  // adjustment for tuning
    GtkObject *lp_tune; // adjustment for filter lowpass
    GtkObject *hp_tune; // adjustment for filter highpass
        
    // things to keep track of between callbacks
    complex dc_remove;
    float agcPeak;
    float agcGain;
    // jack parameters
    unsigned int size;  // periodsize
    unsigned int samplerate;    // samplerate
} SDR_DATA;

int sdr_process(SDR_DATA *sdr);
void make_filter(float rate, int N, float bw, float centre);
#endif
