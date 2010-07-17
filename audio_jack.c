/*
 audio_jack.c
 handle setting up and tearing down jack connections
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <jack/jack.h>
#include "sdr.h"
#include "audio_jack.h"

static jack_port_t *I_in;
static jack_port_t *Q_in;
static jack_port_t *L_out;
static jack_port_t *R_out;
static jack_client_t *client;
static jack_status_t status;
static const char *client_name = "lysdr";

static int audio_process(jack_nframes_t nframes, void *psdr) {
    // actually kick off processing the samples
    jack_default_audio_sample_t *ii, *qq, *L, *R;
    int i, n;
    
    SDR_DATA *sdr;
    sdr = (SDR_DATA *) psdr;    // void* cast back to SDR_DATA*
    
    // get all four buffers
	ii = jack_port_get_buffer (I_in, nframes);
	qq = jack_port_get_buffer (Q_in, nframes);
	L = jack_port_get_buffer (L_out, nframes);
    R = jack_port_get_buffer (R_out, nframes);

    // the SDR expects a bunch of complex samples

    for(i = 0; i < nframes; i++) {
    // uncomment whichever is appropriate
        sdr->iqSample[i] = ii[i] + I * qq[i]; // I on left
    //    sdr->iqSample[i] = qq[i] + I * ii[i]; // I on right
    }

    // actually run the SDR for a frame

    sdr_process(sdr);

    // copy the frames to the output
    for(i = 0; i < nframes; i++) {
        L[i]=sdr->output[i];
        R[i]=sdr->output[i];
    }

    // we're happy, return okay
    return 0;
}

int audio_start(SDR_DATA *sdr) {
    // open a client connection to the JACK server
	client = jack_client_open (client_name, JackNullOption, &status, NULL);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}
	
	// save some info in the SDR
	sdr->size = jack_get_buffer_size(client);
	sdr->samplerate = jack_get_sample_rate(client);
	sdr->iqSample = (complex *)malloc(sizeof(complex)*sdr->size);
	sdr->output = (double *)malloc(sizeof(double)*sdr->size);
}

int audio_stop(SDR_DATA *sdr) {
    // remove the connection to the jack server
    // we may also want to clean up any audio buffers
    printf("audio_stop\n");
    jack_client_close (client);
    if (sdr->iqSample) free(sdr->iqSample);
    if (sdr->output) free(sdr->output);
}

int audio_connect(SDR_DATA *sdr) {
    
    const char **ports;
    // start processing audio
    jack_set_process_callback (client, audio_process, sdr);
    
    
    //jack_on_shutdown (client, jack_shutdown, 0);
    
    I_in = jack_port_register (client, "I input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);	 
	Q_in = jack_port_register (client, "Q input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
	L_out = jack_port_register (client, "L output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
	R_out = jack_port_register (client, "R output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}
	
	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}

	if (jack_connect (client, jack_port_name (L_out), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}
	if (jack_connect (client, jack_port_name (R_out), ports[1])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

    free(ports);
    
}

