#pragma once

// Band-limited sound synthesis and buffering
// Blip_Buffer 0.4.0

namespace quickerNES
{

// Time unit at source clock rate
typedef long blip_time_t;

// Output samples are 16-bit signed, with a range of -32768 to 32767
typedef short blip_sample_t;
enum
{
  blip_sample_max = 32767
};

class Blip_Buffer
{
  public:
  // Set output sample rate and buffer length in milliseconds (1/1000 sec, defaults
  // to 1/4 second), then clear buffer. Returns NULL on success, otherwise if there
  // isn't enough memory, returns error without affecting current buffer setup.
  const char *set_sample_rate(long samples_per_sec, int msec_length = 1000 / 4);

  // Set number of source time units per second
  void clock_rate(long);

  // End current time frame of specified duration and make its samples available
  // (along with any still-unread samples) for reading with read_samples(). Begins
  // a new time frame at the end of the current frame.
  void end_frame(blip_time_t time);

  // Read at most 'max_samples' out of buffer into 'dest', removing them from from
  // the buffer. Returns number of samples actually read and removed. If stereo is
  // true, increments 'dest' one extra time after writing each sample, to allow
  // easy interleving of two channels into a stereo output buffer.
  long read_samples(blip_sample_t *dest, long max_samples, int stereo = 0);

  // Additional optional features

  // Current output sample rate
  long sample_rate() const;

  // Length of buffer, in milliseconds
  int length() const;

  // Number of source time units per second
  long clock_rate() const;

  // Set frequency high-pass filter frequency, where higher values reduce bass more
  void bass_freq(int frequency);

  // Number of samples delay from synthesis to samples read out
  int output_latency() const;

  // Remove all available samples and clear buffer to silence. If 'entire_buffer' is
  // false, just clears out any samples waiting rather than the entire buffer.
  void clear(int entire_buffer = 1);

  // Number of samples available for reading with read_samples()
  long samples_avail() const;

  // Remove 'count' samples from those waiting to be read
  void remove_samples(long count);

  // Experimental features

  // Number of raw samples that can be mixed within frame of specified duration.
  long count_samples(blip_time_t duration) const;

  // Mix 'count' samples from 'buf' into buffer.
  void mix_samples(blip_sample_t const *buf, long count);

  // Count number of clocks needed until 'count' samples will be available.
  // If buffer can't even hold 'count' samples, returns number of clocks until
  // buffer becomes full.
  blip_time_t count_clocks(long count) const;

  // not documented yet
  typedef unsigned long blip_resampled_time_t;
  void remove_silence(long count);
  blip_resampled_time_t resampled_duration(int t) const { return t * factor_; }
  blip_resampled_time_t resampled_time(blip_time_t t) const { return t * factor_ + offset_; }
  blip_resampled_time_t clock_rate_factor(long clock_rate) const;

  public:
  Blip_Buffer();
  ~Blip_Buffer();

  // Deprecated
  typedef blip_resampled_time_t resampled_time_t;
  const char *sample_rate(long r) { return set_sample_rate(r); }
  const char *sample_rate(long r, int msec) { return set_sample_rate(r, msec); }

  private:
  // noncopyable
  Blip_Buffer(const Blip_Buffer &);
  Blip_Buffer &operator=(const Blip_Buffer &);

  public:
  typedef long buf_t_;
  unsigned long factor_;
  blip_resampled_time_t offset_;
  buf_t_ *buffer_;
  long buffer_size_;

  private:
  long reader_accum;
  int bass_shift;
  long sample_rate_;
  long clock_rate_;
  int bass_freq_;
  int length_;
  friend class Blip_Reader;

  private:
  // extra information necessary to load state to an exact sample
  buf_t_ extra_buffer[32];
  int extra_length;
  long extra_reader_accum;
  blip_resampled_time_t extra_offset;

  public:
  void SaveAudioBufferState();
  void RestoreAudioBufferState();
};

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

// Number of bits in resample ratio fraction. Higher values give a more accurate ratio
// but reduce maximum buffer size.
#ifndef BLIP_BUFFER_ACCURACY
  #define BLIP_BUFFER_ACCURACY 16
#endif

// Number bits in phase offset. Fewer than 6 bits (64 phase offsets) results in
// noticeable broadband noise when synthesizing high frequency square waves.
// Affects size of Blip_Synth objects since they store the waveform directly.
#ifndef BLIP_PHASE_BITS
  #define BLIP_PHASE_BITS 6
#endif

// Internal
typedef unsigned long blip_resampled_time_t;
int const blip_widest_impulse_ = 16;
int const blip_res = 1 << BLIP_PHASE_BITS;
class blip_eq_t;

class Blip_Synth_
{
  double volume_unit_;
  short *const impulses;
  int const width;
  long kernel_unit;
  int impulses_size() const { return blip_res / 2 * width + 1; }
  void adjust_impulse();

