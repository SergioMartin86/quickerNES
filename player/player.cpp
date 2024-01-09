#include <cstdlib>
#include "argparse.hpp"
#include "utils.hpp"
#include "core/emuInstance.hpp"

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
  .default_value("");

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
  std::string sequenceFile = program.get<std::string>("sequenceFile");

  // If initial state file is specified, load it
  std::string sequenceFile = program.get<std::string>("stateFile");

  // Getting reproduce flag
  bool isReproduce = program.get<bool>("--reproduce");

  // Getting reproduce flag
  bool disableRender = program.get<bool>("--disableRender");
}

