#pragma once

// Internal NES emulator

/* Copyright (C) 2004-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

// Nes_Emu 0.7.0


#include <cstdio>
#include <string>
#include "blargg_common.h"
#include "Nes_Apu.h"
#include "Nes_Cpu.h"
#include "Nes_Ppu.h"
#include "Nes_Mapper.h"
#include "Nes_State.h"

/*
  New mapping distribution by Sergio Martin (eien86)
  https://github.com/SergioMartin86/jaffarPlus
*/
#include "mappers/mapper000.hpp"
#include "mappers/mapper001.hpp"
#include "mappers/mapper002.hpp"
#include "mappers/mapper003.hpp"
#include "mappers/mapper004.hpp"
#include "mappers/mapper005.hpp"
#include "mappers/mapper007.hpp"
#include "mappers/mapper009.hpp"
#include "mappers/mapper010.hpp"
#include "mappers/mapper011.hpp"
#include "mappers/mapper015.hpp"
#include "mappers/mapper019.hpp"
#include "mappers/mapper021.hpp"
#include "mappers/mapper022.hpp"
#include "mappers/mapper023.hpp"
#include "mappers/mapper024.hpp"
#include "mappers/mapper025.hpp"
#include "mappers/mapper026.hpp"
#include "mappers/mapper030.hpp"
#include "mappers/mapper032.hpp"
#include "mappers/mapper033.hpp"
#include "mappers/mapper034.hpp"
#include "mappers/mapper060.hpp"
#include "mappers/mapper066.hpp"
#include "mappers/mapper069.hpp"
#include "mappers/mapper070.hpp"
#include "mappers/mapper071.hpp"
#include "mappers/mapper073.hpp"
#include "mappers/mapper075.hpp"
#include "mappers/mapper078.hpp"
#include "mappers/mapper079.hpp"
#include "mappers/mapper085.hpp"
#include "mappers/mapper086.hpp"
#include "mappers/mapper087.hpp"
#include "mappers/mapper088.hpp"
#include "mappers/mapper089.hpp"
#include "mappers/mapper093.hpp"
#include "mappers/mapper094.hpp"
#include "mappers/mapper097.hpp"
#include "mappers/mapper113.hpp"
#include "mappers/mapper140.hpp"
#include "mappers/mapper152.hpp"
#include "mappers/mapper154.hpp"
#include "mappers/mapper156.hpp"
#include "mappers/mapper180.hpp"
#include "mappers/mapper184.hpp"
#include "mappers/mapper190.hpp"
#include "mappers/mapper193.hpp"
#include "mappers/mapper206.hpp"
#include "mappers/mapper207.hpp"
#include "mappers/mapper232.hpp"
#include "mappers/mapper240.hpp"
#include "mappers/mapper241.hpp"
#include "mappers/mapper244.hpp"
#include "mappers/mapper246.hpp"

class Nes_Cart;

#undef NES_EMU_CPU_HOOK
#ifndef NES_EMU_CPU_HOOK
	#define NES_EMU_CPU_HOOK( cpu, end_time ) cpu::run( end_time )
#endif

bool const wait_states_enabled = true;
bool const single_instruction_mode = false; // for debugging irq/nmi timing issues
const int unmapped_fill = Nes_Cpu::page_wrap_opcode;
unsigned const low_ram_size = 0x800;
unsigned const low_ram_end  = 0x2000;
unsigned const sram_end     = 0x8000;
const int irq_inhibit_mask = 0x04;

class Nes_Core : private Nes_Cpu {
	typedef Nes_Cpu cpu;
public:
	
	Nes_Core() : ppu( this )
	{
		cart = NULL;
		impl = NULL;
		mapper = NULL;
		memset( &nes, 0, sizeof nes );
		memset( &joypad, 0, sizeof joypad );
	}

	
	~Nes_Core()
	{
		close();
		delete impl;
	}
	
	const char * init()
	{
		if ( !impl )
		{
			CHECK_ALLOC( impl = new impl_t );
			impl->apu.dmc_reader( read_dmc, this );
			impl->apu.irq_notifier( apu_irq_changed, this );
		}
		
		return 0;
	}
	
