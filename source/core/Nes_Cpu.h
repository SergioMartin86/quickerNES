
// NES 6502 CPU emulator

// Nes_Emu 0.7.0. http://www.slack.net/~ant/nes-emu/

/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
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

#ifndef NES_CPU_H
#define NES_CPU_H

#include <stdint.h>
#include "blargg_common.h"

typedef long     nes_time_t; // clock cycle count
typedef unsigned nes_addr_t; // 16-bit address

class Nes_Cpu {
public:

	inline void set_code_page( int i, uint8_t const* p )
	{
	  code_map [i] = p - (unsigned) i * page_size;
	}

	// Clear registers, unmap memory, and map code pages to unmapped_page.
	void reset( void const* unmapped_page = 0 )
	{
	 r.status = 0;
     r.sp = 0;
     r.pc = 0;
     r.a = 0;
     r.x = 0;
     r.y = 0;
     
     error_count_ = 0;
     clock_count = 0;
     clock_limit = 0;
     irq_time_ = LONG_MAX / 2 + 1;
     end_time_ = LONG_MAX / 2 + 1;
     
     set_code_page( 0, low_mem );
     set_code_page( 1, low_mem );
     set_code_page( 2, low_mem );
     set_code_page( 3, low_mem );
     for ( int i = 4; i < page_count + 1; i++ )
     set_code_page( i, (uint8_t*) unmapped_page );
     
     isCorrectExecution = true;
	}
	
	// Map code memory (memory accessed via the program counter). Start and size
	// must be multiple of page_size.
	static const uint8_t page_bits = 11;
	static const uint16_t page_count = 0x10000 >> page_bits;
	static const uint16_t page_size = 1L << page_bits;

	void map_code( nes_addr_t start, unsigned size, void const* code )
	{
	 unsigned first_page = start / page_size;
	 for ( unsigned i = size / page_size; i--; )
	 set_code_page( first_page + i, (uint8_t*) code + i * page_size );
	}
	
	// Access memory as the emulated CPU does.
	int  read( nes_addr_t );
	void write( nes_addr_t, int data );
	uint8_t* get_code( nes_addr_t ); // non-const to allow debugger to modify code
	
	// Push a byte on the stack
	void push_byte( int );
	
	// NES 6502 registers. *Not* kept updated during a call to run().
	struct registers_t {
		long pc; // more than 16 bits to allow overflow detection
		uint8_t a;
		uint8_t x;
		uint8_t y;
		uint8_t status;
		uint8_t sp;
	};
	//registers_t r;
	
	// Reasons that run() returns
	enum result_t {
		result_cycles,  // Requested number of cycles (or more) were executed
		result_sei,     // I flag just set and IRQ time would generate IRQ now
		result_cli,     // I flag just cleared but IRQ should occur *after* next instr
		result_badop    // unimplemented/illegal instruction
	};
	
	result_t run( nes_time_t end_time );
	
	nes_time_t time() const             { return clock_count; }
	void reduce_limit( int offset );
	void set_end_time_( nes_time_t t );
	void set_irq_time_( nes_time_t t );
	unsigned long error_count() const   { return error_count_; }
	
	// If PC exceeds 0xFFFF and encounters page_wrap_opcode, it will be silently wrapped.
	static const uint8_t page_wrap_opcode = 0xF2;
	
	// One of the many opcodes that are undefined and stop CPU emulation.
	static const uint8_t bad_opcode = 0xD2;
	
	uint8_t const* code_map [page_count + 1];
	nes_time_t clock_limit;
	nes_time_t clock_count;
	nes_time_t irq_time_;
	nes_time_t end_time_;
	unsigned long error_count_;
	
	static const uint8_t irq_inhibit = 0x04;
	void update_clock_limit();
	
	registers_t r;
	bool isCorrectExecution = true;
	
	// low_mem is a full page size so it can be mapped with code_map
	uint8_t low_mem [page_size > 0x800 ? page_size : 0x800];
};

inline uint8_t* Nes_Cpu::get_code( nes_addr_t addr )
{
	return (uint8_t*) code_map [addr >> page_bits] + addr;
}
	
inline void Nes_Cpu::update_clock_limit()
{
	nes_time_t t = end_time_;
	if ( t > irq_time_ && !(r.status & irq_inhibit) )
		t = irq_time_;
	clock_limit = t;
}

inline void Nes_Cpu::set_end_time_( nes_time_t t )
{
	end_time_ = t;
	update_clock_limit();
}

inline void Nes_Cpu::set_irq_time_( nes_time_t t )
{
	irq_time_ = t;
	update_clock_limit();
}

inline void Nes_Cpu::reduce_limit( int offset )
{
	clock_limit -= offset;
	end_time_   -= offset;
	irq_time_   -= offset;
}

inline void Nes_Cpu::push_byte( int data )
{
	int sp = r.sp;
	r.sp = (sp - 1) & 0xFF;
	low_mem [0x100 + sp] = data;
}

#endif
