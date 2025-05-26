# quickNES Core sources

quickNESSrc = [

	'core/nes_emu/Nes_Emu.cpp',
	'core/nes_emu/Nes_Vrc6_Apu.cpp',
	'core/nes_emu/emu2413_state.cpp',
	'core/nes_emu/Nes_Fme7_Apu.cpp',
	'core/nes_emu/abstract_file.cpp',
	'core/nes_emu/Multi_Buffer.cpp',
	'core/nes_emu/nes_data.cpp',
	'core/nes_emu/Nes_Apu.cpp',
	'core/nes_emu/Nes_Oscs.cpp',
	'core/nes_emu/nes_ntsc.cpp',
	'core/nes_emu/Data_Reader.cpp',
	'core/nes_emu/Nes_Ppu_Impl.cpp',
	'core/nes_emu/Nes_File.cpp',
	'core/nes_emu/Nes_Ppu_Rendering.cpp',
	'core/nes_emu/Effects_Buffer.cpp',
	'core/nes_emu/Nes_Mapper.cpp',
	'core/nes_emu/Nes_State.cpp',
	'core/nes_emu/nes_util.cpp',
	'core/nes_emu/Nes_Namco_Apu.cpp',
	'core/nes_emu/Nes_Core.cpp',
	'core/nes_emu/Nes_Buffer.cpp',
	'core/nes_emu/Nes_Cpu.cpp',
	'core/nes_emu/emu2413.cpp',
	'core/nes_emu/Nes_Cart.cpp',
	'core/nes_emu/apu_state.cpp',
	'core/nes_emu/Nes_Vrc7.cpp',
	'core/nes_emu/Blip_Buffer.cpp',
	'core/nes_emu/Nes_Effects_Buffer.cpp',
	'core/nes_emu/Nes_Ppu.cpp',
]

# quickNES Core Configuration

 quickNESDependency = declare_dependency(
  compile_args        : [  ],
  include_directories : include_directories(['.', '..', 'core/nes_emu']),
  sources             : [ quickNESSrc ],
		dependencies        : [ 
						  																		dependency('sdl2'),
		                      ]
 )
