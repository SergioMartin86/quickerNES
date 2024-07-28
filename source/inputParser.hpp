#pragma once

// Base controller class
// by eien86

#include <cstdint>
#include <string>
#include <sstream>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffar
{

typedef uint32_t port_t;

struct input_t
{
  bool power = false;
  bool reset = false;
  port_t port1 = 0;
  port_t port2 = 0;
};


class InputParser
{
public:

  enum controller_t { none, joypad, fourscore1, fourscore2 };

  InputParser(const nlohmann::json &config)
  {
    // Parsing controller 1 type
    {
      bool isTypeRecognized = false;
      const auto controller1Type = jaffarCommon::json::getString(config, "Controller 1 Type");
      if (controller1Type == "None") { _controller1Type = controller_t::none; isTypeRecognized = true; }
      if (controller1Type == "Joypad") { _controller1Type = controller_t::joypad; isTypeRecognized = true; }
      if (controller1Type == "FourScore1") { _controller1Type = controller_t::fourscore1; isTypeRecognized = true; }
      if (controller1Type == "FourScore2") { _controller1Type = controller_t::fourscore2; isTypeRecognized = true; }
      if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Controller 1 type not recognized: '%s'\n", controller1Type.c_str());
    }

    // Parsing controller 2 type
    {
      bool isTypeRecognized = false;
      const auto controller2Type = jaffarCommon::json::getString(config, "Controller 2 Type");
      if (controller2Type == "None") { _controller2Type = controller_t::none; isTypeRecognized = true; }
      if (controller2Type == "Joypad") { _controller2Type = controller_t::joypad; isTypeRecognized = true; }
      if (controller2Type == "FourScore1") { _controller2Type = controller_t::fourscore1; isTypeRecognized = true; }
      if (controller2Type == "FourScore2") { _controller2Type = controller_t::fourscore2; isTypeRecognized = true; }
      if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Controller 2 type not recognized: '%s'\n", controller2Type.c_str());
    }

  }

  inline input_t parseInputString(const std::string& inputString) const
  {
    // Storage for the input
    input_t input;

    // Converting input into a stream for parsing
    std::istringstream ss(inputString);

    // Start separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // Parsing console inputs
    parseConsoleInputs(input.reset, input.power, ss, inputString);

    // Parsing controller 1 inputs
    parseControllerInputs(_controller1Type, input.port1, ss, inputString);

    // Parsing controller 1 inputs
    parseControllerInputs(_controller2Type, input.port2, ss, inputString);

    // End separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // If its not the end of the stream, then extra values remain and its invalid
    ss.get();
    if (ss.eof() == false) reportBadInputString(inputString);

    return input;
  }

  private:

  static inline void reportBadInputString(const std::string& inputString)
  {
    JAFFAR_THROW_LOGIC("Could not decode input string: '%s'\n", inputString.c_str());
  }

  static void parseJoyPadInput(uint8_t& code, std::istringstream& ss, const std::string& inputString)
  {
    // Currently read character
    char c;

    // Cleaning code
    code = 0;

    // Up
    c = ss.get();
    if (c != '.' && c != 'U') reportBadInputString(inputString);
    if (c == 'U') code |= 0b00010000;

    // Down
    c = ss.get();
    if (c != '.' && c != 'D') reportBadInputString(inputString);
    if (c == 'D') code |= 0b00100000;

    // Left
    c = ss.get();
    if (c != '.' && c != 'L') reportBadInputString(inputString);
    if (c == 'L') code |= 0b01000000;

    // Right
    c = ss.get();
    if (c != '.' && c != 'R') reportBadInputString(inputString);
    if (c == 'R') code |= 0b10000000;

    // Start
    c = ss.get();
    if (c != '.' && c != 'S') reportBadInputString(inputString);
    if (c == 'S') code |= 0b00001000;

    // Select
    c = ss.get();
    if (c != '.' && c != 's') reportBadInputString(inputString);
    if (c == 's') code |= 0b00000100;

    // B
    c = ss.get();
    if (c != '.' && c != 'B') reportBadInputString(inputString);
    if (c == 'B') code |= 0b00000010;

    // A
    c = ss.get();
    if (c != '.' && c != 'A') reportBadInputString(inputString);
    if (c == 'A') code |= 0b00000001;
  }

  static void parseControllerInputs(const controller_t type, port_t& port, std::istringstream& ss, const std::string& inputString)
  {
    // If no controller assigned then, its port is all zeroes.
    if (type == controller_t::none) { port = 0; return; }

    // Controller separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // If normal joypad, parse its code now
    if (type == controller_t::joypad) 
    {
      // Storage for joypad's code
      uint8_t code = 0;

      // Parsing joypad code
      parseJoyPadInput(code, ss, inputString);

      // Pushing input code into the port
      port = code;

      // Adding joypad signature
      // Per https://www.nesdev.org/wiki/Standard_controller, the joypad reports 1s after the first 8 bits
      port |= ~0xFF;
    }

    // If its fourscore, its like two joypads separated by a |
    if (type == controller_t::fourscore1 || type == controller_t::fourscore2) 
    {
      // Storage for joypad's code
      uint8_t code1 = 0;
      uint8_t code2 = 0;

      // Parsing joypad code1
      parseJoyPadInput(code1, ss, inputString);

      // Separator
      if (ss.get() != '|') reportBadInputString(inputString);

      // Parsing joypad code1
      parseJoyPadInput(code2, ss, inputString);

      // Creating code
      port = code1;
      port |= (uint32_t)0 | code2 << 8;
      if (type == controller_t::fourscore1) port |= (uint32_t)0 | 1 << 19;
      if (type == controller_t::fourscore2) port |= (uint32_t)0 | 1 << 18;
      port |= (uint32_t)0 | 1 << 24;
      port |= (uint32_t)0 | 1 << 25;
      port |= (uint32_t)0 | 1 << 26;
      port |= (uint32_t)0 | 1 << 27;
      port |= (uint32_t)0 | 1 << 28;
      port |= (uint32_t)0 | 1 << 29;
      port |= (uint32_t)0 | 1 << 30;
      port |= (uint32_t)0 | 1 << 31;
    }
  }

  static void parseConsoleInputs(bool& reset, bool& power, std::istringstream& ss, const std::string& inputString)
  {
    // Currently read character
    char c;

    // Power trigger
    c = ss.get();
    if (c != '.' && c != 'P') reportBadInputString(inputString);
    if (c == 'P') power = true;
    if (c == '.') power = false;

    // Reset trigger
    c = ss.get();
    if (c != '.' && c != 'r') reportBadInputString(inputString);
    if (c == 'r') reset = true;
    if (c == '.') reset = false;
  }

  controller_t _controller1Type;
  controller_t _controller2Type;
  
}; // class InputParser

} // namespace jaffar