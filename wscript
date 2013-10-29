#! /usr/bin/env python

# the following two variables are used by the target "waf dist"
VERSION='0.0.6'
APPNAME='lysdr'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
    opt.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check(header_name='stdlib.h')
    conf.check(header_name='math.h')
    
    # set for debugging
    conf.env.CCFLAGS = ['-O0', '-g3', '-ggdb']
    #conf.env.CCFLAGS +=  ['-DG_DISABLE_SINGLE_INCLUDES','-DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES', '-DGTK_DISABLE_SINGLE_INCLUDES']
    #conf.env.CCFLAGS +=  ["-DG_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"]
    #conf.env.CCFLAGS += ["-DGSEAL_ENABLE"]

    conf.check_cfg(package='gtk+-2.0', uselib_store='GTK', atleast_version='2.6.0', mandatory=True, args='--cflags --libs')
    conf.check_cfg(package = 'jack', uselib_store='JACK', atleast_version = '0.118.0', mandatory=True, args = '--cflags --libs')
    conf.check_cfg(package = 'fftw3', uselib_store='FFTW', atleast_version = '3.2.2', mandatory=True, args = '--cflags --libs')
    conf.check(lib=['m'], uselib_store='M')
    
def build(bld):
    # the main program
    bld(
        features = 'c cprogram',
        source = ['lysdr.c', 'sdr.c', 'filter.c', 'audio_jack.c', 'gui.c', 'smeter.c', 'waterfall.c'],
        target = 'lysdr',
        uselib = "GTK JACK FFTW M",
        includes = '. /usr/include ./waterfall')

