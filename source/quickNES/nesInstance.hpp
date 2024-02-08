#pragma once

#include "core/nes_emu/Nes_Emu.h"
#include "core/nes_emu/Nes_State.h"
#include "../nesInstanceBase.hpp"

#define _DUMMY_SIZE 65536

typedef Nes_Emu emulator_t;

extern void register_misc_mappers();
extern void register_extra_mappers();
extern void register_mapper_70();

class NESInstance final : public NESInstanceBase
{
  public:
  NESInstance() : NESInstanceBase()
  {
    // If running the original QuickNES, register extra mappers now
    register_misc_mappers();
    register_extra_mappers();
    register_mapper_70();
  }

  uint8_t *getLowMem() const override { return _nes.low_mem(); };
  size_t getLowMemSize() const override { return 0x800; };

  void serializeState(uint8_t *state) const override
  {
    Mem_Writer w(state, _stateSize, 0);
    Auto_File_Writer a(w);
    _nes.save_state(a);
  }

  void deserializeState(const uint8_t *state) override
  {
    Mem_File_Reader r(state, _stateSize);
    Auto_File_Reader a(r);
    _nes.load_state(a);
  }

  inline size_t getStateSize() const override
  {
    uint8_t *data = (uint8_t *)malloc(_DUMMY_SIZE);
    Mem_Writer w(data, _DUMMY_SIZE);
    Auto_File_Writer a(w);
    _nes.save_state(a);
    free(data);
    return w.size();
  }

  void serializeDifferentialState(uint8_t *differentialData, size_t* differentialDataPos, const uint8_t* referenceData, size_t* referenceDataPos, const size_t maxSize, const bool useZlib) const override
  {  serializeState(differentialData); }

  void deserializeDifferentialState(const uint8_t *differentialData, size_t* differentialDataPos, const uint8_t* referenceData, size_t* referenceDataPos, const bool useZlib) override 
  {  deserializeState(differentialData); }

  size_t getDifferentialStateSize() const override
  { return getStateSize(); }

  std::string getCoreName() const override { return "QuickNES"; }
  void doSoftReset() override { _nes.reset(false); }
  void doHardReset() override { _nes.reset(true); }

  void *getInternalEmulatorPointer() override { return &_nes; }

  protected:

  bool loadROMImpl(const uint8_t* romData, const size_t romSize) override
  {
    // Loading rom data
    Mem_File_Reader romReader(romData, (int)romSize);
    Auto_File_Reader romFile(romReader);
    auto result = _nes.load_ines(romFile);
    return result == 0;
  }

  void enableStateBlockImpl(const std::string& block) override {};
  void disableStateBlockImpl(const std::string& block) override {};

  void advanceStateImpl(const Controller::port_t controller1, const Controller::port_t controller2) override
  {
    if (_doRendering == true) _nes.emulate_frame(controller1, controller2);
    if (_doRendering == false) _nes.emulate_skip_frame(controller1, controller2);
  }

  private:

  // Emulator instance
  emulator_t _nes;
};
