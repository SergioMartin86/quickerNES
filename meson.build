project('quickerNES','c','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release']
)

# quickerNES Core sources

quickerNESCoreSrc = [
 'core/abstract_file.cpp', 
 'core/Nes_Apu.cpp', 
 'core/Nes_File.cpp', 
 'core/Nes_Oscs.cpp', 
 'core/apu_state.cpp', 
 'core/Nes_Buffer.cpp', 
 'core/Nes_Fme7_Apu.cpp', 
 'core/Nes_Ppu.cpp', 
 'core/Blip_Buffer.cpp', 
 'core/Nes_Cart.cpp', 
 'core/Nes_Mapper.cpp', 
 'core/Nes_Ppu_Impl.cpp', 
 'core/Data_Reader.cpp', 
 'core/Nes_Core.cpp', 
 'core/Nes_Ppu_Rendering.cpp', 
 'core/Effects_Buffer.cpp', 
 'core/Nes_Cpu.cpp', 
 'core/Nes_State.cpp', 
 'core/emu2413.cpp', 
 'core/nes_data.cpp', 
 'core/nes_util.cpp', 
 'core/emu2413_state.cpp', 
 'core/Nes_Effects_Buffer.cpp', 
 'core/Nes_Namco_Apu.cpp', 
 'core/Nes_Vrc6_Apu.cpp', 
 'core/Multi_Buffer.cpp', 
 'core/Nes_Emu.cpp', 
 'core/nes_ntsc.cpp', 
 'core/Nes_Vrc7.cpp', 
]

# quickerNES Core Configuration

quickerNESCoreCPPFlags = [ '-Wfatal-errors','-Wall' ]
quickerNESCoreIncludeDirs =  include_directories(['core', 'extern/metrohash128', 'extern/argparse', 'extern/phmap'])
quickerNESCoreDependencies = [ ]
quickerNESCoreCPPFlags = [ ]
quickerNESCoreCFlags = [ ]
quickerNESCoreLinkArgs = [ ]

# Building playback/validation tool

quickerNESPlayerSrc = [
  'player/hqn/hqn.cpp',
  'player/hqn/hqn_gui_controller.cpp',
  'player/hqn/hqn_surface.cpp',
  'player/hqn/hqn_util.cpp',
  'player/hqn/options.cpp'
]

quickerNESPlayerIncludeDirs =  include_directories([ 'player'])
quickerNESPlayerDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]
quickerNESPlayerCPPFlags = [ ]
quickerNESPlayerCFlags = [ ]
quickerNESPlayerLinkArgs = [ '-lncurses' ]

executable('player',
  'player/player.cpp',
  sources: [ quickerNESCoreSrc, quickerNESPlayerSrc ],
  include_directories: [ quickerNESCoreIncludeDirs, quickerNESPlayerIncludeDirs ],
  dependencies: [ quickerNESCoreDependencies, quickerNESPlayerDependencies ],
  cpp_args: [ quickerNESCoreCPPFlags, quickerNESPlayerCPPFlags ],
  c_args: [ quickerNESCoreCFlags, quickerNESPlayerCFlags ],
  link_args: [ quickerNESCoreLinkArgs, quickerNESPlayerLinkArgs ]
)
