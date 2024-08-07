
#pragma once

// Konami VRC6 sound chip emulator
// Snd_Emu 0.1.7

#include "../apu.hpp"
#include "../blipBuffer.hpp"
#include <cstdint>

namespace quickerNES
{

struct vrc6_apu_state_t;

class Vrc6_Apu
{
  public:
  Vrc6_Apu();
  ~Vrc6_Apu();

  // See Apu.h for reference
  void reset();
  void volume(double);
  void treble_eq(blip_eq_t const &);
  void output(Blip_Buffer *);
  enum
  {
    osc_count = 3
  };
  void osc_output(int index, Blip_Buffer *);
  void end_frame(nes_time_t);
  void save_state(vrc6_apu_state_t *) const;
  void load_state(vrc6_apu_state_t const &);

  // Oscillator 0 write-only registers are at $9000-$9002
  // Oscillator 1 write-only registers are at $A000-$A002
  // Oscillator 2 write-only registers are at $B000-$B002
  enum
  {
    reg_count = 3
  };
  enum
  {
    base_addr = 0x9000
  };
  enum
  {
    addr_step = 0x1000
  };
  void write_osc(nes_time_t, int osc, int reg, int data);

  private:
  // noncopyable
  Vrc6_Apu(const Vrc6_Apu &);
  Vrc6_Apu &operator=(const Vrc6_Apu &);

  struct Vrc6_Osc
  {
    uint8_t regs[3];
    Blip_Buffer *output;
    int delay;
    int last_amp;
    int phase;
    int amp; // only used by saw

    int period() const
    {
      return (regs[2] & 0x0f) * 0x100L + regs[1] + 1;
    }
  };

  Vrc6_Osc oscs[osc_count];
  nes_time_t last_time;

  Blip_Synth<blip_med_quality, 1> saw_synth;
  Blip_Synth<blip_good_quality, 1> square_synth;

  void run_until(nes_time_t);
  void run_square(Vrc6_Osc &osc, nes_time_t);
  void run_saw(nes_time_t);
};

struct vrc6_apu_state_t
{
  uint8_t regs[3][3];
  uint8_t saw_amp;
  uint16_t delays[3];
  uint8_t phases[3];
  uint8_t unused;
};
static_assert(sizeof(vrc6_apu_state_t) == 20);

inline void Vrc6_Apu::osc_output(int i, Blip_Buffer *buf)
{
  oscs[i].output = buf;
}

inline void Vrc6_Apu::volume(double v)
{
  double const factor = 0.0967 * 2;
  saw_synth.volume(factor / 31 * v);
  square_synth.volume(factor * 0.5 / 15 * v);
}

inline void Vrc6_Apu::treble_eq(blip_eq_t const &eq)
{
  saw_synth.treble_eq(eq);
  square_synth.treble_eq(eq);
}

} // namespace quickerNES