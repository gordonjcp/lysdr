/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others
	
	lysdr.h
	
	This file is part of lysdr.

	lysdr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	any later version.

	lysdr is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with lysdr.  If not, see <http://www.gnu.org/licenses/>.
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
	demod_t mode;	// what mode we're currently listening to
}

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
