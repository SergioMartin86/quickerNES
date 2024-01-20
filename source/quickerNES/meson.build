# quickerNES Core sources

quickerNESAPUSrc = [
 'apu/apu.cpp', 
 'apu/oscs.cpp', 
 'apu/buffer.cpp', 
 'apu/Blip_Buffer.cpp', 
 'apu/NESEffectsBuffer.cpp', 
 'apu/effectsBuffer.cpp', 
 'apu/Multi_Buffer.cpp',
 'apu/namco/apu.cpp', 
 'apu/vrc6/apu.cpp', 
 'apu/vrc7/emu2413.cpp', 
 'apu/vrc7/emu2413_state.cpp', 
 'apu/vrc7/apu.cpp',
 'apu/fme7/apu.cpp', 
]

quickerNESPPUSrc = [
 'ppu/ppu.cpp', 
 'ppu/ppuImpl.cpp', 
 'ppu/ppuRendering.cpp',
]

quickerNESSrc = quickerNESAPUSrc + quickerNESPPUSrc + [
 'mappers/mapper.cpp', 
 'emu.cpp', 
 'cpu.cpp'
]

# quickerNES Core Configuration

 quickerNESDependency = declare_dependency(
  compile_args        : [ '-D_USE_QUICKERNES', '-D_PAGE_SIZE="' + pageSize.to_string() + '"'],
  include_directories : include_directories(['.']),
  sources             : [ quickerNESSrc ]
 )