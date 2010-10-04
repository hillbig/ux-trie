VERSION = '0.1.0'
APPNAME = 'ux'

srcdir = '.'
blddir = 'build'

def set_options(ctx):
  ctx.tool_options('compiler_cxx')
  ctx.tool_options('unittestt')

def configure(ctx):
  ctx.check_tool('compiler_cxx')
  ctx.check_tool('unittestt')	
  ctx.env.CXXFLAGS += ['-O2', '-Wall', '-g']

def build(bld):
  bld.recurse('src')

def dist_hook():
  import os
  os.remove('upload.sh')
  os.remove('googlecode_upload.py')
