# quickerNES Core sources

quickerNESAPUSrc = [
 'core/apu/apu.cpp', 
 'core/apu/oscs.cpp', 
 'core/apu/buffer.cpp', 
 'core/apu/blipBuffer.cpp', 
 'core/apu/NESEffectsBuffer.cpp', 
 'core/apu/effectsBuffer.cpp', 
 'core/apu/multiBuffer.cpp',
 'core/apu/namco/apu.cpp', 
 'core/apu/vrc6/apu.cpp', 
 'core/apu/vrc7/emu2413.cpp', 
 'core/apu/vrc7/emu2413_state.cpp', 
 'core/apu/vrc7/apu.cpp',
 'core/apu/fme7/apu.cpp', 
]

quickerNESPPUSrc = [
 'core/ppu/ppu.cpp', 
 'core/ppu/ppuImpl.cpp', 
 'core/ppu/ppuRendering.cpp',
]

quickerNESSrc = quickerNESAPUSrc + quickerNESPPUSrc + [
 'core/mappers/mapper.cpp', 
 'core/emu.cpp', 
 'core/cpu.cpp'
]

# quickerNES Core Configuration

 quickerNESDependency = declare_dependency(
  compile_args        : [ '-D_USE_QUICKERNES', '-D_PAGE_SIZE="' + pageSize.to_string() + '"'],
  include_directories : include_directories(['.', 'core']),
  sources             : [ quickerNESSrc ]
 )