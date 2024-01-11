#include "argparse/argparse.hpp"
#include "sha1/sha1.h"
#include "utils.hpp"
#include "emuInstance.hpp"
#include <sstream>

int main(int argc, char *argv[])
{
   // Parsing command line arguments
  argparse::ArgumentParser program("player", "1.0");

  program.add_argument("romFile")
    .help("Path to the rom file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Getting ROM file path
  std::string romFilePath = program.get<std::string>("romFile");

  // Getting ROM data
  std::string romData;
  auto status = loadStringFromFile(romData, romFilePath.c_str());
  if (status == false) EXIT_WITH_ERROR("Could not read rom file: '%s'\n", romFilePath.c_str()); 

  // Getting SHA1 String
  std::string sha1String = SHA1::GetHash((uint8_t*)romData.data(), romData.size());

  // Printing ROM SHA1
  printf("%s\n", sha1String.c_str());

  return 0;
}

