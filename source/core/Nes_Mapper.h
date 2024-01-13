#pragma once

// NES mapper interface
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

#include "Nes_Cart.h"
#include "Nes_Cpu.h"
#include "nes_data.h"
#include "Nes_Core.h"
#include <cstdio>
#include <string.h>
#include "blargg_source.h"

class Blip_Buffer;
class blip_eq_t;
class Nes_Core;

class Nes_Mapper {
public:
	// Register function that creates mapper for given code.
	typedef Nes_Mapper* (*creator_func_t)();
	static void register_mapper( int code, creator_func_t );
	
	// Register optional mappers included with Nes_Emu
	void register_optional_mappers();
	
	virtual ~Nes_Mapper() = default;
	
	// Reset mapper to power-up state.
	virtual inline void reset()
	{
		default_reset_state();
		reset_state();
		apply_mapping();
	}
	
	// Save snapshot of mapper state. Default saves registered state.
  virtual inline void save_state( mapper_state_t& out )
	{
		out.write( state, state_size );
	}
	
	// Resets mapper, loads state, then applies it
	virtual inline void load_state( mapper_state_t const& in )
	{
		default_reset_state();
		read_state( in );
		apply_mapping();
	}

	void setCartridge(const Nes_Cart* cart) {  cart_ = cart; }
	void setCore(Nes_Core* core) {  emu_ = core; }

// I/O

	// Read from memory
	virtual inline int read( nes_time_t, nes_addr_t ) { return -1; } ;
	
	// Write to memory
	virtual void write( nes_time_t, nes_addr_t, int data ) = 0;
	
	// Write to memory below 0x8000 (returns false if mapper didn't handle write)
	virtual inline bool write_intercepted( nes_time_t, nes_addr_t, int data ) { return false; }
	
// Timing
	
	// Time returned when current mapper state won't ever cause an IRQ
	enum { no_irq = LONG_MAX / 2 };
	
	// Time next IRQ will occur at
	virtual inline nes_time_t next_irq( nes_time_t present ) { return no_irq; };
	
	// Run mapper until given time
	virtual inline void run_until( nes_time_t ) { };
	
	// End video frame of given length
	virtual inline void end_frame( nes_time_t length ) { };

// Sound
	
	// Number of sound channels
	virtual inline int channel_count() const { return 0; };
	
	// Set sound buffer for channel to output to, or NULL to silence channel.
	virtual inline void set_channel_buf( int index, Blip_Buffer* ) { };
	
	// Set treble equalization
	virtual inline void set_treble( blip_eq_t const& ) { };

// Misc
	
	// Called when bit 12 of PPU's VRAM address changes from 0 to 1 due to
	// $2006 and $2007 accesses (but not due to PPU scanline rendering).
	virtual inline void a12_clocked() {};
	
protected:
	// Services provided for derived mapper classes
	Nes_Mapper()
	{
		emu_ = NULL;
		static char c;
		state = &c; // TODO: state must not be null?
		state_size = 0;
	}
	
	// Register state data to automatically save and load. Be sure the binary
	// layout is suitable for use in a file, including any byte-order issues.
	// Automatically cleared to zero by default reset().
	inline void register_state(void* p, unsigned s)
	{
	 state = p;
	 state_size = s;
	}
	
	// Enable 8K of RAM at 0x6000-0x7FFF, optionally read-only.
	inline void enable_sram( bool enabled = true, bool read_only = false ) { emu_->enable_sram( enabled, read_only ); }
	
	// Cause CPU writes within given address range to call mapper's write() function.
	// Might map a larger address range, which the mapper can ignore and pass to
	// Nes_Mapper::write(). The range 0x8000-0xffff is always intercepted by the mapper.
	inline void intercept_writes( nes_addr_t addr, unsigned size ) {	emu().add_mapper_intercept( addr, size, false, true ); }
	
	// Cause CPU reads within given address range to call mapper's read() function.
	// Might map a larger address range, which the mapper can ignore and pass to
	// Nes_Mapper::read(). CPU opcode/operand reads and low-memory reads always
	// go directly to memory and cannot be intercepted.
	inline void intercept_reads( nes_addr_t addr, unsigned size )
	{
		emu().add_mapper_intercept( addr, size, true, false );
	}
	
	// Bank sizes for mapping
	enum bank_size_t { // 1 << bank_Xk = X * 1024
		bank_1k  = 10,
		bank_2k  = 11,
		bank_4k  = 12,
		bank_8k  = 13,
		bank_16k = 14,
		bank_32k = 15
	};
	
