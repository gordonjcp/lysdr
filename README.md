lysdr - simple software-defined radio
-------------------------------------

Requirements: gtk3, fftw3 and jack

Installing the prerequisites and compiling

    $ sudo apt-get install libgtk-3-dev libfftw3-dev libjack-jackd2-dev
    $ ./autogen.sh
    $ ./configure
    $ make

On startup, lysdr will connect its output to the first two jack physical
output ports if you pass "--co"
You will need to connect some sort of IQ source to the inputs - this can
be either a real live soundcard with some SDR hardware, or a prerecorded
IQ file.  To automatically connect the first two physical input ports on
startup, pass "--ci" on the command line.

Set the displayed centre frequency with  --freq <frequency>
For the Kanga Finningley with the stock local oscillator crystal, the
standard centre frequency is:
80m  =  3750000

For the Softrock v6.2 Lite boards, the standard centre frequencies are:
160m =  1844250
80m  =  3528000
40m  =  7056000
30m  = 10125000
20m  = 14075000
15m  = 21045000

These options may change in a future build.

Example:

    ## don't connect anything
    $ ./build/lysdr

    ## connect both input and output to lysdr
    $ ./build/lysdr --ci --co

Drag the slider to tune the radio.  The number below the slider is the
frequency offset in Hz from the SDR centre (local oscillator) frequency.
Right-drag for bandspread tuning (1Hz steps).

Drag the sides of the yellow filter bar to adjust the bandpass filter upper and
lower edges.

There are dropdowns to select locked, fast and slow AGC, wide and narrow filtering
and USB/LSB demodulation.

Please report bugs on the github page at https://github.com/gordonjcp/lysdr
