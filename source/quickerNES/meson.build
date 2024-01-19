# quickerNES Core sources

quickerNESAudioSrc = [
 'apu/apu.cpp', 
 'apu/Nes_Oscs.cpp', 
 'apu/Nes_Buffer.cpp', 
 'apu/Blip_Buffer.cpp', 
 'apu/Effects_Buffer.cpp', 
 'apu/Nes_Effects_Buffer.cpp', 
 'apu/Multi_Buffer.cpp',
 'apu/namco/apu.cpp', 
 'apu/vrc6/apu.cpp', 
 'apu/vrc7/emu2413.cpp', 
 'apu/vrc7/emu2413_state.cpp', 
 'apu/vrc7/apu.cpp',
 'apu/fme7/apu.cpp', 
]

quickerNESSrc = quickerNESAudioSrc + [
 'mappers/mapper.cpp', 
 'Nes_Ppu.cpp', 
 'Nes_Ppu_Impl.cpp', 
 'Nes_Ppu_Rendering.cpp', 
 'Nes_Emu.cpp', 
 'Nes_Cpu.cpp'
]

# quickerNES Core Configuration

 quickerNESDependency = declare_dependency(
  compile_args        : [ '-D_USE_QUICKERNES', '-D_PAGE_SIZE="' + pageSize.to_string() + '"'],
  include_directories : include_directories(['.']),
  sources             : [ quickerNESSrc ]
 )