	const char * open( Nes_Cart const* new_cart )
	{
		close();
		RETURN_ERR( init() );

		// Getting cartdrige mapper code
		auto mapperCode = new_cart->mapper_code();

		// Now checking if the detected mapper code is supported
		if (mapperCode ==   0) mapper = new Mapper000();
		if (mapperCode ==   1) mapper = new Mapper001();
		if (mapperCode ==   2) mapper = new Mapper002();
		if (mapperCode ==   3) mapper = new Mapper003();
		if (mapperCode ==   4) mapper = new Mapper004();
		if (mapperCode ==   5) mapper = new Mapper005();
		if (mapperCode ==   7) mapper = new Mapper007();
		if (mapperCode ==   9) mapper = new Mapper009();
		if (mapperCode ==  10) mapper = new Mapper010();
		if (mapperCode ==  11) mapper = new Mapper011();
		if (mapperCode ==  15) mapper = new Mapper015();
		if (mapperCode ==  19) mapper = new Mapper019();
		if (mapperCode ==  21) mapper = new Mapper021();
		if (mapperCode ==  22) mapper = new Mapper022();
		if (mapperCode ==  23) mapper = new Mapper023();
		if (mapperCode ==  24) mapper = new Mapper024();
		if (mapperCode ==  25) mapper = new Mapper025();
		if (mapperCode ==  26) mapper = new Mapper026();
		if (mapperCode ==  30) mapper = new Mapper030();
		if (mapperCode ==  32) mapper = new Mapper032();
		if (mapperCode ==  33) mapper = new Mapper033();
		if (mapperCode ==  34) mapper = new Mapper034();
		if (mapperCode ==  60) mapper = new Mapper060();
		if (mapperCode ==  66) mapper = new Mapper066();
		if (mapperCode ==  69) mapper = new Mapper069();
		if (mapperCode ==  70) mapper = new Mapper070();
		if (mapperCode ==  71) mapper = new Mapper071();
		if (mapperCode ==  73) mapper = new Mapper073();
		if (mapperCode ==  75) mapper = new Mapper075();
		if (mapperCode ==  78) mapper = new Mapper078();
		if (mapperCode ==  79) mapper = new Mapper079();
		if (mapperCode ==  85) mapper = new Mapper085();
		if (mapperCode ==  86) mapper = new Mapper086();
		if (mapperCode ==  87) mapper = new Mapper087();
		if (mapperCode ==  88) mapper = new Mapper088();
		if (mapperCode ==  89) mapper = new Mapper089();
		if (mapperCode ==  93) mapper = new Mapper093();
		if (mapperCode ==  94) mapper = new Mapper094();
		if (mapperCode ==  97) mapper = new Mapper097();
		if (mapperCode == 113) mapper = new Mapper113();
		if (mapperCode == 140) mapper = new Mapper140();
		if (mapperCode == 152) mapper = new Mapper152();
		if (mapperCode == 154) mapper = new Mapper154();
		if (mapperCode == 156) mapper = new Mapper156();
		if (mapperCode == 180) mapper = new Mapper180();
		if (mapperCode == 184) mapper = new Mapper184();
		if (mapperCode == 190) mapper = new Mapper190();
		if (mapperCode == 193) mapper = new Mapper193();
		if (mapperCode == 206) mapper = new Mapper206();
		if (mapperCode == 207) mapper = new Mapper207();
		if (mapperCode == 232) mapper = new Mapper232();
		if (mapperCode == 240) mapper = new Mapper240();
		if (mapperCode == 241) mapper = new Mapper241();
		if (mapperCode == 244) mapper = new Mapper244();
		if (mapperCode == 246) mapper = new Mapper246();

		// If no mapper was found, return null (error) now 
		if (mapper == NULL)
		{
			fprintf(stderr, "Could not find mapper for code: %u\n", mapperCode);
			return NULL;
		} 

		// Assigning backwards pointers to cartdrige and emulator now
		mapper->cart_ = new_cart;
		mapper->emu_ = this;

		RETURN_ERR( ppu.open_chr( new_cart->chr(), new_cart->chr_size() ) );
		
		cart = new_cart;
		memset( impl->unmapped_page, unmapped_fill, sizeof impl->unmapped_page );
		reset( true, true );

		return 0;
	}

	size_t getLiteStateSize() const
	{
    size_t size = 0;

    size += sizeof(nes_state_t);
    size += sizeof(registers_t);
    size += sizeof(ppu_state_t);
    size += sizeof(Nes_Apu::apu_state_t);
    size += sizeof(joypad_state_t);
    size += mapper->state_size;
    size += low_ram_size;
		size += Nes_Ppu::spr_ram_size;
		size_t nametable_size = 0x800;
		if (ppu.nt_banks [3] >= &ppu.impl->nt_ram [0xC00] ) nametable_size = 0x1000;
		size += nametable_size;
    if ( ppu.chr_is_writable ) size += ppu.chr_size;
		if ( sram_present )	size += impl->sram_size;

		return size;
	}
	