  public:
  Blip_Buffer *buf;
  int last_amp;
  int delta_factor;

  Blip_Synth_(short *impulses, int width);
  void treble_eq(blip_eq_t const &);
  void volume_unit(double);
};

// Quality level. Start with blip_good_quality.
const int blip_med_quality = 8;
const int blip_good_quality = 12;
const int blip_high_quality = 16;

// Range specifies the greatest expected change in amplitude. Calculate it
// by finding the difference between the maximum and minimum expected
// amplitudes (max - min).
template <int quality, int range>
class Blip_Synth
{
  public:
  // Set overall volume of waveform
  void volume(double v) { impl.volume_unit(v * (1.0 / (range < 0 ? -range : range))); }

  // Configure low-pass filter (see notes.txt)
  void treble_eq(blip_eq_t const &eq) { impl.treble_eq(eq); }

  // Get/set Blip_Buffer used for output
  Blip_Buffer *output() const { return impl.buf; }
  void output(Blip_Buffer *b)
  {
    impl.buf = b;
    impl.last_amp = 0;
  }

  // Update amplitude of waveform at given time. Using this requires a separate
  // Blip_Synth for each waveform.
  void update(blip_time_t time, int amplitude);

  // Low-level interface

  // Add an amplitude transition of specified delta, optionally into specified buffer
  // rather than the one set with output(). Delta can be positive or negative.
  // The actual change in amplitude is delta * (volume / range)
  void offset(blip_time_t, int delta, Blip_Buffer *) const;
  void offset(blip_time_t t, int delta) const { offset(t, delta, impl.buf); }

  // Works directly in terms of fractional output samples. Contact author for more.
  void offset_resampled(blip_resampled_time_t, int delta, Blip_Buffer *) const;

  // Same as offset(), except code is inlined for higher performance
  void offset_inline(blip_time_t t, int delta, Blip_Buffer *buf) const
  {
    offset_resampled(t * buf->factor_ + buf->offset_, delta, buf);
  }
  void offset_inline(blip_time_t t, int delta) const
  {
    offset_resampled(t * impl.buf->factor_ + impl.buf->offset_, delta, impl.buf);
  }

  public:
  Blip_Synth() : impl(impulses, quality) {}

  private:
  typedef short imp_t;
  imp_t impulses[blip_res * (quality / 2) + 1];
  Blip_Synth_ impl;
};

// Low-pass equalization parameters
class blip_eq_t
{
  public:
  // Logarithmic rolloff to treble dB at half sampling rate. Negative values reduce
  // treble, small positive values (0 to 5.0) increase treble.
  blip_eq_t(double treble_db = 0);

  // See notes.txt
  blip_eq_t(double treble, long rolloff_freq, long sample_rate, long cutoff_freq = 0);

  private:
  double treble;
  long rolloff_freq;
  long sample_rate;
  long cutoff_freq;
  void generate(float *out, int count) const;
  friend class Blip_Synth_;
};

int const blip_sample_bits = 30;

// Optimized inline sample reader for custom sample formats and mixing of Blip_Buffer samples
class Blip_Reader
{
  public:
  // Begin reading samples from buffer. Returns value to pass to next() (can
  // be ignored if default bass_freq is acceptable).
  int begin(Blip_Buffer &);

  // Current sample
  long read() const { return accum >> (blip_sample_bits - 16); }

  // Current raw sample in full internal resolution
  long read_raw() const { return accum; }

  // Advance to next sample
  void next(int bass_shift = 9) { accum += *buf++ - (accum >> bass_shift); }

  // End reading samples from buffer. The number of samples read must now be removed
  // using Blip_Buffer::remove_samples().
  void end(Blip_Buffer &b) { b.reader_accum = accum; }

