#include <cstdlib>
#include "argparse/argparse.hpp"
#include "utils.hpp"
#include "emuInstance.hpp"
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

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

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

  // Printing provided parameters
  printf("Rom File Path:      %s\n", romFilePath.c_str());
  printf("Sequence File Path: %s\n", sequenceFilePath.c_str());
  printf("State File Path:    %s\n", stateFilePath.c_str());

  // Creating emulator instance
  auto e = EmuInstance(romFilePath, stateFilePath);

  // Creating playback instance
  auto p = PlaybackInstance(&e);

  while(true) p.renderFrame(0, "");
}

