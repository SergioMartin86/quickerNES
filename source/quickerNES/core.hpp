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

// Emu 0.7.0

#include "cpu.hpp"
#include "apu/apu.hpp"
#include "mappers/mapper.hpp"
#include "ppu/ppu.hpp"
#include <cstdio>
#include <string>

namespace quickerNES
{

class Cart;

#undef NES_EMU_CPU_HOOK
#ifndef NES_EMU_CPU_HOOK
  #define NES_EMU_CPU_HOOK(cpu, end_time) cpu::run(end_time)
#endif

bool const wait_states_enabled = true;
bool const single_instruction_mode = false; // for debugging irq/nmi timing issues
const int unmapped_fill = Cpu::page_wrap_opcode;
unsigned const low_ram_size = 0x800;
unsigned const low_ram_end = 0x2000;
unsigned const sram_end = 0x8000;
const int irq_inhibit_mask = 0x04;

struct nes_state_t
{
  uint16_t timestamp; // CPU clocks * 15 (for NTSC)
  uint8_t pal;
  uint8_t unused[1];
  uint32_t frame_count; // number of frames emulated since power-up
};

struct joypad_state_t
{
  uint32_t joypad_latches[2]; // joypad 1 & 2 shift registers
  uint8_t w4016;              // strobe
  uint8_t unused[3];
};
static_assert(sizeof(joypad_state_t) == 12);

struct cpu_state_t
{
  uint16_t pc;
  uint8_t s;
  uint8_t p;
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t unused[1];
};
static_assert(sizeof(cpu_state_t) == 8);

class Core : private Cpu
{
  typedef Cpu cpu;

  public:
  Core() : ppu(this)
  {
    cart = NULL;
    impl = NULL;
    mapper = NULL;
    memset(&nes, 0, sizeof nes);
    memset(&joypad, 0, sizeof joypad);
  }

  ~Core()
  {
    close();
    delete impl;
  }

  const char *init()
  {
    if (!impl)
    {
      impl = new impl_t;
      impl->apu.dmc_reader(read_dmc, this);
      impl->apu.irq_notifier(apu_irq_changed, this);
    }

    return 0;
  }

  void open(Cart const *new_cart)
  {
    close();
    init();

    // Getting cartdrige mapper code
    auto mapperCode = new_cart->mapper_code();

    // Getting mapper corresponding to that code
    mapper = Mapper::getMapperFromCode(mapperCode);

    // If no mapper was found, return null (error) now
    if (mapper == nullptr)
    {
      fprintf(stderr, "Could not find mapper for code: %u\n", mapperCode);
      exit(-1);
    }

    // Assigning backwards pointers to cartdrige and emulator now
    mapper->cart_ = new_cart;
    mapper->emu_ = this;

    ppu.open_chr(new_cart->chr(), new_cart->chr_size());

    cart = new_cart;
    memset(impl->unmapped_page, unmapped_fill, sizeof impl->unmapped_page);
    reset(true, true);
  }

  size_t serializeFullState(uint8_t *buffer) const
  {
    size_t pos = 0;
    std::string headerCode;
    const uint32_t headerSize = sizeof(char) * 4;
    uint32_t blockSize = 0;
    void *dataSource;

    headerCode = "NESS"; // NESS Block
    blockSize = 0xFFFFFFFF;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;

    headerCode = "TIME"; // TIME Block
    nes_state_t state = nes;
    state.timestamp *= 5;
    blockSize = sizeof(nes_state_t);
    dataSource = (void *)&state;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "CPUR"; // CPUR Block
    cpu_state_t s;
    memset(&s, 0, sizeof s);
    s.pc = r.pc;
    s.s = r.sp;
    s.a = r.a;
    s.x = r.x;
    s.y = r.y;
    s.p = r.status;
    blockSize = sizeof(cpu_state_t);
    dataSource = (void *)&s;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "PPUR"; // PPUR Block
    blockSize = sizeof(ppu_state_t);
    dataSource = (void *)&ppu;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "APUR"; // APUR Block
    Apu::apu_state_t apuState;
    impl->apu.save_state(&apuState);
    blockSize = sizeof(Apu::apu_state_t);
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &apuState, blockSize);
    pos += blockSize;

