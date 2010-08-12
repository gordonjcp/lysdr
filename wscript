#! /usr/bin/env python

# the following two variables are used by the target "waf dist"
VERSION='0.0.2'
APPNAME='lysdr'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def set_options(opt):
    opt.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check(header_name='stdlib.h')
    conf.check(header_name='math.h')
    
    conf.env.CCFLAGS = ['-O0', '-g3']
    
    conf.check_cfg(package='gtk+-2.0', uselib_store='GTK', atleast_version='2.6.0', mandatory=True, args='--cflags --libs')
    conf.check_cfg(package = 'jack', uselib_store='JACK', atleast_version = '0.118.0', args = '--cflags --libs')
    conf.check_cfg(package = 'fftw3', uselib_store='FFTW', atleast_version = '3.2.2', args = '--cflags --libs')
    
def build(bld):
    # 1. A simple program
    bld(
        features = 'cc cprogram',
        # source = bld.path.ant_glob('./*.c'),
        source = ('audio_jack.c filter.c gui.c lysdr.c sdr.c waterfall/waterfall.c'),
        #include = ['.', './waterfall',],
        target = 'gui',
        uselib = "GTK JACK FFTW",
        includes = '. /usr/include ./waterfall')

