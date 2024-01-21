#include "argparse/argparse.hpp"
#include "nlohmann/json.hpp"
#include "sha1/sha1.hpp"
#include "utils.hpp"
#include <chrono>
#include <sstream>
#include <vector>
#include <string>

#ifdef _USE_QUICKNES
  #include "quickNESInstance.hpp"
#endif

#ifdef _USE_QUICKERNES
  #include "quickerNESInstance.hpp"
#endif

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("tester", "1.0");

  program.add_argument("scriptFile")
    .help("Path to the test script file to run.")
    .required();

  program.add_argument("--cycleType")
    .help("Specifies the emulation actions to be performed per each input. Possible values: 'Simple': performs only advance state, 'Rerecord': performs load/advance/save, and 'Full': performs load/advance/save/advance.")
    .default_value(std::string("Simple"));

  program.add_argument("--hashOutputFile")
    .help("Path to write the hash output to.")
    .default_value(std::string(""));

  // Try to parse arguments
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str());
  }

  // Getting test script file path
  std::string scriptFilePath = program.get<std::string>("scriptFile");

  // Getting path where to save the hash output (if any)
  std::string hashOutputFile = program.get<std::string>("--hashOutputFile");

  // Getting reproduce flag
  std::string cycleType = program.get<std::string>("--cycleType");

  // Loading script file
  std::string scriptJsonRaw;
  if (loadStringFromFile(scriptJsonRaw, scriptFilePath) == false) EXIT_WITH_ERROR("Could not find/read script file: %s\n", scriptFilePath.c_str());

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

  // Getting sequence file path
  if (scriptJson.contains("Sequence File") == false) EXIT_WITH_ERROR("Script file missing 'Sequence File' entry\n");
  if (scriptJson["Sequence File"].is_string() == false) EXIT_WITH_ERROR("Script file 'Sequence File' entry is not a string\n");
  std::string sequenceFilePath = scriptJson["Sequence File"].get<std::string>();

  // Getting expected ROM SHA1 hash
  if (scriptJson.contains("Expected ROM SHA1") == false) EXIT_WITH_ERROR("Script file missing 'Expected ROM SHA1' entry\n");
  if (scriptJson["Expected ROM SHA1"].is_string() == false) EXIT_WITH_ERROR("Script file 'Expected ROM SHA1' entry is not a string\n");
  std::string expectedROMSHA1 = scriptJson["Expected ROM SHA1"].get<std::string>();

  // Parsing disabled blocks in lite state serialization
  std::vector<std::string> stateDisabledBlocks;
  std::string stateDisabledBlocksOutput;
  if (scriptJson.contains("Disable State Blocks") == false) EXIT_WITH_ERROR("Script file missing 'Disable State Blocks' entry\n");
  if (scriptJson["Disable State Blocks"].is_array() == false) EXIT_WITH_ERROR("Script file 'Disable State Blocks' is not an array\n");
  for (const auto& entry : scriptJson["Disable State Blocks"])
  {
    if (entry.is_string() == false) EXIT_WITH_ERROR("Script file 'Disable State Blocks' entry is not a string\n");
    stateDisabledBlocks.push_back(entry.get<std::string>());
    stateDisabledBlocksOutput += entry.get<std::string>() + std::string(" ");
  } 
  
  // Getting Controller 1 type
  if (scriptJson.contains("Controller 1 Type") == false) EXIT_WITH_ERROR("Script file missing 'Controller 1 Type' entry\n");
  if (scriptJson["Controller 1 Type"].is_string() == false) EXIT_WITH_ERROR("Script file 'Controller 1 Type' entry is not a string\n");
  std::string controller1Type = scriptJson["Controller 1 Type"].get<std::string>();

  // Getting Controller 2 type
  if (scriptJson.contains("Controller 2 Type") == false) EXIT_WITH_ERROR("Script file missing 'Controller 2 Type' entry\n");
  if (scriptJson["Controller 2 Type"].is_string() == false) EXIT_WITH_ERROR("Script file 'Controller 2 Type' entry is not a string\n");
  std::string controller2Type = scriptJson["Controller 2 Type"].get<std::string>();

  // Creating emulator instance
  #ifdef _USE_QUICKNES
  auto e = QuickNESInstance();
  #endif

  #ifdef _USE_QUICKERNES
  auto e = quickerNES::QuickerNESInstance();
  #endif

  // Setting controller types
  e.setController1Type(controller1Type);
  e.setController2Type(controller2Type);

  // Disabling requested blocks from light state serialization
  for (const auto& block : stateDisabledBlocks) e.disableLiteStateBlock(block);

  // Loading ROM File
  e.loadROMFile(romFilePath);

  // If an initial state is provided, load it now
  if (initialStateFilePath != "") e.loadStateFile(initialStateFilePath);

  // Disable rendering
  e.disableRendering();

  // Getting lite state size
  const auto liteStateSize = e.getLiteStateSize();

  // Getting actual ROM SHA1
  auto romSHA1 = e.getRomSHA1();

  // Checking with the expected SHA1 hash
  if (romSHA1 != expectedROMSHA1) EXIT_WITH_ERROR("Wrong ROM SHA1. Found: '%s', Expected: '%s'\n", romSHA1.c_str(), expectedROMSHA1.c_str());

  // Loading sequence file
  std::string sequenceRaw;
  if (loadStringFromFile(sequenceRaw, sequenceFilePath) == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from input sequence file: %s\n", sequenceFilePath.c_str());

  // Building sequence information
  const auto sequence = split(sequenceRaw, ' ');

  // Getting sequence lenght
  const auto sequenceLength = sequence.size();

  // Getting emulation core name
  std::string emulationCoreName = e.getCoreName();

  // Printing test information
  printf("[] -----------------------------------------\n");
  printf("[] Running Script:          '%s'\n", scriptFilePath.c_str());
  printf("[] Cycle Type:              '%s'\n", cycleType.c_str());
  printf("[] Emulation Core:          '%s'\n", emulationCoreName.c_str());
  printf("[] ROM File:                '%s'\n", romFilePath.c_str());
  printf("[] Controller Types:        '%s' / '%s'\n", controller1Type.c_str(), controller2Type.c_str());
  //printf("[] ROM SHA1:                '%s'\n", romSHA1.c_str());
  printf("[] Sequence File:           '%s'\n", sequenceFilePath.c_str());
  printf("[] Sequence Length:         %lu\n", sequenceLength);
  #ifdef _USE_QUICKNES
  printf("[] State Size:              %lu bytes\n", e.getFullStateSize());
  #endif
  #ifdef _USE_QUICKERNES
  printf("[] State Size:              %lu bytes - Disabled Blocks:  [ %s ]\n", e.getLiteStateSize(), stateDisabledBlocksOutput.c_str());
  #endif
  printf("[] ********** Running Test **********\n");

  fflush(stdout);

  // Serializing initial state
  uint8_t *currentState = (uint8_t *)malloc(liteStateSize);
  e.serializeLiteState(currentState);

  // Check whether to perform each action
  bool doPreAdvance = cycleType == "Full";
  bool doDeserialize = cycleType == "Rerecord" || cycleType == "Full";
  bool doSerialize = cycleType == "Rerecord" || cycleType == "Full";

  // Actually running the sequence
  auto t0 = std::chrono::high_resolution_clock::now();
  for (const std::string &input : sequence)
  {
    if (doPreAdvance == true) e.advanceState(input);
    if (doDeserialize == true) e.deserializeLiteState(currentState);
    e.advanceState(input);
    if (doSerialize == true) e.serializeLiteState(currentState);
  }
  auto tf = std::chrono::high_resolution_clock::now();

  // Calculating running time
  auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
  double elapsedTimeSeconds = (double)dt * 1.0e-9;

  // Calculating final state hash
  const auto finalStateHash = e.getStateHash();

  // Creating hash string
  char hashStringBuffer[256];
  sprintf(hashStringBuffer, "0x%lX%lX", finalStateHash.first, finalStateHash.second);

  // Printing time information
  printf("[] Elapsed time:            %3.3fs\n", (double)dt * 1.0e-9);
  printf("[] Performance:             %.3f inputs / s\n", (double)sequenceLength / elapsedTimeSeconds);
  printf("[] Final State Hash:        %s\n", hashStringBuffer);

  // If saving hash, do it now
  if (hashOutputFile != "") saveStringToFile(std::string(hashStringBuffer), hashOutputFile.c_str());

  // If reached this point, everything ran ok
  return 0;
}