  private:
  const Blip_Buffer::buf_t_ *buf;
  long accum;
};

// End of public interface

// Compatibility with older version
const long blip_unscaled = 65535;
const int blip_low_quality = blip_med_quality;
const int blip_best_quality = blip_high_quality;

#define BLIP_FWD(i)                                               \
  {                                                               \
    long t0 = i0 * delta + buf[fwd + i];                          \
    long t1 = imp[blip_res * (i + 1)] * delta + buf[fwd + 1 + i]; \
    i0 = imp[blip_res * (i + 2)];                                 \
    buf[fwd + i] = t0;                                            \
    buf[fwd + 1 + i] = t1;                                        \
  }

#define BLIP_REV(r)                                         \
  {                                                         \
    long t0 = i0 * delta + buf[rev - r];                    \
    long t1 = imp[blip_res * r] * delta + buf[rev + 1 - r]; \
    i0 = imp[blip_res * (r - 1)];                           \
    buf[rev - r] = t0;                                      \
    buf[rev + 1 - r] = t1;                                  \
  }

template <int quality, int range>
inline void Blip_Synth<quality, range>::offset_resampled(blip_resampled_time_t time,
                                                         int delta,
                                                         Blip_Buffer *blip_buf) const
{
  // Fails if time is beyond end of Blip_Buffer, due to a bug in caller code or the
  // need for a longer buffer as set by set_sample_rate().
  delta *= impl.delta_factor;
  int phase = (int)(time >> (BLIP_BUFFER_ACCURACY - BLIP_PHASE_BITS) & (blip_res - 1));
  imp_t const *imp = impulses + blip_res - phase;
  long *buf = blip_buf->buffer_ + (time >> BLIP_BUFFER_ACCURACY);
  long i0 = *imp;

  int const fwd = (blip_widest_impulse_ - quality) / 2;
  int const rev = fwd + quality - 2;

  BLIP_FWD(0)
  if (quality > 8) BLIP_FWD(2)
  if (quality > 12) BLIP_FWD(4)
    {
      int const mid = quality / 2 - 1;
      long t0 = i0 * delta + buf[fwd + mid - 1];
      long t1 = imp[blip_res * mid] * delta + buf[fwd + mid];
      imp = impulses + phase;
      i0 = imp[blip_res * mid];
      buf[fwd + mid - 1] = t0;
      buf[fwd + mid] = t1;
    }
  if (quality > 12) BLIP_REV(6)
  if (quality > 8) BLIP_REV(4)
  BLIP_REV(2)

  long t0 = i0 * delta + buf[rev];
  long t1 = *imp * delta + buf[rev + 1];
  buf[rev] = t0;
  buf[rev + 1] = t1;
}

#undef BLIP_FWD
#undef BLIP_REV

template <int quality, int range>
void Blip_Synth<quality, range>::offset(blip_time_t t, int delta, Blip_Buffer *buf) const
{
  offset_resampled(t * buf->factor_ + buf->offset_, delta, buf);
}

template <int quality, int range>
void Blip_Synth<quality, range>::update(blip_time_t t, int amp)
{
  int delta = amp - impl.last_amp;
  impl.last_amp = amp;
  offset_resampled(t * impl.buf->factor_ + impl.buf->offset_, delta, impl.buf);
}

inline blip_eq_t::blip_eq_t(double t) : treble(t), rolloff_freq(0), sample_rate(44100), cutoff_freq(0) {}
inline blip_eq_t::blip_eq_t(double t, long rf, long sr, long cf) : treble(t), rolloff_freq(rf), sample_rate(sr), cutoff_freq(cf) {}

inline int Blip_Buffer::length() const { return length_; }
inline long Blip_Buffer::samples_avail() const { return (long)(offset_ >> BLIP_BUFFER_ACCURACY); }
inline long Blip_Buffer::sample_rate() const { return sample_rate_; }
inline int Blip_Buffer::output_latency() const { return blip_widest_impulse_ / 2; }
inline long Blip_Buffer::clock_rate() const { return clock_rate_; }
inline void Blip_Buffer::clock_rate(long cps) { factor_ = clock_rate_factor(clock_rate_ = cps); }

inline int Blip_Reader::begin(Blip_Buffer &blip_buf)
{
  buf = blip_buf.buffer_;
  accum = blip_buf.reader_accum;
  return blip_buf.bass_shift;
}

int const blip_max_length = 0;
int const blip_default_length = 250;

} // namespace quickerNES