    headerCode = "CTRL"; // CTRL Block
    blockSize = sizeof(joypad_state_t);
    dataSource = (void *)&joypad;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "MAPR"; // MAPR Block
    blockSize = mapper->state_size;
    dataSource = (void *)mapper->state;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "LRAM"; // LRAM Block
    blockSize = low_ram_size;
    dataSource = (void *)low_mem;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "SPRT"; // SPRT Block
    blockSize = Ppu::spr_ram_size;
    dataSource = (void *)ppu.spr_ram;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "NTAB"; // NTAB Block
    size_t nametable_size = 0x800;
    if (ppu.nt_banks[3] >= &ppu.impl->nt_ram[0xC00]) nametable_size = 0x1000;
    blockSize = nametable_size;
    dataSource = (void *)ppu.impl->nt_ram;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    if (ppu.chr_is_writable)
    {
      headerCode = "CHRR"; // CHRR Block
      blockSize = ppu.chr_size;
      dataSource = (void *)ppu.impl->chr_ram;
      if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
      pos += blockSize;
    }

    if (sram_present)
    {
      headerCode = "SRAM"; // SRAM Block
      blockSize = impl->sram_size;
      dataSource = (void *)impl->sram;
      if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
      pos += blockSize;
    }

    headerCode = "gend"; // gend Block
    blockSize = 0;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;

    return pos; // Bytes written
  }

  size_t deserializeFullState(const uint8_t *buffer)
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
    memcpy(&nesState, &buffer[pos], blockSize);
    pos += blockSize;
    nes = nesState;
    nes.timestamp /= 5;

    // CPUR Block
    cpu_state_t s;
    blockSize = sizeof(cpu_state_t);
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)&s, &buffer[pos], blockSize);
    pos += blockSize;
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
    memcpy((void *)&ppu, &buffer[pos], blockSize);
    pos += blockSize;

    // APUR Block
    Apu::apu_state_t apuState;
    blockSize = sizeof(Apu::apu_state_t);
    pos += headerSize;
    pos += headerSize;
    memcpy(&apuState, &buffer[pos], blockSize);
    pos += blockSize;
    impl->apu.load_state(apuState);
    impl->apu.end_frame(-(int)nes.timestamp / ppu_overclock);

    // CTRL Block
    blockSize = sizeof(joypad_state_t);
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)&joypad, &buffer[pos], blockSize);
    pos += blockSize;

    // MAPR Block
    mapper->default_reset_state();
    blockSize = mapper->state_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)mapper->state, &buffer[pos], blockSize);
    pos += blockSize;
    mapper->apply_mapping();

    // LRAM Block
    blockSize = low_ram_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)low_mem, &buffer[pos], blockSize);
    pos += blockSize;

    // SPRT Block
    blockSize = Ppu::spr_ram_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)ppu.spr_ram, &buffer[pos], blockSize);
    pos += blockSize;

    // NTAB Block
    size_t nametable_size = 0x800;
    if (ppu.nt_banks[3] >= &ppu.impl->nt_ram[0xC00]) nametable_size = 0x1000;
    blockSize = nametable_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)ppu.impl->nt_ram, &buffer[pos], blockSize);
    pos += blockSize;

    if (ppu.chr_is_writable)
    {
      // CHRR Block
      blockSize = ppu.chr_size;
      pos += headerSize;
      pos += headerSize;
      memcpy((void *)ppu.impl->chr_ram, &buffer[pos], blockSize);
      pos += blockSize;
    }

    if (sram_present)
    {
      // SRAM Block
      blockSize = impl->sram_size;
      pos += headerSize;
      pos += headerSize;
      memcpy((void *)impl->sram, &buffer[pos], blockSize);
      pos += blockSize;
      enable_sram(true);
    }

    // headerCode = "gend"; // gend Block
    pos += headerSize;
    pos += headerSize;

    return pos; // Bytes read
  }

