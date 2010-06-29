/*
 sdr.h
 */
 
#ifndef __SDR_H
#define __SDR_H

#include <complex.h>

typedef struct {
    complex *iqSample;  // the array of incoming samples
    complex loVector;   // local oscillator vector
    complex loPhase;    // local oscillator phase angle (sets tuning)
    double *output;     // pointer to output samples
    
    // things to keep track of between callbacks
    complex dc_remove;
    float agcPeak;
    float agcGain;
    // jack parameters
    unsigned int size;  // periodsize
    unsigned int samplerate;    // samplerate
} SDR_DATA;

int sdr_process(SDR_DATA *sdr);

#endif
