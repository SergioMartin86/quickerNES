
// Emu 0.7.0. http://www.slack.net/~ant/

#include "apu_fme7.hpp"
#include <cstring>

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

namespace quickerNES
{

void Fme7_Apu::reset()
{
  last_time = 0;

  for (int i = 0; i < osc_count; i++)
    oscs[i].last_amp = 0;

  fme7_apu_state_t *state = this;
  memset(state, 0, sizeof *state);
}

unsigned char Fme7_Apu::amp_table[16] =
  {
#define ENTRY(n) (unsigned char)(n * +amp_range + 0.5)
    ENTRY(0.0000), ENTRY(0.0078), ENTRY(0.0110), ENTRY(0.0156), ENTRY(0.0221), ENTRY(0.0312), ENTRY(0.0441), ENTRY(0.0624), ENTRY(0.0883), ENTRY(0.1249), ENTRY(0.1766), ENTRY(0.2498), ENTRY(0.3534), ENTRY(0.4998), ENTRY(0.7070), ENTRY(1.0000)
#undef ENTRY
};

void Fme7_Apu::run_until(blip_time_t end_time)
{
  for (int index = 0; index < osc_count; index++)
  {
    int mode = regs[7] >> index;
    int vol_mode = regs[010 + index];
    int volume = amp_table[vol_mode & 0x0f];

    if (!oscs[index].output)
      continue;

    if ((mode & 001) | (vol_mode & 0x10))
      volume = 0; // noise and envelope aren't supported

    // period
    int const period_factor = 16;
    unsigned period = (regs[index * 2 + 1] & 0x0f) * 0x100 * period_factor +
                      regs[index * 2] * period_factor;
    if (period < 50) // around 22 kHz
    {
      volume = 0;
      if (!period) // on my AY-3-8910A, period doesn't have extra one added
        period = period_factor;
    }

    // current amplitude
    int amp = volume;
    if (!phases[index])
      amp = 0;
    int delta = amp - oscs[index].last_amp;
    if (delta)
    {
      oscs[index].last_amp = amp;
      synth.offset(last_time, delta, oscs[index].output);
    }

    blip_time_t time = last_time + delays[index];
    if (time < end_time)
    {
      Blip_Buffer *const osc_output = oscs[index].output;
      int delta = amp * 2 - volume;

      if (volume)
      {
        do
        {
          delta = -delta;
          synth.offset_inline(time, delta, osc_output);
          time += period;
        } while (time < end_time);

        oscs[index].last_amp = (delta + volume) >> 1;
        phases[index] = (delta > 0);
      }
      else
      {
        // maintain phase when silent
        int count = (end_time - time + period - 1) / period;
        phases[index] ^= count & 1;
        time += (long)count * period;
      }
    }

    delays[index] = time - end_time;
  }

  last_time = end_time;
}

} // namespace quickerNES