project('quickerNES','c','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release']
)

# quickerNES Core sources

quickerNESCoreSrc = [
 'source/core/abstract_file.cpp', 
 'source/core/Nes_Apu.cpp', 
 'source/core/Nes_File.cpp', 
 'source/core/Nes_Oscs.cpp', 
 'source/core/apu_state.cpp', 
 'source/core/Nes_Buffer.cpp', 
 'source/core/Nes_Fme7_Apu.cpp', 
 'source/core/Nes_Ppu.cpp', 
 'source/core/Blip_Buffer.cpp', 
 'source/core/Nes_Cart.cpp', 
 'source/core/Nes_Mapper.cpp', 
 'source/core/Nes_Ppu_Impl.cpp', 
 'source/core/Data_Reader.cpp', 
 'source/core/Nes_Core.cpp', 
 'source/core/Nes_Ppu_Rendering.cpp', 
 'source/core/Effects_Buffer.cpp', 
 'source/core/Nes_Cpu.cpp', 
 'source/core/Nes_State.cpp', 
 'source/core/emu2413.cpp', 
 'source/core/nes_data.cpp', 
 'source/core/nes_util.cpp', 
 'source/core/emu2413_state.cpp', 
 'source/core/Nes_Effects_Buffer.cpp', 
 'source/core/Nes_Namco_Apu.cpp', 
 'source/core/Nes_Vrc6_Apu.cpp', 
 'source/core/Multi_Buffer.cpp', 
 'source/core/Nes_Emu.cpp', 
 'source/core/nes_ntsc.cpp', 
 'source/core/Nes_Vrc7.cpp',
]

# quickerNES Core Configuration

 quickerNESCoreDependency = declare_dependency(
  compile_args        : [ '-Wfatal-errors', '-Wall',  '-Wno-multichar'  ],
  include_directories : include_directories(['source', 'source/core', 'extern']),
  sources             : [ quickerNESCoreSrc, 'extern/metrohash128/metrohash128.cpp' ]
 )

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
  cpp_args            : [ '-DNCURSES' ],
  dependencies        : [ quickerNESCoreDependency, dependency('sdl2'), dependency('SDL2_image') ],
  include_directories : include_directories(['source']),
  link_args           : [ '-lncurses' ],
  sources             : quickerNESPlayerSrc
)

# Building tester tool

tester = executable('tester',
   'source/tester.cpp',
   dependencies: [ quickerNESCoreDependency ],
   include_directories: include_directories(['../extern/json'])
  )

# Building tests
if get_option('buildTests') == true
  subdir('tests')
endif