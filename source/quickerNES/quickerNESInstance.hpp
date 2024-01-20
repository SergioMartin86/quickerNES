#pragma once

#include <quickerNES/emu.hpp>
#include <emuInstance.hpp>

namespace quickerNES
{

class QuickerNESInstance : public EmuInstance
{
  public:
  QuickerNESInstance() : EmuInstance()
  {
    // Creating new emulator
    _nes = new Emu;

    // Allocating video buffer
    video_buffer = (uint8_t *)malloc(image_width * image_height);

    // Setting video buffer
    _nes->set_pixels(video_buffer, image_width + 8);
  }

  ~QuickerNESInstance() = default;

  virtual bool loadROMFileImpl(const std::string &romData) override
  {
    // Loading rom data
    _nes->load_ines((uint8_t *)romData.data());
    return true;
  }

  uint8_t *getLowMem() const override { return _nes->low_mem(); };
  uint8_t *getNametableMem() const override { return _nes->nametable_mem(); };
  uint8_t *getHighMem() const override { return _nes->high_mem(); };
  const uint8_t *getChrMem() const override { return _nes->chr_mem(); };
  size_t getChrMemSize() const override { return _nes->chr_size(); };

  void serializeFullState(uint8_t *state) const override {  _nes->serializeFullState(state); }
  void deserializeFullState(const uint8_t *state) override { _nes->deserializeFullState(state); }

  void serializeLiteState(uint8_t *state) const override {  _nes->serializeLiteState(state); }
  void deserializeLiteState(const uint8_t *state) override { _nes->deserializeLiteState(state); }

  size_t getFullStateSize() const override { return _nes->getFullStateSize(); }
  size_t getLiteStateSize() const override { return _nes->getLiteStateSize(); }

  void advanceStateImpl(const inputType controller1, const inputType controller2) override
  {
    if (_doRendering == true) _nes->emulate_frame(controller1, controller2);
    if (_doRendering == false) _nes->emulate_skip_frame(controller1, controller2);
  }

  std::string getCoreName() const override { return "QuickerNES"; }
  void doSoftReset() override { _nes->reset(false); }
  void doHardReset() override { _nes->reset(true); }

  void *getInternalEmulatorPointer() const override { return _nes; }

  // Video buffer
  uint8_t *video_buffer;

  // Emulator instance
  Emu *_nes;
};

} // namespace quickNES