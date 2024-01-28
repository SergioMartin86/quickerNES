#pragma once

#include <quickerNES/core/emu.hpp>
#include <nesInstanceBase.hpp>

typedef quickerNES::Emu emulator_t;

class NESInstance final : public NESInstanceBase
{
  public:

  uint8_t *getLowMem() override { return _nes.low_mem(); };

  void serializeState(uint8_t *state) const override {  _nes.serializeState(state); }
  void deserializeState(const uint8_t *state) override { _nes.deserializeState(state); }
  size_t getStateSize() const override { return _nes.getStateSize(); }

  std::string getCoreName() const override { return "QuickerNES"; }
  
  void doSoftReset() override { _nes.reset(false); }
  void doHardReset() override { _nes.reset(true); }
  
  void *getInternalEmulatorPointer() override { return &_nes; }
  
  protected:

  bool loadROMImpl(const uint8_t* romData, const size_t romSize) override
  {
    // Loading rom data
    _nes.load_ines(romData);
    return true;
  }

  void enableStateBlockImpl(const std::string& block) override { _nes.enableStateBlock(block); };
  void disableStateBlockImpl(const std::string& block) override { _nes.disableStateBlock(block); };
  void advanceStateImpl(const Controller::port_t controller1, const Controller::port_t controller2) override
  {
    if (_doRendering == true) _nes.emulate_frame(controller1, controller2);
    if (_doRendering == false) _nes.emulate_skip_frame(controller1, controller2);
  }

  private:

  // Video buffer
  uint8_t *video_buffer;

  // Emulator instance
  emulator_t _nes;
};