size_t serializeLiteState(uint8_t *buffer) const
  {
    size_t pos = 0;
    std::string headerCode;
    const uint32_t headerSize = sizeof(char) * 4;
    uint32_t blockSize = 0;
    void *dataSource;

    headerCode = "NESS"; // NESS Block
    blockSize = 0xFFFFFFFF;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;

    headerCode = "TIME"; // TIME Block
    nes_state_t state = nes;
    state.timestamp *= 5;
    blockSize = sizeof(nes_state_t);
    dataSource = (void *)&state;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "CPUR"; // CPUR Block
    cpu_state_t s;
    memset(&s, 0, sizeof s);
    s.pc = r.pc;
    s.s = r.sp;
    s.a = r.a;
    s.x = r.x;
    s.y = r.y;
    s.p = r.status;
    blockSize = sizeof(cpu_state_t);
    dataSource = (void *)&s;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "PPUR"; // PPUR Block
    blockSize = sizeof(ppu_state_t);
    dataSource = (void *)&ppu;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "APUR"; // APUR Block
    Apu::apu_state_t apuState;
    impl->apu.save_state(&apuState);
    blockSize = sizeof(Apu::apu_state_t);
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &apuState, blockSize);
    pos += blockSize;

    headerCode = "CTRL"; // CTRL Block
    blockSize = sizeof(joypad_state_t);
    dataSource = (void *)&joypad;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "MAPR"; // MAPR Block
    blockSize = mapper->state_size;
    dataSource = (void *)mapper->state;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "LRAM"; // LRAM Block
    blockSize = low_ram_size;
    dataSource = (void *)low_mem;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "SPRT"; // SPRT Block
    blockSize = Ppu::spr_ram_size;
    dataSource = (void *)ppu.spr_ram;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    headerCode = "NTAB"; // NTAB Block
    size_t nametable_size = 0x800;
    if (ppu.nt_banks[3] >= &ppu.impl->nt_ram[0xC00]) nametable_size = 0x1000;
    blockSize = nametable_size;
    dataSource = (void *)ppu.impl->nt_ram;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
    pos += blockSize;

    if (ppu.chr_is_writable)
    {
      headerCode = "CHRR"; // CHRR Block
      blockSize = ppu.chr_size;
      dataSource = (void *)ppu.impl->chr_ram;
      if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
      pos += blockSize;
    }

    if (sram_present)
    {
      headerCode = "SRAM"; // SRAM Block
      blockSize = impl->sram_size;
      dataSource = (void *)impl->sram;
      if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
      pos += headerSize;
      if (buffer != nullptr) memcpy(&buffer[pos], dataSource, blockSize);
      pos += blockSize;
    }

    headerCode = "gend"; // gend Block
    blockSize = 0;
    if (buffer != nullptr) memcpy(&buffer[pos], headerCode.data(), headerSize);
    pos += headerSize;
    if (buffer != nullptr) memcpy(&buffer[pos], &blockSize, headerSize);
    pos += headerSize;

    return pos; // Bytes written
  }

  size_t deserializeLiteState(const uint8_t *buffer)
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
    memcpy(&nesState, &buffer[pos], blockSize);
    pos += blockSize;
    nes = nesState;
    nes.timestamp /= 5;

    // CPUR Block
    cpu_state_t s;
    blockSize = sizeof(cpu_state_t);
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)&s, &buffer[pos], blockSize);
    pos += blockSize;
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
    memcpy((void *)&ppu, &buffer[pos], blockSize);
    pos += blockSize;

    // APUR Block
    Apu::apu_state_t apuState;
    blockSize = sizeof(Apu::apu_state_t);
    pos += headerSize;
    pos += headerSize;
    memcpy(&apuState, &buffer[pos], blockSize);
    pos += blockSize;
    impl->apu.load_state(apuState);
    impl->apu.end_frame(-(int)nes.timestamp / ppu_overclock);

    // CTRL Block
    blockSize = sizeof(joypad_state_t);
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)&joypad, &buffer[pos], blockSize);
    pos += blockSize;

    // MAPR Block
    mapper->default_reset_state();
    blockSize = mapper->state_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)mapper->state, &buffer[pos], blockSize);
    pos += blockSize;
    mapper->apply_mapping();

    // LRAM Block
    blockSize = low_ram_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)low_mem, &buffer[pos], blockSize);
    pos += blockSize;

    // SPRT Block
    blockSize = Ppu::spr_ram_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)ppu.spr_ram, &buffer[pos], blockSize);
    pos += blockSize;

    // NTAB Block
    size_t nametable_size = 0x800;
    if (ppu.nt_banks[3] >= &ppu.impl->nt_ram[0xC00]) nametable_size = 0x1000;
    blockSize = nametable_size;
    pos += headerSize;
    pos += headerSize;
    memcpy((void *)ppu.impl->nt_ram, &buffer[pos], blockSize);
    pos += blockSize;

    if (ppu.chr_is_writable)
    {
      // CHRR Block
      blockSize = ppu.chr_size;
      pos += headerSize;
      pos += headerSize;
      memcpy((void *)ppu.impl->chr_ram, &buffer[pos], blockSize);
      pos += blockSize;
    }

    if (sram_present)
    {
      // SRAM Block
      blockSize = impl->sram_size;
      pos += headerSize;
      pos += headerSize;
      memcpy((void *)impl->sram, &buffer[pos], blockSize);
      pos += blockSize;
      enable_sram(true);
    }

    // headerCode = "gend"; // gend Block
    pos += headerSize;
    pos += headerSize;

    return pos; // Bytes read
  }

  void reset(bool full_reset, bool erase_battery_ram)
  {
    if (full_reset)
    {
      cpu::reset(impl->unmapped_page);
      cpu_time_offset = -1;
      clock_ = 0;

      // Low RAM
      memset(cpu::low_mem, 0xFF, low_ram_size);
      cpu::low_mem[8] = 0xf7;
      cpu::low_mem[9] = 0xef;
      cpu::low_mem[10] = 0xdf;
      cpu::low_mem[15] = 0xbf;

      // SRAM
      lrom_readable = 0;
      sram_present = true;
      enable_sram(false);
      if (!cart->has_battery_ram() || erase_battery_ram)
        memset(impl->sram, 0xFF, impl->sram_size);

      joypad.joypad_latches[0] = 0;
      joypad.joypad_latches[1] = 0;

      nes.frame_count = 0;
    }

    // to do: emulate partial reset

    ppu.reset(full_reset);
    impl->apu.reset();

    mapper->reset();

    cpu::r.pc = read_vector(0xFFFC);
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
    current_joypad[0] = (joypad1 |= ~0xFF);
    current_joypad[1] = (joypad2 |= ~0xFF);

    cpu_time_offset = ppu.begin_frame(nes.timestamp) - 1;
    ppu_2002_time = 0;
    clock_ = cpu_time_offset;

    // TODO: clean this fucking mess up
    auto t0 = emulate_frame_();
    impl->apu.run_until_(t0);
    clock_ = cpu_time_offset;
    auto t1 = cpu_time();
    impl->apu.run_until_(t1);

    nes_time_t ppu_frame_length = ppu.frame_length();
    nes_time_t length = cpu_time();
    nes.timestamp = ppu.end_frame(length);
    mapper->end_frame(length);

    impl->apu.end_frame(ppu_frame_length);

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

  void irq_changed()
  {
    cpu_set_irq_time(earliest_irq(cpu_time()));
  }

  void event_changed()
  {
    cpu_set_end_time(earliest_event(cpu_time()));
  }

  public:
  private:
  friend class Emu;

  struct impl_t
  {
    enum
    {
      sram_size = 0x2000
    };
    uint8_t sram[sram_size];
    Apu apu;

    // extra byte allows CPU to always read operand of instruction, which
    // might go past end of data
    uint8_t unmapped_page[Cpu::page_size + 1];
  };
  impl_t *impl; // keep large arrays separate
  unsigned long error_count;
  bool sram_present;

  public:
  unsigned long current_joypad[2];
  Cart const *cart;
  Mapper *mapper;
  nes_state_t nes;
  Ppu ppu;

  private:
  // noncopyable
  Core(const Core &);
  Core &operator=(const Core &);

  // Timing
  nes_time_t ppu_2002_time;
  void disable_rendering() { clock_ = 0; }

  inline nes_time_t earliest_irq(nes_time_t present)
  {
    return std::min(impl->apu.earliest_irq(present), mapper->next_irq(present));
  }

  inline nes_time_t ppu_frame_length(nes_time_t present)
  {
    nes_time_t t = ppu.frame_length();
    if (t > present)
      return t;

    ppu.render_bg_until(clock()); // to do: why this call to clock() rather than using present?
    return ppu.frame_length();
  }

  inline nes_time_t earliest_event(nes_time_t present)
  {
    // PPU frame
    nes_time_t t = ppu_frame_length(present);

    // DMC
    if (wait_states_enabled)
      t = std::min(t, impl->apu.next_dmc_read_time() + 1);

    // NMI
    t = std::min(t, ppu.nmi_time());

    if (single_instruction_mode)
      t = std::min(t, present + 1);

    return t;
  }

  // APU and Joypad
  joypad_state_t joypad;

  int read_io(nes_addr_t addr)
  {
    if ((addr & 0xFFFE) == 0x4016)
    {
      // to do: to aid with recording, doesn't emulate transparent latch,
      // so a game that held strobe at 1 and read $4016 or $4017 would not get
      // the current A status as occurs on a NES
      unsigned long result = joypad.joypad_latches[addr & 1];
      if (!(joypad.w4016 & 1))
        joypad.joypad_latches[addr & 1] = (result >> 1) | 0x80000000;
      return result & 1;
    }

    if (addr == Apu::status_addr)
      return impl->apu.read_status(clock());

    return addr >> 8; // simulate open bus
  }

  void write_io(nes_addr_t addr, int data)
  {
    // sprite dma
    if (addr == 0x4014)
    {
      ppu.dma_sprites(clock(), cpu::get_code(data * 0x100));
      cpu_adjust_time(513);
      return;
    }

    // joypad strobe
    if (addr == 0x4016)
    {
      // if strobe goes low, latch data
      if (joypad.w4016 & 1 & ~data)
      {
        joypad.joypad_latches[0] = current_joypad[0];
        joypad.joypad_latches[1] = current_joypad[1];
      }
      joypad.w4016 = data;
      return;
    }

    // apu
    if (unsigned(addr - impl->apu.start_addr) <= impl->apu.end_addr - impl->apu.start_addr)
    {
      impl->apu.write_register(clock(), addr, data);
      if (wait_states_enabled)
      {
        if (addr == 0x4010 || (addr == 0x4015 && (data & 0x10)))
        {
          impl->apu.run_until(clock() + 1);
          event_changed();
        }
      }
      return;
    }
  }

  static inline int read_dmc(void *data, nes_addr_t addr)
  {
    Core *emu = (Core *)data;
    int result = *emu->cpu::get_code(addr);
    if (wait_states_enabled)
      emu->cpu_adjust_time(4);
    return result;
  }

  static inline void apu_irq_changed(void *emu)
  {
    ((Core *)emu)->irq_changed();
  }

  // CPU
  unsigned sram_readable;
  unsigned sram_writable;
  unsigned lrom_readable;
  nes_time_t clock_;
  nes_time_t cpu_time_offset;

  nes_time_t emulate_frame_()
  {
    Cpu::result_t last_result = cpu::result_cycles;
    int extra_instructions = 0;
    while (true)
    {
      // Add DMC wait-states to CPU time
      if (wait_states_enabled)
      {
        impl->apu.run_until(cpu_time());
        clock_ = cpu_time_offset;
      }

      nes_time_t present = cpu_time();
      if (present >= ppu_frame_length(present))
      {
        if (ppu.nmi_time() <= present)
        {
          // NMI will occur next, so delayed CLI and SEI don't need to be handled.
          // If NMI will occur normally ($2000.7 and $2002.7 set), let it occur
          // next frame, otherwise vector it now.

          if (!(ppu.w2000 & 0x80 & ppu.r2002))
          {
            /* vectored NMI at end of frame */
            vector_interrupt(0xFFFA);
            present += 7;
          }
          return present;
        }

        if (extra_instructions > 2)
        {
          return present;
        }

        if (last_result != cpu::result_cli && last_result != cpu::result_sei &&
            (ppu.nmi_time() >= 0x10000 || (ppu.w2000 & 0x80 & ppu.r2002)))
          return present;

        /* Executing extra instructions for frame */
        extra_instructions++; // execute one more instruction
      }

      // NMI
      if (present >= ppu.nmi_time())
      {
        ppu.acknowledge_nmi();
        vector_interrupt(0xFFFA);
        last_result = cpu::result_cycles; // most recent sei/cli won't be delayed now
      }

      // IRQ
      nes_time_t irq_time = earliest_irq(present);
      cpu_set_irq_time(irq_time);
      if (present >= irq_time && (!(cpu::r.status & irq_inhibit_mask) ||
                                  last_result == cpu::result_sei))
      {
        if (last_result != cpu::result_cli)
        {
          /* IRQ vectored */
          mapper->run_until(present);
          vector_interrupt(0xFFFE);
        }
        else
        {
          // CLI delays IRQ
          cpu_set_irq_time(present + 1);
        }
      }

      // CPU
      nes_time_t end_time = earliest_event(present);
      if (extra_instructions)
        end_time = present + 1;
      unsigned long cpu_error_count = cpu::error_count();
      last_result = NES_EMU_CPU_HOOK(cpu, end_time - cpu_time_offset - 1);
      cpu_adjust_time(cpu::time());
      clock_ = cpu_time_offset;
      error_count += cpu::error_count() - cpu_error_count;
    }
  }

  nes_addr_t read_vector(nes_addr_t addr)
  {
    uint8_t const *p = cpu::get_code(addr);
    return p[1] * 0x100 + p[0];
  }

  void vector_interrupt(nes_addr_t vector)
  {
    cpu::push_byte(cpu::r.pc >> 8);
    cpu::push_byte(cpu::r.pc & 0xFF);
    cpu::push_byte(cpu::r.status | 0x20); // reserved bit is set

    cpu_adjust_time(7);
    cpu::r.status |= irq_inhibit_mask;
    cpu::r.pc = read_vector(vector);
  }

  static void log_unmapped(nes_addr_t addr, int data = -1);
  void cpu_set_irq_time(nes_time_t t) { cpu::set_irq_time_(t - 1 - cpu_time_offset); }
  void cpu_set_end_time(nes_time_t t) { cpu::set_end_time_(t - 1 - cpu_time_offset); }
  nes_time_t cpu_time() const { return clock_ + 1; }

  inline void cpu_adjust_time(int n)
  {
    ppu_2002_time -= n;
    cpu_time_offset += n;
    cpu::reduce_limit(n);
  }

  public:
  private:
  friend class Ppu;
  void set_ppu_2002_time(nes_time_t t) { ppu_2002_time = t - 1 - cpu_time_offset; }

  public:
  private:
  friend class Mapper;

  void enable_prg_6000()
  {
    sram_writable = 0;
    sram_readable = 0;
    lrom_readable = 0x8000;
  }

  void enable_sram(bool b, bool read_only = false)
  {
    sram_writable = 0;
    if (b)
    {
      if (!sram_present)
      {
        sram_present = true;
        memset(impl->sram, 0xFF, impl->sram_size);
      }
      sram_readable = sram_end;
      if (!read_only)
        sram_writable = sram_end;
      cpu::map_code(0x6000, impl->sram_size, impl->sram);
    }
    else
    {
      sram_readable = 0;
      for (int i = 0; i < impl->sram_size; i += cpu::page_size)
        cpu::map_code(0x6000 + i, cpu::page_size, impl->unmapped_page);
    }
  }

  nes_time_t clock() const { return clock_; }

  void add_mapper_intercept(nes_addr_t addr, unsigned size, bool read, bool write)
  {
    int end = (addr + size + (page_size - 1)) >> page_bits;
    for (int page = addr >> page_bits; page < end; page++)
    {
      data_reader_mapped[page] |= read;
      data_writer_mapped[page] |= write;
    }
  }

  public:
  private:
  friend class Cpu;
  int cpu_read_ppu(nes_addr_t, nes_time_t);
  int cpu_read(nes_addr_t, nes_time_t);
  void cpu_write(nes_addr_t, int data, nes_time_t);
  void cpu_write_2007(int data);

  private:
  unsigned char data_reader_mapped[page_count + 1]; // extra entry for overflow
  unsigned char data_writer_mapped[page_count + 1];
};

