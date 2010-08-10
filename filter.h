/* filter.h */

#include <math.h>
#include <complex.h>
#include "sdr.h"

#ifndef __FILTER_H
#define __FILTER_H
void make_filter(float sample_rate, int taps, float bw, float centre);


void *filter_fir_new(int taps, int size);
void filter_fir_destroy();
void filter_fir_set_response(int sample_rate, float bw, float centre);
void filter_fir_process(SDRData *sdr);
#endif
