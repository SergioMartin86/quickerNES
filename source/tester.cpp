#include <sstream>
#include <chrono>
#include "argparse/argparse.hpp"
#include "sha1/sha1.hpp"
#include "utils.hpp"
#include "nlohmann/json.hpp"
#include "emuInstance.hpp"

int main(int argc, char *argv[])
{
   // Parsing command line arguments
  argparse::ArgumentParser program("player", "1.0");

  program.add_argument("scriptFile")
    .help("Path to the test script file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Getting test script file path
  std::string scriptFilePath = program.get<std::string>("scriptFile");

  // Loading script file
  std::string scriptJsonRaw;
  if (loadStringFromFile(scriptJsonRaw, scriptFilePath.c_str()) == false)  EXIT_WITH_ERROR("Could not find/read script file: %s\n", scriptFilePath.c_str());

  // Parsing script 
  const auto scriptJson = nlohmann::json::parse(scriptJsonRaw);

  // Getting rom file path
  if (scriptJson.contains("Rom File") == false) EXIT_WITH_ERROR("Script file missing 'Rom File' entry\n");
  if (scriptJson["Rom File"].is_string() == false) EXIT_WITH_ERROR("Script file 'Rom File' entry is not a string\n");
  std::string romFilePath = scriptJson["Rom File"].get<std::string>();

  // Getting initial state file path
  if (scriptJson.contains("Initial State File") == false) EXIT_WITH_ERROR("Script file missing 'Initial State File' entry\n");
  if (scriptJson["Initial State File"].is_string() == false) EXIT_WITH_ERROR("Script file 'Initial State File' entry is not a string\n");
  std::string initialStateFilePath = scriptJson["Initial State File"].get<std::string>();

  // Getting verification state file path
  if (scriptJson.contains("Verification State File") == false) EXIT_WITH_ERROR("Script file missing 'Verification State File' entry\n");
  if (scriptJson["Verification State File"].is_string() == false) EXIT_WITH_ERROR("Script file 'Verification State File' entry is not a string\n");
  std::string verificationStateFilePath = scriptJson["Verification State File"].get<std::string>();

  // Getting sequence file path
  if (scriptJson.contains("Sequence File") == false) EXIT_WITH_ERROR("Script file missing 'Sequence File' entry\n");
  if (scriptJson["Sequence File"].is_string() == false) EXIT_WITH_ERROR("Script file 'Sequence File' entry is not a string\n");
  std::string sequenceFilePath = scriptJson["Sequence File"].get<std::string>();

  // Getting expected ROM SHA1 hash
  if (scriptJson.contains("Expected ROM SHA1") == false) EXIT_WITH_ERROR("Script file missing 'Expected ROM SHA1' entry\n");
  if (scriptJson["Expected ROM SHA1"].is_string() == false) EXIT_WITH_ERROR("Script file 'Expected ROM SHA1' entry is not a string\n");
  std::string expectedROMSHA1 = scriptJson["Expected ROM SHA1"].get<std::string>();

  // Creating emulator instance
  auto e = EmuInstance(romFilePath, initialStateFilePath);
  
  auto initialHash = e.getStateHash();
  printf("[] Initial State Hash:        0x%lX%lX\n", initialHash.first, initialHash.second);

  // Getting state size
  const auto stateSize = e.getStateSize();

  // Getting actual ROM SHA1
  auto romSHA1 = e.getRomSHA1();

  // Checking with the expected SHA1 hash
  if (romSHA1 != expectedROMSHA1) EXIT_WITH_ERROR("Wrong ROM SHA1. Found: '%s', Expected: '%s'\n", romSHA1.c_str(), expectedROMSHA1.c_str());

  // Loading state file for verication, if provided
  std::string verificationState;
  if (verificationStateFilePath != "")
  {
    // Loading file
    if (loadStringFromFile(verificationState, verificationStateFilePath) == false)
     EXIT_WITH_ERROR("[ERROR] Could not find or read from verification state file: %s\n", verificationStateFilePath.c_str());

    // Checking size
    if (verificationState.length() != stateSize) EXIT_WITH_ERROR("[ERROR] The provided verification state has different size from expected: %lu vs %lu\n", verificationState.length(), stateSize);
  }

  // Loading sequence file
  std::string sequenceRaw;
  if (loadStringFromFile(sequenceRaw, sequenceFilePath) == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from input sequence file: %s\n", sequenceFilePath.c_str());

  // Building sequence information
  const auto sequence = split(sequenceRaw, ' ');

  // Getting sequence lenght
  const auto sequenceLength = sequence.size();

  // Printing test information
  printf("[] Running Script:          '%s'\n", scriptFilePath.c_str());
  printf("[] ROM File:                '%s'\n", romFilePath.c_str());
  printf("[] ROM SHA1:                '%s'\n", romSHA1.c_str());
  printf("[] Verification State File: '%s'\n", verificationStateFilePath.c_str());
  printf("[] Sequence File:           '%s'\n", sequenceFilePath.c_str());
  printf("[] Sequence Length:         %lu\n", sequenceLength);
  printf("[] State Size:              %lu bytes\n", stateSize);
  printf("[] Running Test...\n");
  
  fflush(stdout);
  
  // Actually running the sequence
  auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& input : sequence) e.advanceState(input);
  auto tf = std::chrono::high_resolution_clock::now();

  // Calculating running time
  auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
  double elapsedTimeSeconds = (double)dt * 1.0e-9;

  // Calculating final state hash
  const auto finalStateHash = e.getStateHash();

  // If provided, verify result against the passed verification movie
  bool verificationPassed = true;
  hash_t verificationStateHash;
  if (verificationStateFilePath != "") 
  {
    // Loading verification state into the emulator
    e.deserializeState((uint8_t*)verificationState.data());

    // Calculating hash for the verification state
    verificationStateHash = e.getStateHash();

    // Comparing hashes
    if (verificationStateHash != finalStateHash) verificationPassed = false;
  }

  // Printing time information
  printf("[] -----------------------------------------\n");
  printf("[] Elapsed time:            %3.3fs\n", (double)dt * 1.0e-9);
  printf("[] Performance:             %.3f steps / s\n", (double)sequenceLength / elapsedTimeSeconds);
  printf("[] Final State Hash:        0x%lX%lX\n", finalStateHash.first, finalStateHash.second);
  if (verificationStateFilePath != "")
  printf("[] Verification Hash:       0x%lX%lX (%s)\n", verificationStateHash.first, verificationStateHash.second, verificationPassed ? "Passed" : "Failed");

  // Fail if verification didn't pass
  return verificationPassed ? 0 : -1;
}

