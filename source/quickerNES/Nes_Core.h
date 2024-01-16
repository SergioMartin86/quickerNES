#pragma once

// Internal NES emulator

// Nes_Emu 0.7.0


#include "blargg_common.h"
#include "Nes_Apu.h"
#include "Nes_Cpu.h"
#include "Nes_Ppu.h"
#include "Nes_Mapper.h"
class Nes_Cart;
class Nes_State;

class Nes_Core : private Nes_Cpu {
	typedef Nes_Cpu cpu;
public:
	Nes_Core();
	~Nes_Core();
	
	const char * init();
	const char * open( Nes_Cart const* );
	void reset( bool full_reset = true, bool erase_battery_ram = false );
	blip_time_t emulate_frame(int joypad1, int joypad2);
	void close();
	
	void save_state( Nes_State* ) const;
	void save_state( Nes_State_* ) const;
	void load_state( Nes_State_ const& );
	
	void irq_changed();
	void event_changed();
	 
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
	nes_time_t earliest_irq( nes_time_t present );
	nes_time_t ppu_frame_length( nes_time_t present );
	nes_time_t earliest_event( nes_time_t present );
	
	// APU and Joypad
	joypad_state_t joypad;
	int  read_io( nes_addr_t );
	void write_io( nes_addr_t, int data );
	static int  read_dmc( void* emu, nes_addr_t );
	static void apu_irq_changed( void* emu );
	
	// CPU
	unsigned sram_readable;
	unsigned sram_writable;
	unsigned lrom_readable;
	nes_time_t clock_;
	nes_time_t cpu_time_offset;
	nes_time_t emulate_frame_();
	nes_addr_t read_vector( nes_addr_t );
	void vector_interrupt( nes_addr_t );
	static void log_unmapped( nes_addr_t addr, int data = -1 );
	void cpu_set_irq_time( nes_time_t t ) { cpu::set_irq_time_( t - 1 - cpu_time_offset ); }
	void cpu_set_end_time( nes_time_t t ) { cpu::set_end_time_( t - 1 - cpu_time_offset ); }
	nes_time_t cpu_time() const { return clock_ + 1; }
	void cpu_adjust_time( int offset );
	
public: private: friend class Nes_Ppu;
	void set_ppu_2002_time( nes_time_t t ) { ppu_2002_time = t - 1 - cpu_time_offset; }
	
public: private: friend class Nes_Mapper;
	void enable_prg_6000();
	void enable_sram( bool enabled, bool read_only = false );
	nes_time_t clock() const { return clock_; }
	void add_mapper_intercept( nes_addr_t start, unsigned size, bool read, bool write );
	
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