	// Index of last PRG/CHR bank. Last_bank selects last bank, last_bank - 1
	// selects next-to-last bank, etc.
	enum { last_bank = -1 };
	
	// Map 'size' bytes from 'PRG + bank * size' to CPU address space starting at 'addr'
	void set_prg_bank( nes_addr_t addr, bank_size_t bs, int bank )
	{
		int bank_size = 1 << bs;
		
		int bank_count = cart_->prg_size() >> bs;
		if ( bank < 0 )
			bank += bank_count;
		
		if ( bank >= bank_count )
			bank %= bank_count;
		
		emu().map_code( addr, bank_size, cart_->prg() + (bank << bs) );
		
		if ( unsigned (addr - 0x6000) < 0x2000 )
			emu().enable_prg_6000();
	}

	// Map 'size' bytes from 'CHR + bank * size' to PPU address space starting at 'addr'
	inline void set_chr_bank( nes_addr_t addr, bank_size_t bs, int bank )
	{
		emu().ppu.render_until( emu().clock() ); 
		emu().ppu.set_chr_bank( addr, 1 << bs, bank << bs );
	}

	inline void set_chr_bank_ex( nes_addr_t addr, bank_size_t bs, int bank )
	{
		emu().ppu.render_until( emu().clock() ); 
		emu().ppu.set_chr_bank_ex( addr, 1 << bs, bank << bs );
	}
	
	// Set PPU mirroring. All mappings implemented using mirror_manual().
	inline void mirror_manual( int page0, int page1, int page2, int page3 )
	{
		emu().ppu.render_bg_until( emu().clock() ); 
		emu().ppu.set_nt_banks( page0, page1, page2, page3 );
	}

	inline void mirror_horiz(  int p = 0) { mirror_manual( p, p, p ^ 1, p ^ 1 ); }
	inline void mirror_vert(   int p = 0 ) { mirror_manual( p, p ^ 1, p, p ^ 1 ); }
	inline void mirror_single( int p ) { mirror_manual( p, p, p, p ); }
	inline void mirror_full()          { mirror_manual( 0, 1, 2, 3 ); }
	
	// True if PPU rendering is enabled. Some mappers watch PPU memory accesses to determine
	// when scanlines occur, and can only do this when rendering is enabled.
	inline bool ppu_enabled() const { return emu().ppu.w2001 & 0x08; }
	
	// Cartridge being emulated
	Nes_Cart const& cart() const { return *cart_; }
	
	// Must be called when next_irq()'s return value is earlier than previous,
	// current CPU run can be stopped earlier. Best to call whenever time may
	// have changed (no performance impact if called even when time didn't change).
	inline void irq_changed()	{ emu_->irq_changed(); }
	
	// Handle data written to mapper that doesn't handle bus conflict arising due to
	// PRG also reading data. Returns data that mapper should act as if were
	// written. Currently always returns 'data' and just checks that data written is
	// the same as byte in PRG at same address and writes debug message if it doesn't.
	int handle_bus_conflict( nes_addr_t addr, int data ) { return data; }
	
	// Reference to emulator that uses this mapper.
	Nes_Core& emu() const { return *emu_; }
	
protected:
	// Services derived classes provide
	
	// Read state from snapshot. Default reads data into registered state, then calls
	// apply_mapping().
	inline void read_state( mapper_state_t const& in )
	{
		memset( state, 0, state_size );
		in.read( state, state_size );
		apply_mapping();
	}
	
	// Apply current mapping state to hardware. Called after reading mapper state
	// from a snapshot.
	virtual void apply_mapping() = 0;
	
	// Called by default reset() before apply_mapping() is called.
	virtual void reset_state() { }
	
	// End of general interface
private:
	Nes_Core* emu_;
	void* state;
	unsigned state_size;
	Nes_Cart const* cart_;
	
	// Sets mirroring, maps first 8K CHR in, first and last 16K of PRG,
	// intercepts writes to upper half of memory, and clears registered state.
	inline void default_reset_state()
	{
		int mirroring = cart_->mirroring();
		if ( mirroring & 8 )
			mirror_full();
		else if ( mirroring & 1 )
			mirror_vert();
		else
			mirror_horiz();
		
		set_chr_bank( 0, bank_8k, 0 );
		
		set_prg_bank( 0x8000, bank_16k, 0 );
		set_prg_bank( 0xC000, bank_16k, last_bank );
		
		intercept_writes( 0x8000, 0x8000 );
		
		memset( state, 0, state_size );
	}
};

