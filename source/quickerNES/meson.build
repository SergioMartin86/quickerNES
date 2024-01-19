# quickerNES Core sources

subdir('audio')

quickerNESSrc = quickerNESAudioSrc + [
 'Nes_Ppu.cpp', 
 'Nes_Ppu_Impl.cpp', 
 'Nes_Ppu_Rendering.cpp', 
 'Nes_Emu.cpp', 
 'Nes_Mapper.cpp', 
 'Nes_Cpu.cpp'
]

# quickerNES Core Configuration

 quickerNESDependency = declare_dependency(
  compile_args        : [ '-D_USE_QUICKERNES', '-D_PAGE_SIZE="' + pageSize.to_string() + '"'],
  include_directories : include_directories(['.']),
  sources             : [ quickerNESSrc ]
 )