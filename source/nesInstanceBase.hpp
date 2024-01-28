#pragma once

#include "logger.hpp"
#include "controller.hpp"

#define _LOW_MEM_SIZE 0x800
#define _HIGH_MEM_SIZE 0x2000
#define _NAMETABLES_MEM_SIZE 0x1000

// Size of image generated in graphics buffer
static const uint16_t image_width = 256;
static const uint16_t image_height = 240;

class NESInstanceBase
{
  public:

  NESInstanceBase() = default;
  virtual ~NESInstanceBase() = default;

  inline void advanceState(const std::string &move)
  {
    bool isInputValid = _controller.parseInputString(move);
    if (isInputValid == false) EXIT_WITH_ERROR("Move provided cannot be parsed: '%s'\n", move.c_str());

    // Parsing power
    if (_controller.getPowerButtonState() == true) EXIT_WITH_ERROR("Power button pressed, but not supported: '%s'\n", move.c_str());

    // Parsing reset
    if (_controller.getResetButtonState() == true) doSoftReset();

    // Parsing Controllers
    const auto controller1 = _controller.getController1Code();
    const auto controller2 = _controller.getController2Code();

    advanceStateImpl(controller1, controller2);
  }

  inline void setController1Type(const std::string& type)
  {
    bool isTypeRecognized = false;

    if (type == "None") { _controller.setController1Type(Controller::controller_t::none); isTypeRecognized = true; }
    if (type == "Joypad") { _controller.setController1Type(Controller::controller_t::joypad); isTypeRecognized = true; }
    if (type == "FourScore1") { _controller.setController1Type(Controller::controller_t::fourscore1); isTypeRecognized = true; }
    if (type == "FourScore2") { _controller.setController1Type(Controller::controller_t::fourscore2); isTypeRecognized = true; }

    if (isTypeRecognized == false) EXIT_WITH_ERROR("Input type not recognized: '%s'\n", type.c_str());
  }

  inline void setController2Type(const std::string& type)
  {
    bool isTypeRecognized = false;

    if (type == "None") { _controller.setController2Type(Controller::controller_t::none); isTypeRecognized = true; }
    if (type == "Joypad") { _controller.setController2Type(Controller::controller_t::joypad); isTypeRecognized = true; }
    if (type == "FourScore1") { _controller.setController2Type(Controller::controller_t::fourscore1); isTypeRecognized = true; }
    if (type == "FourScore2") { _controller.setController2Type(Controller::controller_t::fourscore2); isTypeRecognized = true; }
    
    if (isTypeRecognized == false) EXIT_WITH_ERROR("Input type not recognized: '%s'\n", type.c_str());
  }

  inline void enableRendering() { _doRendering = true; };
  inline void disableRendering() { _doRendering = false; };

  inline bool loadROM(const uint8_t* romData, const size_t romSize)
  {
    // Actually loading rom file
    auto status = loadROMImpl(romData, romSize);

    // Detecting full state size
    _stateSize = getStateSize();

    // Returning status
    return status;
  }

  void enableStateBlock(const std::string& block)
  {
    // Calling implementation
    enableStateBlockImpl(block);

    // Recalculating State size
    _stateSize = getStateSize();
  }

  void disableStateBlock(const std::string& block)
  {
    // Calling implementation
    disableStateBlockImpl(block);

    // Recalculating State Size
    _stateSize = getStateSize();
  }

  // Virtual functions

  virtual uint8_t *getLowMem() = 0;

  virtual void serializeState(uint8_t *state) const = 0;
  virtual void deserializeState(const uint8_t *state) = 0;
  virtual size_t getStateSize() const = 0;
  
  virtual void doSoftReset() = 0;
  virtual void doHardReset() = 0;
  virtual std::string getCoreName() const = 0;
  virtual void *getInternalEmulatorPointer() = 0;

  protected:

  virtual void enableStateBlockImpl(const std::string& block) = 0;
  virtual void disableStateBlockImpl(const std::string& block) = 0;
  virtual bool loadROMImpl(const uint8_t* romData, const size_t romSize) = 0;
  virtual void advanceStateImpl(const Controller::port_t controller1, const Controller::port_t controller2) = 0;

  // Storage for the light state size
  size_t _stateSize;

  // Flag to determine whether to enable/disable rendering
  bool _doRendering = true;

  private:

  // Controller class for input parsing
  Controller _controller;
};
