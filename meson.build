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

# Common includes
commonIncludes = include_directories(['extern'])

# Common sources
commonSources = [ 
   'extern/jaffarCommon/extern/metrohash128/metrohash128.cpp',
   'extern/jaffarCommon/extern/xdelta3/xdelta3.c',
   ]

# Building playback tool

quickerNESPlayerSrc = [
  'extern/hqn/hqn.cpp',
  'extern/hqn/hqn_gui_controller.cpp',
  'extern/hqn/hqn_surface.cpp',
]

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

  if get_option('buildPlayer') == true
    executable('player',
      'source/player.cpp',
      cpp_args            : [ commonCompileArgs, '-DNCURSES' ],
      dependencies        : [ quickerNESDependency, toolDependency, dependency('sdl2'), dependency('SDL2_image') ],
      link_args           : [ '-lncurses' ],
      sources             : [ commonSources, quickerNESPlayerSrc ],
      include_directories : [ commonIncludes ]
    )
  endif

  # Building tester tool for QuickerNES

  quickerNESTester = executable('quickerNESTester',
    'source/tester.cpp',
    cpp_args            : [ commonCompileArgs, '-Werror' ],
    dependencies        : [ quickerNESDependency, toolDependency ],
    sources             : [ commonSources ],
    include_directories : [ commonIncludes ]
  )

  # Building tester tool for the original QuickNES

  if get_option('buildQuickNES') == true
  quickNESTester = executable('quickNESTester',
    'source/tester.cpp',
    cpp_args            : [ commonCompileArgs, '-w', '-DDISABLE_AUTO_FILE', '-D__LIBRETRO__', '-DNDEBUG', '-DBLARGG_NONPORTABLE'  ],
    dependencies        : [ quickNESDependency, toolDependency ],
    sources             : [ commonSources ],
    include_directories : [ commonIncludes ]
  )
  endif

  # Building tests
  if get_option('buildQuickNES') == true
  subdir('tests')
  endif
  
endif # If not subproject