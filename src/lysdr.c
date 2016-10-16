/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others

	lysdr.c
	A simple software-defined radio for Linux

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

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"
#include "filter.h"

extern void gui_display(sdr_data_t *sdr);  // ugh, there should be a header file for the GUI
sdr_data_t *sdr;

static gboolean connect_input = FALSE;
static gboolean connect_output = FALSE;
static gint centre_freq = 0;
static gint fft_size = 1024;

static GOptionEntry opts[] =
{
	{ "ci", 0, 0, G_OPTION_ARG_NONE, &connect_input, "Autoconnect input to first two jack capture ports", NULL },
	{ "co", 0, 0, G_OPTION_ARG_NONE, &connect_output, "Autoconnect output to first two jack playback ports", NULL },
	{ "freq", 'f', 0, G_OPTION_ARG_INT, &centre_freq, "Set the centre frequency in Hz", "FREQUENCY" },
	{ "fft-size", 'F', 0, G_OPTION_ARG_INT, &fft_size, "Set the FFT size (default=1024)", "FFT_SIZE" },
	{ NULL }
};

int main(int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context;


	printf("lysdr starting\n");

	// gtk3 remove threads, see how badly things go wrong
	//gdk_threads_init();
	//gdk_threads_enter();

	gtk_init(&argc, &argv);

	context = g_option_context_new ("-");
	g_option_context_add_main_entries (context, opts, NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("option parsing failed: %s\n", error->message);
		exit (1);
	}

	// create a new SDR, and set up the jack client
	sdr = sdr_new(fft_size);
	audio_start(sdr);

	// define a filter and configure a default shape
	sdr->filter = filter_fir_new(128, sdr->size);

	// hook up the jack ports and start the client
	fft_setup(sdr);
	audio_connect(sdr, connect_input, connect_output);

	sdr->centre_freq = centre_freq;

	gui_display(sdr);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->tuning), 0);

	gtk_main();
	audio_stop(sdr);
	filter_fir_destroy(sdr->filter);
	fft_teardown(sdr);

	sdr_destroy(sdr);
	//gdk_threads_leave();
}

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
