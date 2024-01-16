project('quickerNES','c','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release']
)

# Getting page alignment option
pageSize = get_option('CPUFunctionAlignment')

# Loading dependencies
subdir('source')

# Common application flags
commonCompileArgs = [ '-Wfatal-errors', '-Wall', '-Wno-multichar' ]

# Building playback tool

quickerNESPlayerSrc = [
  'extern/hqn/hqn.cpp',
  'extern/hqn/hqn_gui_controller.cpp',
  'extern/hqn/hqn_surface.cpp',
  'extern/hqn/hqn_util.cpp',
  'extern/hqn/options.cpp',
]

executable('player',
  'source/player.cpp',
  cpp_args            : [ commonCompileArgs, '-DNCURSES' ],
  dependencies        : [ quickerNESDependency, toolDependency, dependency('sdl2'), dependency('SDL2_image') ],
  include_directories : include_directories(['source']),
  link_args           : [ '-lncurses' ],
  sources             : quickerNESPlayerSrc
)

# Building tester tool for QuickerNES

quickerNESTester = executable('qrTester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs, '-Werror' ],
  dependencies        : [ quickerNESDependency, toolDependency ],
  include_directories : include_directories(['../extern/json'])
)

# Building tester tool for the original QuickNES

quickNESTester = executable('qTester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs ],
  dependencies        : [ quickNESDependency, toolDependency ],
  include_directories : include_directories(['../extern/json'])
)

# Building tests
if get_option('buildTests') == true
  subdir('tests')
endif