	size_t getStateSize() const
	{
    size_t size = 0;

	  size += sizeof(char[4]); // NESS Block
		size += sizeof(uint32_t); // Block Size
  
	  size += sizeof(char[4]); // TIME Block
		size += sizeof(uint32_t); // Block Size
    size += sizeof(nes_state_t);

    size += sizeof(char[4]); // CPUR Block
		size += sizeof(uint32_t); // Block Size
    size += sizeof(registers_t);

    size += sizeof(char[4]); // PPUR Block
		size += sizeof(uint32_t); // Block Size
    size += sizeof(ppu_state_t);

    size += sizeof(char[4]); // APUR Block
		size += sizeof(uint32_t); // Block Size
    size += sizeof(Nes_Apu::apu_state_t);

    size += sizeof(char[4]); // CTRL Block
		size += sizeof(uint32_t); // Block Size
    size += sizeof(joypad_state_t);

		size += sizeof(char[4]); // MAPR Block
		size += sizeof(uint32_t); // Block Size
    size += mapper->state_size;

    size += sizeof(char[4]); // LRAM Block
		size += sizeof(uint32_t); // Block Size
    size += low_ram_size;

    size += sizeof(char[4]); // SPRT Block
		size += sizeof(uint32_t); // Block Size
		size += Nes_Ppu::spr_ram_size;

		size += sizeof(char[4]); // NTAB Block
		size += sizeof(uint32_t); // Block Size
		size_t nametable_size = 0x800;
		if (ppu.nt_banks [3] >= &ppu.impl->nt_ram [0xC00] ) nametable_size = 0x1000;
		size += nametable_size;

    if ( ppu.chr_is_writable )
		{
			size += sizeof(char[4]); // CHRR Block
			size += sizeof(uint32_t); // Block Size
			size += ppu.chr_size;
		}

		if ( sram_present )
		{
			size += sizeof(char[4]); // SRAM Block
			size += sizeof(uint32_t); // Block Size
			size += impl->sram_size;
		}

		size += sizeof(char[4]);  // gend Block
		size += sizeof(uint32_t); // Block Size

		return size;
	}

