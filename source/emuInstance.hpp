#pragma once

#include <core/Nes_Emu.h>
#include <core/Nes_State.h>
#include <string>
#include <utils.hpp> 

#define _LOW_MEM_SIZE 0x800
#define _HIGH_MEM_SIZE 0x2000

class EmuInstance
{
 public:

 typedef uint8_t inputType;

 // Deleting default constructors
 EmuInstance() = delete;
 EmuInstance(EmuInstance& e) = delete;
 ~EmuInstance() = default;

 EmuInstance(const std::string& romFilePath, const std::string& stateFilePath)
 {
  // Creating new emulator
  _nes = new Nes_Emu;

  // Loading ROM
  std::string romData;
  loadStringFromFile(romData, romFilePath.c_str());
  Mem_File_Reader romReader(romData.data(), (int)romData.size());
  Auto_File_Reader romFile(romReader);
  auto result = _nes->load_ines(romFile);
  if (result != 0) EXIT_WITH_ERROR("Could not initialize emulator with rom file: %s\n", romFilePath.c_str());

  // Setting base memory pointers
  _baseMem = _nes->low_mem();
  _ppuNameTableMem = _nes->nametable_mem();
  _highMem = _nes->high_mem();

  // Getting state size to use
  _stateSize = getStateSizeImpl();

  // Loading state file, if specified
  if (stateFilePath != "") loadStateFile(stateFilePath);
 }

 void loadStateFile(const std::string& stateFilePath) 
 {
  // Loading state data
  std::string stateData;
  if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());
  Mem_File_Reader stateReader(stateData.data(), (int)stateData.size());
  Auto_File_Reader stateFile(stateReader);

  // Loading state data into state object
  Nes_State state;
  state.read(stateFile);

  // Loading state object into the emulator
  _nes->load_state(state);
 }

 inline size_t getStateSize() const { return _stateSize; }

 void saveStateFile(const std::string& stateFilePath) const 
 {
  std::string stateData;
  stateData.resize(_stateSize);
  serializeState((uint8_t*)stateData.data());
  saveStringToFile(stateData, stateFilePath.c_str());
 }

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

 static inline inputType moveStringToCode(const std::string& move)
 {
  inputType moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'U': moveCode |= 0b00010000; break;
    case 'D': moveCode |= 0b00100000; break;
    case 'L': moveCode |= 0b01000000; break;
    case 'R': moveCode |= 0b10000000; break;
    case 'S': moveCode |= 0b00001000; break;
    case 's': moveCode |= 0b00000100; break;
    case 'B': moveCode |= 0b00000010; break;
    case 'A': moveCode |= 0b00000001; break;
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

  if (move & 0b00010000) moveString += 'U'; else moveString += '.';
  if (move & 0b00100000) moveString += 'D'; else moveString += '.';
  if (move & 0b01000000) moveString += 'L'; else moveString += '.';
  if (move & 0b10000000) moveString += 'R'; else moveString += '.';
  if (move & 0b00001000) moveString += 'S'; else moveString += '.';
  if (move & 0b00000100) moveString += 's'; else moveString += '.';
  if (move & 0b00000010) moveString += 'B'; else moveString += '.';
  if (move & 0b00000001) moveString += 'A'; else moveString += '.';

  moveString += "|";
  return moveString;
 }

 void advanceState(const std::string& move)
 {
  advanceState(moveStringToCode(move), 0);
 }

 void advanceState(const inputType controller1, const inputType controller2) 
 {
  _nes->emulate_frame(controller1, controller2);
 }

 Nes_Emu* getInternalEmulator() const { return _nes; }

 private:

 inline size_t getStateSizeImpl() const
 {
  // Using dry writer to just obtain the state size
  Dry_Writer w;
  Auto_File_Writer a(w);
  _nes->save_state(a);
  return w.size();
 }

 // Emulator instance
 Nes_Emu* _nes;

 // Base low-memory pointer
 uint8_t* _baseMem;
 uint8_t* _ppuNameTableMem;
 uint8_t* _highMem;

 // State size for the given rom
 size_t _stateSize;

};
