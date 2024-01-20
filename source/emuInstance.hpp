#pragma once

#include "sha1/sha1.hpp"
#include <utils.hpp>

#define _LOW_MEM_SIZE 0x800
#define _HIGH_MEM_SIZE 0x2000
#define _NAMETABLES_MEM_SIZE 0x1000

// Size of image generated in graphics buffer
static const uint16_t image_width = 256;
static const uint16_t image_height = 240;

class EmuInstance
{
  public:
  typedef uint8_t inputType;

  // Deleting default constructors
  virtual ~EmuInstance() = default;

  // Controller input bits
  // 0 - A / 1
  // 1 - B / 2
  // 2 - Select / 4
  // 3 - Start / 8
  // 4 - Up / 16
  // 5 - Down / 32
  // 6 - Left / 64
  // 7 - Right / 128

  // Move Format:
  // RLDUTSBA
  // ........

  static inline inputType moveStringToCode(const std::string &move)
  {
    inputType moveCode = 0;

    for (size_t i = 0; i < move.size(); i++) switch (move[i])
      {
      case 'U': moveCode |= 0b00010000; break;
      case 'D': moveCode |= 0b00100000; break;
      case 'L': moveCode |= 0b01000000; break;
      case 'R': moveCode |= 0b10000000; break;
      case 'S': moveCode |= 0b00001000; break;
      case 's': moveCode |= 0b00000100; break;
      case 'B': moveCode |= 0b00000010; break;
      case 'A': moveCode |= 0b00000001; break;
      case 'r': break;
      case '.': break;
      case '|': break;
      default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
      }

    return moveCode;
  }

  static inline std::string moveCodeToString(const inputType move)
  {
#ifndef _NES_PLAYER_2
    std::string moveString = "|..|";
#else
    std::string moveString = "|..|........|";
#endif

    if (move & 0b00010000) moveString += 'U'; else  moveString += '.';
    if (move & 0b00100000) moveString += 'D'; else  moveString += '.';
    if (move & 0b01000000) moveString += 'L'; else  moveString += '.';
    if (move & 0b10000000) moveString += 'R'; else  moveString += '.';
    if (move & 0b00001000) moveString += 'S'; else  moveString += '.';
    if (move & 0b00000100) moveString += 's'; else  moveString += '.';
    if (move & 0b00000010) moveString += 'B'; else  moveString += '.';
    if (move & 0b00000001) moveString += 'A'; else  moveString += '.';

    moveString += "|";
    return moveString;
  }

  inline void advanceState(const std::string &move)
  {
    if (move.find("r") != std::string::npos) doSoftReset();

    advanceStateImpl(moveStringToCode(move), 0);
  }

  inline std::string getRomSHA1() const { return _romSHA1String; }

  inline hash_t getStateHash() const
  {
    MetroHash128 hash;

    hash.Update(getLowMem(), _LOW_MEM_SIZE);
    hash.Update(getHighMem(), _HIGH_MEM_SIZE);
    hash.Update(getNametableMem(), _NAMETABLES_MEM_SIZE);
    hash.Update(getChrMem(), getChrMemSize());

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
  virtual void advanceStateImpl(const inputType controller1, const inputType controller2) = 0;
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
  virtual void doSoftReset() = 0;
  virtual void doHardReset() = 0;
  virtual std::string getCoreName() const = 0;
  virtual void *getInternalEmulatorPointer() const = 0;

  protected:
  EmuInstance() = default;

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
};