	size_t serializeState(uint8_t* buffer) const
	{
    size_t pos = 0;
    std::string headerCode;
		const uint32_t headerSize = sizeof(char) * 4;
    uint32_t blockSize = 0;
		void* dataSource;

    headerCode = "NESS"; // NESS Block
		blockSize = 0xFFFFFFFF; 
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
  
	  headerCode = "TIME"; // TIME Block
		nes_state_t state = nes;
		state.timestamp *= 5;
		blockSize = sizeof(nes_state_t); 
		dataSource = (void*) &state;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "CPUR"; // CPUR Block
		cpu_state_t s;
		memset( &s, 0, sizeof s );
		s.pc = r.pc;
		s.s = r.sp;
		s.a = r.a;
		s.x = r.x;
		s.y = r.y;
		s.p = r.status;
		blockSize = sizeof(cpu_state_t); 
		dataSource = (void*) &s;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "PPUR"; // PPUR Block
		blockSize = sizeof(ppu_state_t); 
		dataSource = (void*) &ppu;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "APUR"; // APUR Block
		Nes_Apu::apu_state_t apuState;
		impl->apu.save_state(&apuState);
		blockSize = sizeof(Nes_Apu::apu_state_t); 
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], &apuState, blockSize);   pos += blockSize;

	  headerCode = "CTRL"; // CTRL Block
		blockSize = sizeof(joypad_state_t); 
		dataSource = (void*) &joypad;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "MAPR"; // MAPR Block
		blockSize = mapper->state_size; 
		dataSource = (void*) mapper->state;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "LRAM"; // LRAM Block
		blockSize = low_ram_size; 
		dataSource = (void*) low_mem;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "SPRT"; // SPRT Block
		blockSize = Nes_Ppu::spr_ram_size; 
		dataSource = (void*) ppu.spr_ram;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

	  headerCode = "NTAB"; // NTAB Block
		size_t nametable_size = 0x800;
		if (ppu.nt_banks [3] >= &ppu.impl->nt_ram [0xC00] ) nametable_size = 0x1000;
		blockSize = nametable_size; 
		dataSource = (void*) ppu.impl->nt_ram;
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
		memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;

    if ( ppu.chr_is_writable )
		{
		  headerCode = "CHRR"; // CHRR Block
			blockSize = ppu.chr_size; 
			dataSource = (void*) ppu.impl->chr_ram;
			memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
			memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
			memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;
		}

		if ( sram_present )
		{
			headerCode = "SRAM"; // SRAM Block
			blockSize = impl->sram_size; 
			dataSource = (void*) impl->sram;
			memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
			memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;
			memcpy(&buffer[pos], dataSource, blockSize);   pos += blockSize;
		}

		headerCode = "gend"; // gend Block
		blockSize = 0; 
		memcpy(&buffer[pos], headerCode.data(), headerSize); pos += headerSize;
		memcpy(&buffer[pos], &blockSize,  headerSize); pos += headerSize;

		return pos; // Bytes written
	}

  size_t deserializeState(const uint8_t* buffer)
	{
		disable_rendering();
		error_count = 0;
    ppu.burst_phase = 0; // avoids shimmer when seeking to same time over and over

    size_t pos = 0;
		const uint32_t headerSize = sizeof(char) * 4;
    uint32_t blockSize = 0;

    // NESS Block
		pos += headerSize;
		pos += headerSize;
  
	  // TIME Block
		nes_state_t nesState;
		pos += headerSize;
		pos += headerSize;
		blockSize = sizeof(nes_state_t);
		memcpy(&nesState, &buffer[pos], blockSize);  pos += blockSize;
		nes = nesState;
	  nes.timestamp /= 5;

	  // CPUR Block
		cpu_state_t s;
		blockSize = sizeof(cpu_state_t); 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) &s, &buffer[pos], blockSize); pos += blockSize;
		r.pc = s.pc; 
		r.sp = s.s; 
		r.a = s.a; 
		r.x = s.x; 
		r.y = s.y; 
		r.status = s.p; 

	  // PPUR Block
		blockSize = sizeof(ppu_state_t); 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) &ppu, &buffer[pos], blockSize);   pos += blockSize;

	  // APUR Block
		Nes_Apu::apu_state_t apuState;
		blockSize = sizeof(Nes_Apu::apu_state_t); 
		pos += headerSize;
		pos += headerSize;
		memcpy(&apuState, &buffer[pos], blockSize); 
		pos += blockSize;
		impl->apu.load_state(apuState);
    impl->apu.end_frame( -(int) nes.timestamp / ppu_overclock );

	  // CTRL Block
		blockSize = sizeof(joypad_state_t); 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) &joypad, &buffer[pos], blockSize); pos += blockSize;

	  // MAPR Block
		mapper->default_reset_state();
		blockSize = mapper->state_size; 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) mapper->state, &buffer[pos], blockSize); pos += blockSize;
    mapper->apply_mapping();

	  // LRAM Block
		blockSize = low_ram_size; 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) low_mem, &buffer[pos], blockSize); pos += blockSize;

	  // SPRT Block
		blockSize = Nes_Ppu::spr_ram_size; 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) ppu.spr_ram, &buffer[pos], blockSize);  pos += blockSize;

	  // NTAB Block
		size_t nametable_size = 0x800;
		if (ppu.nt_banks [3] >= &ppu.impl->nt_ram [0xC00] ) nametable_size = 0x1000;
		blockSize = nametable_size; 
		pos += headerSize;
		pos += headerSize;
		memcpy((void*) ppu.impl->nt_ram, &buffer[pos], blockSize); pos += blockSize;

    if ( ppu.chr_is_writable )
		{
		  // CHRR Block
			blockSize = ppu.chr_size; 
			pos += headerSize;
			pos += headerSize;
			memcpy((void*) ppu.impl->chr_ram, &buffer[pos], blockSize);  pos += blockSize;
		}

		if ( sram_present )
		{
			// SRAM Block
			blockSize = impl->sram_size; 
			pos += headerSize;
			pos += headerSize;
			memcpy((void*) impl->sram, &buffer[pos], blockSize);  pos += blockSize;
			enable_sram(true);
		}
   
		// headerCode = "gend"; // gend Block
		pos += headerSize;
		pos += headerSize;

		return pos; // Bytes read
	}

	void reset( bool full_reset, bool erase_battery_ram )
	{
		if ( full_reset )
		{
			cpu::reset( impl->unmapped_page );
			cpu_time_offset = -1;
			clock_ = 0;
			
			// Low RAM
			memset( cpu::low_mem, 0xFF, low_ram_size );
			cpu::low_mem [8] = 0xf7;
			cpu::low_mem [9] = 0xef;
			cpu::low_mem [10] = 0xdf;
			cpu::low_mem [15] = 0xbf;
			
			// SRAM
			lrom_readable = 0;
			sram_present = true;
			enable_sram( false );
			if ( !cart->has_battery_ram() || erase_battery_ram )
				memset( impl->sram, 0xFF, impl->sram_size );
			
			joypad.joypad_latches [0] = 0;
			joypad.joypad_latches [1] = 0;
			
			nes.frame_count = 0;
		}
		
		// to do: emulate partial reset
		
		ppu.reset( full_reset );
		impl->apu.reset();
		
		mapper->reset();
		
		cpu::r.pc = read_vector( 0xFFFC );
		cpu::r.sp = 0xfd;
		cpu::r.a = 0;
		cpu::r.x = 0;
		cpu::r.y = 0;
		cpu::r.status = irq_inhibit_mask;
		nes.timestamp = 0;
		error_count = 0;
	}
	
	nes_time_t emulate_frame(int joypad1, int joypad2)
  {
	current_joypad [0] = (joypad1 |= ~0xFF);
	current_joypad [1] = (joypad2 |= ~0xFF);

	cpu_time_offset = ppu.begin_frame( nes.timestamp ) - 1;
	ppu_2002_time = 0;
	clock_ = cpu_time_offset;
	
	// TODO: clean this fucking mess up
	auto t0 = emulate_frame_();
	impl->apu.run_until_( t0 );
	clock_ = cpu_time_offset;
	auto t1 = cpu_time();
	impl->apu.run_until_( t1 );
	
	nes_time_t ppu_frame_length = ppu.frame_length();
	nes_time_t length = cpu_time();
	nes.timestamp = ppu.end_frame( length );
	mapper->end_frame( length );

	impl->apu.end_frame( ppu_frame_length );
	
	disable_rendering();
	nes.frame_count++;
	
	return ppu_frame_length;
 }

	void close()
	{
		cart = NULL;
		delete mapper;
		mapper = NULL;
		
		ppu.close_chr();
		
		disable_rendering();
	}
	
	void save_state( Nes_State* out ) const
	{
		save_state( reinterpret_cast<Nes_State_*>(out) );
	}

	void save_state( Nes_State_* out ) const
	{
		out->clear();
		
		out->nes = nes;
		out->nes_valid = true;
		
		*out->cpu = cpu::r;
		out->cpu_valid = true;
		
		*out->joypad = joypad;
		out->joypad_valid = true;
		
		impl->apu.save_state( out->apu );
		out->apu_valid = true;
		
		ppu.save_state( out );
		
		memcpy( out->ram, cpu::low_mem, out->ram_size );
		out->ram_valid = true;
		
		out->sram_size = 0;
		if ( sram_present )
		{
			out->sram_size = sizeof impl->sram;
			memcpy( out->sram, impl->sram, out->sram_size );
		}
		
		out->mapper->size = 0;
		mapper->save_state( *out->mapper );
		out->mapper_valid = true;
	}
		
	void load_state( Nes_State_ const& in )
	{
		// disable_rendering();
		// error_count = 0;
		
		// if ( in.nes_valid )
		// 	nes = in.nes;
		
		// // always use frame count
		// ppu.burst_phase = 0; // avoids shimmer when seeking to same time over and over
		// nes.frame_count = in.nes.frame_count;
		// if ( (frame_count_t) nes.frame_count == invalid_frame_count )
		// 	nes.frame_count = 0;
		
		// if ( in.cpu_valid )
		// 	cpu::r = *in.cpu;
		
		// if ( in.joypad_valid )
		// 	joypad = *in.joypad;
		
		// if ( in.apu_valid )
		// {
		// 	impl->apu.load_state( *in.apu );
		// 	// prevent apu from running extra at beginning of frame
		// 	impl->apu.end_frame( -(int) nes.timestamp / ppu_overclock );
		// }
		// else
		// {
		// 	impl->apu.reset();
		// }
		
		// ppu.load_state( in );
		
		// if ( in.ram_valid )
		// 	memcpy( cpu::low_mem, in.ram, in.ram_size );
		
		// sram_present = false;
		// if ( in.sram_size )
		// {
		// 	sram_present = true;
		// 	// memcpy( impl->sram, in.sram, min( (int) in.sram_size, (int) sizeof impl->sram ) );
		// 	enable_sram( true ); // mapper can override (read-only, unmapped, etc.)
		// }
		
		// if ( in.mapper_valid ) // restore last since it might reconfigure things
		// 	mapper->load_state( *in.mapper );
	}
	
	void irq_changed()
	{
		cpu_set_irq_time( earliest_irq( cpu_time() ) );
	}
	
	void event_changed()
	{
		cpu_set_end_time( earliest_event( cpu_time() ) );
	}
	 
