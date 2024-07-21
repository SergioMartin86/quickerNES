#pragma once

#include "jaffarCommon/serializers/contiguous.hpp"
#include "jaffarCommon/serializers/differential.hpp"
#include "jaffarCommon/logger.hpp"
#include "controller.hpp"

// Size of image generated in graphics buffer
static const uint16_t image_width = 256;
static const uint16_t image_height = 240;

class NESInstanceBase
{
  public:

  NESInstanceBase() = default;
  virtual ~NESInstanceBase() = default;

  inline void advanceState(const std::string &input)
  {
    // Storage for the decoded input
    quickNES::Controller::input_t decodedInput;

    // Getting decoded input from the input string
    decodeInput(input, &decodedInput);

    // Calling advance state with the decoded input
    advanceState(&decodedInput);
  }

  inline void advanceState(const void* decodedInputBuffer)
  {
    // Casting decoded input to the right type
    const auto decodedInput = (quickNES::Controller::input_t*) decodedInputBuffer;

    // Parsing power
    if (decodedInput->power == true) JAFFAR_THROW_LOGIC("Power button pressed, but not supported.");

    // Parsing reset
    if (decodedInput->reset == true) doSoftReset();

    // Running specified inputs
    advanceStateImpl(decodedInput->port1, decodedInput->port2);
  }

  inline size_t getDecodedInputSize() const
  {
    return sizeof(quickNES::Controller::input_t);
  }

  inline void decodeInput(const std::string &input, void* decodedInputBuffer) const
  {
    const auto decodedInput = (quickNES::Controller::input_t*) decodedInputBuffer;
    bool isInputValid = _controller.parseInputString(input, decodedInput);
    if (isInputValid == false) JAFFAR_THROW_LOGIC("Move provided cannot be parsed: '%s'\n", input.c_str());
  }

  inline void setController1Type(const std::string& type)
  {
    bool isTypeRecognized = false;

    if (type == "None") { _controller.setController1Type(quickNES::Controller::controller_t::none); isTypeRecognized = true; }
    if (type == "Joypad") { _controller.setController1Type(quickNES::Controller::controller_t::joypad); isTypeRecognized = true; }
    if (type == "FourScore1") { _controller.setController1Type(quickNES::Controller::controller_t::fourscore1); isTypeRecognized = true; }
    if (type == "FourScore2") { _controller.setController1Type(quickNES::Controller::controller_t::fourscore2); isTypeRecognized = true; }

    if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Input type not recognized: '%s'\n", type.c_str());
  }

  inline void setController2Type(const std::string& type)
  {
    bool isTypeRecognized = false;

    if (type == "None") { _controller.setController2Type(quickNES::Controller::controller_t::none); isTypeRecognized = true; }
    if (type == "Joypad") { _controller.setController2Type(quickNES::Controller::controller_t::joypad); isTypeRecognized = true; }
    if (type == "FourScore1") { _controller.setController2Type(quickNES::Controller::controller_t::fourscore1); isTypeRecognized = true; }
    if (type == "FourScore2") { _controller.setController2Type(quickNES::Controller::controller_t::fourscore2); isTypeRecognized = true; }
    
    if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Input type not recognized: '%s'\n", type.c_str());
  }

  inline void enableRendering() { _doRendering = true; };
  inline void disableRendering() { _doRendering = false; };

  inline bool loadROM(const uint8_t* romData, const size_t romSize)
  {
    // Actually loading rom file
    auto status = loadROMImpl(romData, romSize);

    // Detecting full state size
    _stateSize = getFullStateSize();

    // Returning status
    return status;
  }

  void enableStateBlock(const std::string& block)
  {
    // Calling implementation
    enableStateBlockImpl(block);

    // Recalculating State size
    _stateSize = getFullStateSize();
  }

  void disableStateBlock(const std::string& block)
  {
    // Calling implementation
    disableStateBlockImpl(block);

    // Recalculating State Size
    _stateSize = getFullStateSize();
  }

  virtual size_t getFullStateSize() const = 0;
  virtual size_t getDifferentialStateSize() const = 0;

  // Virtual functions

  virtual uint8_t *getLowMem() const = 0;
  virtual size_t getLowMemSize() const = 0;

  virtual void serializeState(jaffarCommon::serializer::Base& serializer) const = 0;
  virtual void deserializeState(jaffarCommon::deserializer::Base& deserializer) = 0;

  virtual void doSoftReset() = 0;
  virtual void doHardReset() = 0;
  virtual std::string getCoreName() const = 0;
  virtual void *getInternalEmulatorPointer() = 0;
  virtual void setNTABBlockSize(const size_t size) { };

  protected:

  virtual void enableStateBlockImpl(const std::string& block) = 0;
  virtual void disableStateBlockImpl(const std::string& block) = 0;
  virtual bool loadROMImpl(const uint8_t* romData, const size_t romSize) = 0;
  virtual void advanceStateImpl(const quickNES::Controller::port_t controller1, const quickNES::Controller::port_t controller2) = 0;

  // Storage for the light state size
  size_t _stateSize;

  // Flag to determine whether to enable/disable rendering
  bool _doRendering = true;

  private:

  // Controller class for input parsing
  quickNES::Controller _controller;
};
