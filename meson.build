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

quickerNESCoreCPPFlags = [ '-Wfatal-errors','-Wall' ]
quickerNESCoreIncludeDirs =  include_directories(['source/core', 'extern'])
quickerNESCoreDependencies = [ ]
quickerNESCoreCPPFlags = [ ]
quickerNESCoreCFlags = [ ]
quickerNESCoreLinkArgs = [ ]

# Building playback/validation tool

quickerNESPlayerSrc = [
  'extern/hqn/hqn.cpp',
  'extern/hqn/hqn_gui_controller.cpp',
  'extern/hqn/hqn_surface.cpp',
  'extern/hqn/hqn_util.cpp',
  'extern/hqn/options.cpp'
]

quickerNESPlayerIncludeDirs =  include_directories([ 'source'])
quickerNESPlayerDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]
quickerNESPlayerCPPFlags = [ ]
quickerNESPlayerCFlags = [ ]
quickerNESPlayerLinkArgs = [ '-lncurses' ]

executable('player',
  'source/player.cpp',
  sources: [ quickerNESCoreSrc, quickerNESPlayerSrc ],
  include_directories: [ quickerNESCoreIncludeDirs, quickerNESPlayerIncludeDirs ],
  dependencies: [ quickerNESCoreDependencies, quickerNESPlayerDependencies ],
  cpp_args: [ quickerNESCoreCPPFlags, quickerNESPlayerCPPFlags ],
  c_args: [ quickerNESCoreCFlags, quickerNESPlayerCFlags ],
  link_args: [ quickerNESCoreLinkArgs, quickerNESPlayerLinkArgs ]
)