public: private: friend class Nes_Emu; 
	
	struct impl_t
	{
		enum { sram_size = 0x2000 };
		uint8_t sram [sram_size];
		Nes_Apu apu;
		
		// extra byte allows CPU to always read operand of instruction, which
		// might go past end of data
		uint8_t unmapped_page [::Nes_Cpu::page_size + 1];
	};
	impl_t* impl; // keep large arrays separate
	unsigned long error_count;
	bool sram_present;

public:
	unsigned long current_joypad [2];
	Nes_Cart const* cart;
	Nes_Mapper* mapper;
	nes_state_t nes;
	Nes_Ppu ppu;

private:
	// noncopyable
	Nes_Core( const Nes_Core& );
	Nes_Core& operator = ( const Nes_Core& );
	
	// Timing
	nes_time_t ppu_2002_time;
	void disable_rendering() { clock_ = 0; }
	
	inline nes_time_t earliest_irq( nes_time_t present )
	{
		return min( impl->apu.earliest_irq( present ), mapper->next_irq( present ) );
	}
	
	inline nes_time_t ppu_frame_length( nes_time_t present )
	{
		nes_time_t t = ppu.frame_length();
		if ( t > present )
			return t;
		
		ppu.render_bg_until( clock() ); // to do: why this call to clock() rather than using present?
		return ppu.frame_length();
	}


	inline nes_time_t earliest_event( nes_time_t present )
	{
		// PPU frame
		nes_time_t t = ppu_frame_length( present );
		
		// DMC
		if ( wait_states_enabled )
			t = min( t, impl->apu.next_dmc_read_time() + 1 );
		
		// NMI
		t = min( t, ppu.nmi_time() );
		
		if ( single_instruction_mode )
			t = min( t, present + 1 );
		
		return t;
	}
	
	// APU and Joypad
	joypad_state_t joypad;
	
	int read_io( nes_addr_t addr )
	{
		if ( (addr & 0xFFFE) == 0x4016 )
		{
			// to do: to aid with recording, doesn't emulate transparent latch,
			// so a game that held strobe at 1 and read $4016 or $4017 would not get
			// the current A status as occurs on a NES
			unsigned long result = joypad.joypad_latches [addr & 1];
			if ( !(joypad.w4016 & 1) )
				joypad.joypad_latches [addr & 1] = (result >> 1) | 0x80000000;
			return result & 1;
		}
		
		if ( addr == Nes_Apu::status_addr )
			return impl->apu.read_status( clock() );
		
		return addr >> 8; // simulate open bus
	}

	void write_io( nes_addr_t addr, int data )
	{
		// sprite dma
		if ( addr == 0x4014 )
		{
			ppu.dma_sprites( clock(), cpu::get_code( data * 0x100 ) );
			cpu_adjust_time( 513 );
			return;
		}
		
		// joypad strobe
		if ( addr == 0x4016 )
		{
			// if strobe goes low, latch data
			if ( joypad.w4016 & 1 & ~data )
			{
				joypad.joypad_latches [0] = current_joypad [0];
				joypad.joypad_latches [1] = current_joypad [1];
			}
			joypad.w4016 = data;
			return;
		}
		
		// apu
		if ( unsigned (addr - impl->apu.start_addr) <= impl->apu.end_addr - impl->apu.start_addr )
		{
			impl->apu.write_register( clock(), addr, data );
			if ( wait_states_enabled )
			{
				if ( addr == 0x4010 || (addr == 0x4015 && (data & 0x10)) )
				{
					impl->apu.run_until( clock() + 1 );
					event_changed();
				}
			}
			return;
		}
	}

	static inline int read_dmc( void* data, nes_addr_t addr )
	{
		Nes_Core* emu = (Nes_Core*) data;
		int result = *emu->cpu::get_code( addr );
		if ( wait_states_enabled )
			emu->cpu_adjust_time( 4 );
		return result;
	}
	
	static inline void apu_irq_changed( void* emu )
	{
		((Nes_Core*) emu)->irq_changed();
	}

	
	// CPU
	unsigned sram_readable;
	unsigned sram_writable;
	unsigned lrom_readable;
	nes_time_t clock_;
	nes_time_t cpu_time_offset;
	
	nes_time_t emulate_frame_()
	{
		Nes_Cpu::result_t last_result = cpu::result_cycles;
		int extra_instructions = 0;
		while ( true )
		{
			// Add DMC wait-states to CPU time
			if ( wait_states_enabled )
			{
				impl->apu.run_until( cpu_time() );
				clock_ = cpu_time_offset;
			}
			
			nes_time_t present = cpu_time();
			if ( present >= ppu_frame_length( present ) )
			{
				if ( ppu.nmi_time() <= present )
				{
					// NMI will occur next, so delayed CLI and SEI don't need to be handled.
					// If NMI will occur normally ($2000.7 and $2002.7 set), let it occur
					// next frame, otherwise vector it now.
					
					if ( !(ppu.w2000 & 0x80 & ppu.r2002) )
					{
						/* vectored NMI at end of frame */
						vector_interrupt( 0xFFFA );
						present += 7;
					}
					return present;
				}
				
				if ( extra_instructions > 2 )
				{
					return present;
				}
				
				if ( last_result != cpu::result_cli && last_result != cpu::result_sei &&
						(ppu.nmi_time() >= 0x10000 || (ppu.w2000 & 0x80 & ppu.r2002)) )
					return present;
				
				/* Executing extra instructions for frame */
				extra_instructions++; // execute one more instruction
			}
			
			// NMI
			if ( present >= ppu.nmi_time() )
			{
				ppu.acknowledge_nmi();
				vector_interrupt( 0xFFFA );
				last_result = cpu::result_cycles; // most recent sei/cli won't be delayed now
			}
			
			// IRQ
			nes_time_t irq_time = earliest_irq( present );
			cpu_set_irq_time( irq_time );
			if ( present >= irq_time && (!(cpu::r.status & irq_inhibit_mask) ||
					last_result == cpu::result_sei) )
			{
				if ( last_result != cpu::result_cli )
				{
					/* IRQ vectored */
					mapper->run_until( present );
					vector_interrupt( 0xFFFE );
				}
				else
				{
					// CLI delays IRQ
					cpu_set_irq_time( present + 1 );
				}
			}
			
			// CPU
			nes_time_t end_time = earliest_event( present );
			if ( extra_instructions )
				end_time = present + 1;
			unsigned long cpu_error_count = cpu::error_count();
			last_result = NES_EMU_CPU_HOOK( cpu, end_time - cpu_time_offset - 1 );
			cpu_adjust_time( cpu::time() );
			clock_ = cpu_time_offset;
			error_count += cpu::error_count() - cpu_error_count;
		}
	}

	nes_addr_t read_vector( nes_addr_t addr )
	{
		uint8_t const* p = cpu::get_code( addr );
		return p [1] * 0x100 + p [0];
	}
	
	void vector_interrupt( nes_addr_t vector )
	{
		cpu::push_byte( cpu::r.pc >> 8 );
		cpu::push_byte( cpu::r.pc & 0xFF );
		cpu::push_byte( cpu::r.status | 0x20 ); // reserved bit is set
		
		cpu_adjust_time( 7 );
		cpu::r.status |= irq_inhibit_mask;
		cpu::r.pc = read_vector( vector );
	}

	static void log_unmapped( nes_addr_t addr, int data = -1 );
	void cpu_set_irq_time( nes_time_t t ) { cpu::set_irq_time_( t - 1 - cpu_time_offset ); }
	void cpu_set_end_time( nes_time_t t ) { cpu::set_end_time_( t - 1 - cpu_time_offset ); }
	nes_time_t cpu_time() const { return clock_ + 1; }
	
	inline void cpu_adjust_time( int n )
	{
		ppu_2002_time   -= n;
		cpu_time_offset += n;
		cpu::reduce_limit( n );
	}
	