inline int Core::cpu_read(nes_addr_t addr, nes_time_t time)
{
  {
    int result = cpu::low_mem[addr & 0x7FF];
    if (!(addr & 0xE000))
      return result;
  }

  {
    int result = *cpu::get_code(addr);
    if (addr > 0x7FFF)
      return result;
  }

  time += cpu_time_offset;
  if (addr < 0x4000)
    return ppu.read(addr, time);

  clock_ = time;
  if (data_reader_mapped[addr >> page_bits])
  {
    int result = mapper->read(time, addr);
    if (result >= 0)
      return result;
  }

  if (addr < 0x6000)
    return read_io(addr);

  if (addr < sram_readable)
    return impl->sram[addr & (impl_t::sram_size - 1)];

  if (addr < lrom_readable)
    return *cpu::get_code(addr);

  return addr >> 8; // simulate open bus
}

inline int Core::cpu_read_ppu(nes_addr_t addr, nes_time_t time)
{
  // LOG_FREQ( "cpu_read_ppu", 16, addr >> 12 );

  // Read of status register (0x2002) is heavily optimized since many games
  // poll it hundreds of times per frame.
  nes_time_t next = ppu_2002_time;
  int result = ppu.r2002;
  if (addr == 0x2002)
  {
    ppu.second_write = false;
    if (time >= next)
      result = ppu.read_2002(time + cpu_time_offset);
  }
  else
  {
    result = cpu::low_mem[addr & 0x7FF];
    if (addr >= 0x2000)
      result = cpu_read(addr, time);
  }

  return result;
}

