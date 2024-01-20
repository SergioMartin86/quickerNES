#pragma once

#include <Nes_Emu.h>
#include <Nes_State.h>
#include <emuInstance.hpp>

#define _DUMMY_SIZE 65536

extern void register_misc_mappers();
extern void register_extra_mappers();
extern void register_mapper_70();

class QuickNESInstance : public EmuInstance
{
  public:
  QuickNESInstance() : EmuInstance()
  {
    // Creating new emulator
    _nes = new Nes_Emu;

    // Allocating video buffer
    video_buffer = (uint8_t *)malloc(image_width * image_height);

    // Setting video buffer
    _nes->set_pixels(video_buffer, image_width + 8);

    // If running the original QuickNES, register extra mappers now
    register_misc_mappers();
    register_extra_mappers();
    register_mapper_70();
  }

  virtual bool loadROMFileImpl(const std::string &romData) override
  {
    // Loading rom data
    Mem_File_Reader romReader(romData.data(), (int)romData.size());
    Auto_File_Reader romFile(romReader);
    auto result = _nes->load_ines(romFile);
    return result == 0;
  }

  uint8_t *getLowMem() const override { return _nes->low_mem(); };
  uint8_t *getNametableMem() const override { return _nes->nametable_mem(); };
  uint8_t *getHighMem() const override { return _nes->high_mem(); };
  const uint8_t *getChrMem() const override { return _nes->chr_mem(); };
  size_t getChrMemSize() const override { return _nes->chr_size(); };

  void serializeState(uint8_t *state) const override
  {
    Mem_Writer w(state, _stateSize, 0);
    Auto_File_Writer a(w);
    _nes->save_state(a);
  }

  void deserializeState(const uint8_t *state) override
  {
    Mem_File_Reader r(state, _stateSize);
    Auto_File_Reader a(r);
    _nes->load_state(a);
  }

  void advanceStateImpl(const inputType controller1, const inputType controller2) override
  {
    if (_doRendering == true) _nes->emulate_frame(controller1, controller2);
    if (_doRendering == false) _nes->emulate_skip_frame(controller1, controller2);
  }

  std::string getCoreName() const override { return "QuickNES"; }
  void doSoftReset() override { _nes->reset(false); }
  void doHardReset() override { _nes->reset(true); }

  void *getInternalEmulatorPointer() const override { return _nes; }

  private:
  inline size_t getStateSizeImpl() const override
  {
    uint8_t *data = (uint8_t *)malloc(_DUMMY_SIZE);
    Mem_Writer w(data, _DUMMY_SIZE);
    Auto_File_Writer a(w);
    _nes->save_state(a);
    free(data);
    return w.size();
  }

  // Video buffer
  uint8_t *video_buffer;

  // Emulator instance
  Nes_Emu *_nes;
};