public: private: friend class Nes_Ppu;
	void set_ppu_2002_time( nes_time_t t ) { ppu_2002_time = t - 1 - cpu_time_offset; }
	
public: private: friend class Nes_Mapper;
	
	void enable_prg_6000()
	{
		sram_writable = 0;
		sram_readable = 0;
		lrom_readable = 0x8000;
	}
	
	void enable_sram( bool b, bool read_only = false)
	{
		sram_writable = 0;
		if ( b )
		{
			if ( !sram_present )
			{
				sram_present = true;
				memset( impl->sram, 0xFF, impl->sram_size );
			}
			sram_readable = sram_end;
			if ( !read_only )
				sram_writable = sram_end;
			cpu::map_code( 0x6000, impl->sram_size, impl->sram );
		}
		else
		{
			sram_readable = 0;
			for ( int i = 0; i < impl->sram_size; i += cpu::page_size )
				cpu::map_code( 0x6000 + i, cpu::page_size, impl->unmapped_page );
		}
	}

	nes_time_t clock() const { return clock_; }
	
	void add_mapper_intercept( nes_addr_t addr, unsigned size, bool read, bool write )
{
	int end = (addr + size + (page_size - 1)) >> page_bits;
	for ( int page = addr >> page_bits; page < end; page++ )
	{
		data_reader_mapped [page] |= read;
		data_writer_mapped [page] |= write;
	}
}
	
