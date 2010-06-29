#! /usr/bin/env python

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='cc_test'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def set_options(opt):
    opt.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check_cfg(package='gtk+-2.0', uselib_store='GTK', atleast_version='2.6.0', mandatory=True, args='--cflags --libs')
    #conf.env.append_value('CCFLAGS') #, '-DHAVE_CONFIG_H')
    #conf.write_config_header('config.h')

def build(bld):
    # 1. A simple program
    bld(
        features = 'cc cprogram',
        source = bld.path.ant_glob('**/*.c'),
        target = 'gui',
        uselib = "GTK",
        includes = '. /usr/include')

