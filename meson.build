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
commonCompileArgs = [ '-Wfatal-errors', '-Wall']

# Building playback tool

quickerNESPlayerSrc = [
  'extern/hqn/hqn.cpp',
  'extern/hqn/hqn_gui_controller.cpp',
  'extern/hqn/hqn_surface.cpp',
  'extern/hqn/hqn_util.cpp',
  'extern/hqn/options.cpp',
]

if get_option('buildPlayer') == true
  executable('player',
    'source/player.cpp',
    cpp_args            : [ commonCompileArgs, '-DNCURSES' ],
    dependencies        : [ quickerNESDependency, toolDependency, dependency('sdl2'), dependency('SDL2_image') ],
    include_directories : include_directories(['source']),
    link_args           : [ '-lncurses' ],
    sources             : quickerNESPlayerSrc
  )
endif

# Building tester tool for QuickerNES

quickerNESTester = executable('quickerNESTester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs, '-Werror' ],
  dependencies        : [ quickerNESDependency, toolDependency ],
  include_directories : include_directories(['../extern/json'])
)

# Building tester tool for the original QuickNES

quickNESTester = executable('quickNESTester',
  'source/tester.cpp',
  cpp_args            : [ commonCompileArgs, '-w', '-DDISABLE_AUTO_FILE', '-D__LIBRETRO__', '-DNDEBUG', '-DBLARGG_NONPORTABLE'  ],
  dependencies        : [ quickNESDependency, toolDependency ],
  include_directories : include_directories(['../extern/json'])
)

# Building tests
subdir('tests')