public: private: friend class Nes_Cpu;
	int  cpu_read_ppu( nes_addr_t, nes_time_t );
	int  cpu_read( nes_addr_t, nes_time_t );
	void cpu_write( nes_addr_t, int data, nes_time_t );
	void cpu_write_2007( int data );
	
private:
	unsigned char data_reader_mapped [page_count + 1]; // extra entry for overflow
	unsigned char data_writer_mapped [page_count + 1];
};

int mem_differs( void const* p, int cmp, unsigned long s );

inline int Nes_Core::cpu_read( nes_addr_t addr, nes_time_t time )
{
	//LOG_FREQ( "cpu_read", 16, addr >> 12 );
	
	{
		int result = cpu::low_mem [addr & 0x7FF];
		if ( !(addr & 0xE000) )
			return result;
	}
	
	{
		int result = *cpu::get_code( addr );
		if ( addr > 0x7FFF )
			return result;
	}
	
	time += cpu_time_offset;
	if ( addr < 0x4000 )
		return ppu.read( addr, time );
	
	clock_ = time;
	if ( data_reader_mapped [addr >> page_bits] )
	{
		int result = mapper->read( time, addr );
		if ( result >= 0 )
			return result;
	}
	
	if ( addr < 0x6000 )
		return read_io( addr );
	
	if ( addr < sram_readable )
		return impl->sram [addr & (impl_t::sram_size - 1)];
	
	if ( addr < lrom_readable )
		return *cpu::get_code( addr );
	
	return addr >> 8; // simulate open bus
}

