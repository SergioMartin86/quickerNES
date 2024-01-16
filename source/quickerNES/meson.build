# quickerNES Core sources

quickerNESSrc = [
 'Nes_Apu.cpp', 
 'Nes_File.cpp', 
 'Nes_Oscs.cpp', 
 'Nes_Buffer.cpp', 
 'Nes_Fme7_Apu.cpp', 
 'Nes_Ppu.cpp', 
 'Blip_Buffer.cpp', 
 'Nes_Ppu_Impl.cpp', 
 'Data_Reader.cpp', 
 'Nes_Ppu_Rendering.cpp', 
 'Effects_Buffer.cpp', 
 'Nes_State.cpp', 
 'emu2413.cpp', 
 'nes_util.cpp', 
 'emu2413_state.cpp', 
 'Nes_Effects_Buffer.cpp', 
 'Nes_Namco_Apu.cpp', 
 'Nes_Vrc6_Apu.cpp', 
 'Multi_Buffer.cpp', 
 'Nes_Emu.cpp', 
 'nes_ntsc.cpp', 
 'Nes_Vrc7.cpp',
 'Nes_Mapper.cpp', 
 'Nes_Core.cpp', 
 'Nes_Cpu.cpp', 
 'quickerNESInstance.hpp'
]

# quickerNES Core Configuration

 quickerNESDependency = declare_dependency(
  compile_args        : [ '-D_USE_QUICKERNES', '-D_PAGE_SIZE="' + pageSize.to_string() + '"'],
  include_directories : include_directories(['.']),
  sources             : [ quickerNESSrc ]
 )