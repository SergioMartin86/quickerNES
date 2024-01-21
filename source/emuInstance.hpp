#pragma once

#include "sha1/sha1.hpp"
#include <utils.hpp>
#include <controller.hpp>

#define _LOW_MEM_SIZE 0x800
#define _HIGH_MEM_SIZE 0x2000
#define _NAMETABLES_MEM_SIZE 0x1000

// Size of image generated in graphics buffer
static const uint16_t image_width = 256;
static const uint16_t image_height = 240;

class EmuInstance
{
  public:

  EmuInstance() = default;
  virtual ~EmuInstance() = default;

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

  inline std::string getRomSHA1() const { return _romSHA1String; }

  inline hash_t getStateHash() const
  {
    MetroHash128 hash;

    hash.Update(getLowMem(), _LOW_MEM_SIZE);
    // hash.Update(getHighMem(), _HIGH_MEM_SIZE);
    // hash.Update(getNametableMem(), _NAMETABLES_MEM_SIZE);
    // hash.Update(getChrMem(), getChrMemSize());

    hash_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  inline void enableRendering() { _doRendering = true; };
  inline void disableRendering() { _doRendering = false; };

  inline void loadStateFile(const std::string &stateFilePath)
  {
    std::string stateData;
    bool status = loadStringFromFile(stateData, stateFilePath);
    if (status == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());
    deserializeFullState((uint8_t *)stateData.data());
  }

  inline void saveStateFile(const std::string &stateFilePath) const
  {
    std::string stateData;
    stateData.resize(_fullStateSize);
    serializeFullState((uint8_t *)stateData.data());
    saveStringToFile(stateData, stateFilePath.c_str());
  }

  inline void loadROMFile(const std::string &romFilePath)
  {
    // Loading ROM data
    bool status = loadStringFromFile(_romData, romFilePath);
    if (status == false) EXIT_WITH_ERROR("Could not find/read ROM file: %s\n", romFilePath.c_str());

    // Calculating ROM hash value
    _romSHA1String = SHA1::GetHash((uint8_t *)_romData.data(), _romData.size());

    // Actually loading rom file
    status = loadROMFileImpl(_romData);
    if (status == false) EXIT_WITH_ERROR("Could not process ROM file: %s\n", romFilePath.c_str());

    // Detecting full state size
    _fullStateSize = getFullStateSize();

    // Detecting lite state size
    _liteStateSize = getLiteStateSize();
  }

  // Virtual functions

  virtual bool loadROMFileImpl(const std::string &romFilePath) = 0;
  virtual void advanceStateImpl(const Controller::port_t controller1, const Controller::port_t controller2) = 0;
  virtual uint8_t *getLowMem() const = 0;
  virtual uint8_t *getNametableMem() const = 0;
  virtual uint8_t *getHighMem() const = 0;
  virtual const uint8_t *getChrMem() const = 0;
  virtual size_t getChrMemSize() const = 0;
  virtual void serializeFullState(uint8_t *state) const = 0;
  virtual void deserializeFullState(const uint8_t *state) = 0;
  virtual void serializeLiteState(uint8_t *state) const = 0;
  virtual void deserializeLiteState(const uint8_t *state) = 0;
  virtual size_t getFullStateSize() const = 0;
  virtual size_t getLiteStateSize() const = 0;
  virtual void enableLiteStateBlock(const std::string& block) = 0;
  virtual void disableLiteStateBlock(const std::string& block) = 0;

  virtual void doSoftReset() = 0;
  virtual void doHardReset() = 0;
  virtual std::string getCoreName() const = 0;
  virtual void *getInternalEmulatorPointer() const = 0;

  protected:

  // Storage for the light state size
  size_t _liteStateSize;

  // Storage for the full state size
  size_t _fullStateSize;

  // Flag to determine whether to enable/disable rendering
  bool _doRendering = true;

  private:
  // Storage for the ROM data
  std::string _romData;

  // SHA1 rom hash
  std::string _romSHA1String;

  // Controller class for input parsing
  Controller _controller;
};
