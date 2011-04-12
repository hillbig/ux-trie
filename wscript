#!/usr/bin/python

VERSION = '0.1.4'
APPNAME = 'ux'

srcdir = '.'
blddir = 'build'

def set_options(ctx):
  ctx.tool_options('compiler_cxx')
  ctx.tool_options('unittestt')

def configure(ctx):
  ctx.check_tool('compiler_cxx')
  ctx.check_tool('misc')
  ctx.check_tool('unittestt')	
  ctx.env.CXXFLAGS += ['-O2', '-W', '-Wall', '-g']

def build(bld):
  bld(features = 'subst',
      source = 'ux.pc.in',
      target = 'ux.pc',
      dict = {
        'prefix': bld.env['PREFIX'],
        'exec_prefix': '${prefix}',
        'libdir': '${exec_prefix}/lib',
        'includedir': '${prefix}/include',
        'PACKAGE': APPNAME,
        'VERSION': VERSION,
        }
      )
  bld.install_files('${PREFIX}/lib/pkgconfig', 'ux.pc')

  bld.recurse('src')

def dist_hook():
  import os
  os.remove('upload.sh')
  os.remove('googlecode_upload.py')
