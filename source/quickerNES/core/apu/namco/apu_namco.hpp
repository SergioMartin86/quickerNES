#pragma once

// Namco 106 sound chip emulator
// Snd_Emu 0.1.7

#include "../apu.hpp"
#include <cstdint>

namespace quickerNES
{

struct namco_state_t
{
  uint8_t regs[0x80];
  uint8_t addr;
  uint8_t unused;
  uint8_t positions[8];
  uint32_t delays[8];
};
static_assert(sizeof(namco_state_t) == 172);

class Namco_Apu
{
  public:
  Namco_Apu();
  ~Namco_Apu();

  // See Apu.h for reference.
  void volume(double);
  void treble_eq(const blip_eq_t &);
  void output(Blip_Buffer *);
  enum
  {
    osc_count = 8
  };
  void osc_output(int index, Blip_Buffer *);
  void reset();
  void end_frame(nes_time_t);

  // Read/write data register is at 0x4800
  enum
  {
    data_reg_addr = 0x4800
  };
  void write_data(nes_time_t, int);
  int read_data();

  // Write-only address register is at 0xF800
  enum
  {
    addr_reg_addr = 0xF800
  };
  void write_addr(int);

  // to do: implement save/restore
  void save_state(namco_state_t *out) const;
  void load_state(namco_state_t const &);

  private:
  // noncopyable
  Namco_Apu(const Namco_Apu &);
  Namco_Apu &operator=(const Namco_Apu &);

  struct Namco_Osc
  {
    long delay;
    Blip_Buffer *output;
    short last_amp;
    short wave_pos;
  };

  Namco_Osc oscs[osc_count];

  nes_time_t last_time;
  int addr_reg;

  enum
  {
    reg_count = 0x80
  };
  uint8_t reg[reg_count];
  Blip_Synth<blip_good_quality, 15> synth;

  uint8_t &access();
  void run_until(nes_time_t);
};

inline uint8_t &Namco_Apu::access()
{
  int addr = addr_reg & 0x7f;
  if (addr_reg & 0x80)
    addr_reg = (addr + 1) | 0x80;
  return reg[addr];
}

inline void Namco_Apu::volume(double v) { synth.volume(0.10 / +osc_count * v); }

inline void Namco_Apu::treble_eq(const blip_eq_t &eq) { synth.treble_eq(eq); }

inline void Namco_Apu::write_addr(int v) { addr_reg = v; }

inline int Namco_Apu::read_data() { return access(); }

inline void Namco_Apu::osc_output(int i, Blip_Buffer *buf)
{
  oscs[i].output = buf;
}

inline void Namco_Apu::write_data(nes_time_t time, int data)
{
  run_until(time);
  access() = data;
}

} // namespace quickerNES