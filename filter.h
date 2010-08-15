/* filter.h */

#include <math.h>
#include <complex.h>
#include "sdr.h"

#ifndef __FILTER_H
#define __FILTER_H

void make_filter(float sample_rate, int taps, float bw, float centre);

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
#endif
