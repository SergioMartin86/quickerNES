#include <cstdlib>
#include "argparse.hpp"
#include "utils.hpp"

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("player", "1.0");

  program.add_argument("romFile")
    .help("path to the rom file to run.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the solution sequence file (.sol) to reproduce.")
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
}