inline void Core::cpu_write_2007(int data)
{
  // ppu.write_2007() is inlined
  if (ppu.write_2007(data) & Ppu::vaddr_clock_mask)
    mapper->a12_clocked();
}

inline void Core::cpu_write(nes_addr_t addr, int data, nes_time_t time)
{
  // LOG_FREQ( "cpu_write", 16, addr >> 12 );

  if (!(addr & 0xE000))
  {
    cpu::low_mem[addr & 0x7FF] = data;
    return;
  }

  time += cpu_time_offset;
  if (addr < 0x4000)
  {
    if ((addr & 7) == 7)
      cpu_write_2007(data);
    else
      ppu.write(time, addr, data);
    return;
  }

  clock_ = time;
  if (data_writer_mapped[addr >> page_bits] && mapper->write_intercepted(time, addr, data))
    return;

  if (addr < 0x6000)
  {
    write_io(addr, data);
    return;
  }

  if (addr < sram_writable)
  {
    impl->sram[addr & (impl_t::sram_size - 1)] = data;
    return;
  }

  if (addr > 0x7FFF)
  {
    mapper->write(clock_, addr, data);
    return;
  }
}

#define NES_CPU_READ_PPU(cpu, addr, time) \
  static_cast<Core &>(*cpu).cpu_read_ppu(addr, time)

#define NES_CPU_READ(cpu, addr, time) \
  static_cast<Core &>(*cpu).cpu_read(addr, time)

#define NES_CPU_WRITEX(cpu, addr, data, time)                  \
  {                                                            \
    static_cast<Core &>(*cpu).cpu_write(addr, data, time); \
  }

#define NES_CPU_WRITE(cpu, addr, data, time)                     \
  {                                                              \
    if (addr < 0x800)                                            \
      cpu->low_mem[addr] = data;                                 \
    else if (addr == 0x2007)                                     \
      static_cast<Core &>(*cpu).cpu_write_2007(data);        \
    else                                                         \
      static_cast<Core &>(*cpu).cpu_write(addr, data, time); \
  }

} // namespace quickNES