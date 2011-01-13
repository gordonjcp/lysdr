/*  lysdr Software Defined Radio
    (C) 2010 Gordon JC Pearce MM0YEQ
    
    lysdr.c
    A simple software-defined radio for Linux
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include <gtk/gtk.h>
#include <stdlib.h>

#include "sdr.h"
#include "audio_jack.h"
#include "filter.h"

extern void gui_display();  // ugh, there should be a header file for the GUI
sdr_data_t *sdr;

static gboolean connect_input = FALSE;
static gboolean connect_output = FALSE;
static gint centre_freq = 0;
static gchar *tuning_hook = NULL;

static GOptionEntry opts[] = 
{
  { "ci", 0, 0, G_OPTION_ARG_NONE, &connect_input, "Autoconnect input to first two jack capture ports", NULL },
  { "co", 0, 0, G_OPTION_ARG_NONE, &connect_output, "Autoconnect output to first two jack playback ports", NULL },
  { "freq", 'f', 0, G_OPTION_ARG_INT, &centre_freq, "Set the centre frequency in Hz", "FREQUENCY" },
  { "tuning-hook", 0, 0, G_OPTION_ARG_STRING, &tuning_hook, "Program to run when tuned frequency changes", "PROGRAM" },
  { NULL }
};

static void hook_setup(gpointer data) {
    GString *s = g_string_new("");
    gint tuning = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(sdr->tuning));

    g_string_printf(s, "%d", sdr->centre_freq);
    g_setenv("LYSDR_CENTRE", s->str, TRUE);

    g_string_printf(s, "%d", tuning);
    g_setenv("LYSDR_OFFSET", s->str, TRUE);

    g_string_printf(s, "%d", sdr->centre_freq + tuning);
    g_setenv("LYSDR_FREQ", s->str, TRUE);

    switch (sdr->mode) {
        case SDR_LSB:
            g_string_printf(s, "LSB");
            break;
        case SDR_USB:
            g_string_printf(s, "USB");
            break;
    }
    g_setenv("LYSDR_MODE", s->str, TRUE);

    g_string_free(s, TRUE);
}

static gboolean run_hook(gpointer data) {
    gchar *hook = (gchar *)data;

    if (hook != NULL) {
        GPid pid;
        gchar *argv[] = { hook, NULL };

        g_spawn_async(NULL, argv, NULL, 0, hook_setup, NULL, &pid, NULL);
        g_spawn_close_pid(pid);
    }

    return FALSE;
}

static guint tuning_hook_timeout = 0;

static void tuning_changed(GtkAdjustment *adjustment, gpointer data) {
    if (tuning_hook_timeout) {
        // Cancel if already scheduled: we want to only run the hook once
        // the value's settled.
        g_source_remove(tuning_hook_timeout);
    }

    tuning_hook_timeout = g_timeout_add(100, run_hook, (gpointer)tuning_hook);
}

int main(int argc, char *argv[]) {
    GError *error = NULL;
    GOptionContext *context;


    printf("lysdr starting\n");
    
    // get the Gtk threads support going
    if(!g_thread_supported())
        g_thread_init(NULL);

    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);
    
    context = g_option_context_new ("-");
    g_option_context_add_main_entries (context, opts, NULL);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        exit (1);
    }

    // create a new SDR, and set up the jack client
    sdr = sdr_new();
    audio_start(sdr);

    // define a filter and configure a default shape
    sdr->filter = filter_fir_new(250, sdr->size);
    filter_fir_set_response(sdr->filter, sdr->sample_rate, 3100, 1850);
    
    // hook up the jack ports and start the client  
    fft_setup(sdr);
    audio_connect(sdr, connect_input, connect_output);
    
    sdr->centre_freq = centre_freq;

    gui_display(sdr);

    gtk_signal_connect(GTK_OBJECT(sdr->tuning), "value-changed", G_CALLBACK(tuning_changed), NULL);

    gtk_adjustment_set_value(GTK_ADJUSTMENT(sdr->tuning), 0);

    gtk_main();
    audio_stop(sdr);
    filter_fir_destroy(sdr->filter);
    fft_teardown(sdr);
    
    sdr_destroy(sdr);
    gdk_threads_leave();
}
