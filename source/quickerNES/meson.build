# quickerNES Core sources

quickerNESAudioSrc = [
 'audio/apu.cpp', 
 'audio/Nes_Oscs.cpp', 
 'audio/Nes_Buffer.cpp', 
 'audio/Blip_Buffer.cpp', 
 'audio/Effects_Buffer.cpp', 
 'audio/Nes_Effects_Buffer.cpp', 
 'audio/Multi_Buffer.cpp',
 'audio/namco/apu.cpp', 
 'audio/vrc6/apu.cpp', 
 'audio/vrc7/emu2413.cpp', 
 'audio/vrc7/emu2413_state.cpp', 
 'audio/vrc7/apu.cpp',
 'audio/fme7/apu.cpp', 
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