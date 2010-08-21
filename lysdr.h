/*  lysdr Software Defined Radio
    (C) 2010 Gordon JC Pearce MM0YEQ
    
    lysdr.h
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

typedef enum {
    MODE_LSB,
    MODE_USB,
    MODE_CW,
    MODE_AM
} demod_t;

typedef struct {
    // SDR parameters
    gint centre_freq;   // local oscillator freq used for VFO and display
    demod_t mode;       // what mode we're currently listening to
    
}
