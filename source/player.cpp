#include <cstdlib>
#include "argparse/argparse.hpp"
#include "jaffarCommon/serializers/contiguous.hpp"
#include "jaffarCommon/deserializers/contiguous.hpp"
#include "jaffarCommon/file.hpp"
#include "jaffarCommon/logger.hpp"
#include "jaffarCommon/string.hpp"
#include "nesInstance.hpp"
#include "playbackInstance.hpp"

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("player", "1.0");

  program.add_argument("romFile")
    .help("Path to the rom file to run.")
    .required();

  program.add_argument("sequenceFile")
    .help("Path to the input sequence file (.sol) to reproduce.")
    .required();

  program.add_argument("stateFile")
    .help("(Optional) Path to the initial state file to load.")
    .default_value(std::string(""));

  program.add_argument("--reproduce")
    .help("Plays the entire sequence without interruptions and exit at the end.")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("--disableRender")
    .help("Do not render game window.")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("--controller1")
    .help("Specifies the controller 1 type.")
    .default_value(std::string("Joypad"));

  program.add_argument("--controller2")
    .help("Specifies the controller 2 type.")
    .default_value(std::string("None"));

  // Try to parse arguments
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
  }

  // Getting ROM file path
  std::string romFilePath = program.get<std::string>("romFile");

  // Getting sequence file path
  std::string sequenceFilePath = program.get<std::string>("sequenceFile");

  // If initial state file is specified, load it
  std::string stateFilePath = program.get<std::string>("stateFile");

  // Getting reproduce flag
  bool isReproduce = program.get<bool>("--reproduce");

  // Getting reproduce flag
  bool disableRender = program.get<bool>("--disableRender");

  // Getting controller 1 Type
  std::string controller1Type = program.get<std::string>("--controller1");

  // Getting controller 2 Type
  std::string controller2Type = program.get<std::string>("--controller2");

  // Loading sequence file
  std::string inputSequence;
  auto status = jaffarCommon::file::loadStringFromFile(inputSequence, sequenceFilePath.c_str());
  if (status == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from sequence file: %s\n", sequenceFilePath.c_str());

  // Building sequence information
  const auto sequence = jaffarCommon::string::split(inputSequence, ' ');

  // Initializing terminal
  jaffarCommon::logger::initializeTerminal();

  // Printing provided parameters
  printw("[] Rom File Path:      '%s'\n", romFilePath.c_str());
  printw("[] Sequence File Path: '%s'\n", sequenceFilePath.c_str());
  printw("[] Sequence Length:    %lu\n", sequence.size());
  printw("[] State File Path:    '%s'\n", stateFilePath.empty() ? "<Boot Start>" : stateFilePath.c_str());
  printw("[] Generating Sequence...\n");

  jaffarCommon::logger::refreshTerminal();

  // Creating emulator instance
  NESInstance e;

  // Setting controller types
  e.setController1Type(controller1Type);
  e.setController2Type(controller2Type);
  
  // Loading ROM File
  std::string romFileData;
  if (jaffarCommon::file::loadStringFromFile(romFileData, romFilePath) == false) JAFFAR_THROW_LOGIC("Could not rom file: %s\n", romFilePath.c_str());
  e.loadROM((uint8_t*)romFileData.data(), romFileData.size());

  // If an initial state is provided, load it now
  if (stateFilePath != "")
  {
    std::string stateFileData;
    if (jaffarCommon::file::loadStringFromFile(stateFileData, stateFilePath) == false) JAFFAR_THROW_LOGIC("Could not initial state file: %s\n", stateFilePath.c_str());
    jaffarCommon::deserializer::Contiguous deserializer(stateFileData.data());
    e.deserializeState(deserializer);
  }

  // Creating playback instance
  auto p = PlaybackInstance(&e, sequence);

  // Getting state size
  auto stateSize = e.getFullStateSize();

  // Flag to continue running playback
  bool continueRunning = true;

  // Variable for current step in view
  ssize_t sequenceLength = p.getSequenceLength();
  ssize_t currentStep = 0;

  // Flag to display frame information
  bool showFrameInfo = true;

  // Interactive section
  while (continueRunning)
  {
    // Updating display
    if (disableRender == false) p.renderFrame(currentStep);

    // Getting input
    const auto &input = p.getStateInput(currentStep);

    // Getting state hash
    const auto hash = p.getStateHash(currentStep);

    // Getting state data
    const auto stateData = p.getStateData(currentStep);

    // Printing data and commands
    if (showFrameInfo)
    {
      jaffarCommon::logger::clearTerminal();

      printw("[] ----------------------------------------------------------------\n");
      printw("[] Current Step #: %lu / %lu\n", currentStep + 1, sequenceLength);
      printw("[] Input:          %s\n", input.c_str());
      printw("[] State Hash:     0x%lX%lX\n", hash.first, hash.second);

      // Only print commands if not in reproduce mode
      if (isReproduce == false) printw("[] Commands: n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | k: -1000 | i: +1000 | s: quicksave | p: play | q: quit\n");

      jaffarCommon::logger::refreshTerminal();
    }

    // Resetting show frame info flag
    showFrameInfo = true;

    // Get command
    auto command = jaffarCommon::logger::waitForKeyPress();

    // Advance/Rewind commands
    if (command == 'n') currentStep = currentStep - 1;
    if (command == 'm') currentStep = currentStep + 1;
    if (command == 'h') currentStep = currentStep - 10;
    if (command == 'j') currentStep = currentStep + 10;
    if (command == 'y') currentStep = currentStep - 100;
    if (command == 'u') currentStep = currentStep + 100;
    if (command == 'k') currentStep = currentStep - 1000;
    if (command == 'i') currentStep = currentStep + 1000;

    // Correct current step if requested more than possible
    if (currentStep < 0) currentStep = 0;
    if (currentStep >= sequenceLength) currentStep = sequenceLength - 1;

    // Quicksave creation command
    if (command == 's')
    {
      // Storing state file
      std::string saveFileName = "quicksave.state";

      std::string saveData;
      saveData.resize(stateSize);
      memcpy(saveData.data(), stateData, stateSize);
      if (jaffarCommon::file::saveStringToFile(saveData, saveFileName.c_str()) == false) JAFFAR_THROW_LOGIC("[ERROR] Could not save state file: %s\n", saveFileName.c_str());
      printw("[] Saved state to %s\n", saveFileName.c_str());

      // Do no show frame info again after this action
      showFrameInfo = false;
    }

    // Start playback from current point
    if (command == 'p') isReproduce = true;

    // Start playback from current point
    if (command == 'q') continueRunning = false;
  }

  // Ending ncurses window
  jaffarCommon::logger::finalizeTerminal();
}