inline int Nes_Core::cpu_read_ppu( nes_addr_t addr, nes_time_t time )
{
	//LOG_FREQ( "cpu_read_ppu", 16, addr >> 12 );
	
	// Read of status register (0x2002) is heavily optimized since many games
	// poll it hundreds of times per frame.
	nes_time_t next = ppu_2002_time;
	int result = ppu.r2002;
	if ( addr == 0x2002 )
	{
		ppu.second_write = false;
		if ( time >= next )
			result = ppu.read_2002( time + cpu_time_offset );
	}
	else
	{
		result = cpu::low_mem [addr & 0x7FF];
		if ( addr >= 0x2000 )
			result = cpu_read( addr, time );
	}
	
	return result;
}

inline void Nes_Core::cpu_write_2007( int data )
{
	// ppu.write_2007() is inlined
	if ( ppu.write_2007( data ) & Nes_Ppu::vaddr_clock_mask )
		mapper->a12_clocked();
}

inline void Nes_Core::cpu_write( nes_addr_t addr, int data, nes_time_t time )
{
	//LOG_FREQ( "cpu_write", 16, addr >> 12 );
	
	if ( !(addr & 0xE000) )
	{
		cpu::low_mem [addr & 0x7FF] = data;
		return;
	}
	
	time += cpu_time_offset;
	if ( addr < 0x4000 )
	{
		if ( (addr & 7) == 7 )
			cpu_write_2007( data );
		else
			ppu.write( time, addr, data );
		return;
	}
	
	clock_ = time;
	if ( data_writer_mapped [addr >> page_bits] && mapper->write_intercepted( time, addr, data ) )
		return;
	
	if ( addr < 0x6000 )
	{
		write_io( addr, data );
		return;
	}
	
	if ( addr < sram_writable )
	{
		impl->sram [addr & (impl_t::sram_size - 1)] = data;
		return;
	}
	
	if ( addr > 0x7FFF )
	{
		mapper->write( clock_, addr, data );
		return;
	}
}

#define NES_CPU_READ_PPU( cpu, addr, time ) \
	STATIC_CAST(Nes_Core&,*cpu).cpu_read_ppu( addr, time )

#define NES_CPU_READ( cpu, addr, time ) \
	STATIC_CAST(Nes_Core&,*cpu).cpu_read( addr, time )

#define NES_CPU_WRITEX( cpu, addr, data, time ){\
	STATIC_CAST(Nes_Core&,*cpu).cpu_write( addr, data, time );\
}

#define NES_CPU_WRITE( cpu, addr, data, time ){\
	if ( addr < 0x800 ) cpu->low_mem [addr] = data;\
	else if ( addr == 0x2007 ) STATIC_CAST(Nes_Core&,*cpu).cpu_write_2007( data );\
	else STATIC_CAST(Nes_Core&,*cpu).cpu_write( addr, data, time );\
}

