#!/usr/bin/env python

import os, subprocess, sys
import Options, Utils
from os import unlink, symlink, chdir, popen, system
from os.path import exists, join

cwd = os.getcwd()
srcdir = '.'
blddir = 'build'

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.tool_options("compiler_cc")
  opt.tool_options('misc')
  opt.add_option('--debug',
                 action='store',
                 default=False,
                 help='Enable debug variant [Default: False]',
                 dest='debug')

def configure(conf):
  conf.check_tool('compiler_cxx')
  if not conf.env.CXX: conf.fatal('c++ compiler not found')
  conf.check_tool("compiler_cc")
  if not conf.env.CC: conf.fatal('c compiler not found')
  conf.check_tool('node_addon')

  o = Options.options

  conf.env['USE_DEBUG'] = o.debug

  conf.env.append_value('CXXFLAGS', ['-D_FILE_OFFSET_BITS=64',
                                     '-D_LARGEFILE_SOURCE',
																		 '-D' + os.uname()[0],
                                     '-Wall',
                                     '-fPIC',
                                     '-Werror',
                                     '-fno-strict-aliasing'])
  if o.debug:
    conf.env.append_value('CXXFLAGS', ["-g"])
  else:
    conf.env.append_value('CXXFLAGS', ['-O3'])

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'sysevent_bindings'
  obj.source = './src/subscription.cc ./src/sysevent_bindings.cc'
  obj.name = "node-sysevent"
  obj.lib = ['sysevent', 'nvpair']

def test(ctx):
  system('nodeunit ./tst')

def lint(ctx):
  dirname = cwd + '/src'
  for f in os.listdir(dirname):
    subprocess.check_call(['./devtools/cpplint.py',
                           '--filter=-build/include,-build/header_guard,-runtime/rtti,-runtime/sizeof,-readability/casting',
                           os.path.join(dirname, f)])

  dirname = cwd + '/lib'
  for f in os.listdir(dirname):
    print 'jshint: ' + f
    subprocess.call(['jshint', os.path.join(dirname, f)])

  dirname = cwd + '/tst'
  for f in os.listdir(dirname):
    print 'jshint: ' + f
    subprocess.call(['jshint', os.path.join(dirname, f)])


def clean(ctx):
  os.popen('rm -rf .lock-wscript build')
