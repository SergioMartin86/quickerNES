#pragma once

#include <Nes_Emu.h>
#include <Nes_State.h>
#include <emuInstance.hpp>

class QuickerNESInstance : public EmuInstance
{
 public:

 QuickerNESInstance() : EmuInstance()
 {
  // Creating new emulator
  _nes = new Nes_Emu;

  // Setting video buffer
  _nes->set_pixels(video_buffer, image_width+8);
 }
  
 ~QuickerNESInstance() = default;

 virtual bool loadROMFileImpl(const std::string& romData) override
 {
  // Loading rom data
  auto result = _nes->load_ines((uint8_t*)romData.data());
  return result == 0;
 }

 uint8_t* getLowMem() const override { return _nes->low_mem(); };
 uint8_t* getNametableMem() const override { return _nes->nametable_mem(); };
 uint8_t* getHighMem() const override { return _nes->high_mem();};
 const uint8_t* getChrMem() const override { return _nes->chr_mem();};
 size_t getChrMemSize() const override { return _nes->chr_size();};

 void serializeState(uint8_t* state) const 
 {
  Mem_Writer w(state, _stateSize, 0);
  Auto_File_Writer a(w);
  _nes->save_state(a);
 }

 void deserializeState(const uint8_t* state) 
 {
  Mem_File_Reader r(state, _stateSize);
  Auto_File_Reader a(r);
  _nes->load_state(a);
 }

 void advanceStateImpl(const inputType controller1, const inputType controller2) override
 {
  if (_doRendering == true)  _nes->emulate_frame(controller1, controller2);
  if (_doRendering == false) _nes->emulate_skip_frame(controller1, controller2);
 }

 std::string getCoreName() const override { return "QuickerNES"; }
 void doSoftReset() override { _nes->reset(false); }
 void doHardReset() override { _nes->reset(true); }

 void* getInternalEmulatorPointer() const override { return _nes; }

 private:

 inline size_t getStateSizeImpl() const
 {
    // Using dry writer to just obtain the state size
    Dry_Writer w;
    Auto_File_Writer a(w);
    _nes->save_state(a);
    return w.size();
 }

 // Video buffer
 uint8_t video_buffer[image_width * image_height];

 // Emulator instance
 Nes_Emu* _nes;
};
