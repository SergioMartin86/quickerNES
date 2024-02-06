#include "argparse/argparse.hpp"
#include "nlohmann/json.hpp"
#include "sha1/sha1.hpp"
#include "utils.hpp"
#include "nesInstance.hpp"
#include <chrono>
#include <sstream>
#include <vector>
#include <string>

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

  // Getting differential compression configuration
  if (scriptJson.contains("Differential Compression") == false) EXIT_WITH_ERROR("Script file missing 'Differential Compression' entry\n");
  if (scriptJson["Differential Compression"].is_object() == false) EXIT_WITH_ERROR("Script file 'Differential Compression' entry is not a key/value object\n");
  const auto& differentialCompressionJs = scriptJson["Differential Compression"];

  if (differentialCompressionJs.contains("Enabled") == false) EXIT_WITH_ERROR("Script file missing 'Differential Compression / Enabled' entry\n");
  if (differentialCompressionJs["Enabled"].is_boolean() == false) EXIT_WITH_ERROR("Script file 'Differential Compression / Enabled' entry is not a boolean\n");
  const auto differentialCompressionEnabled = differentialCompressionJs["Enabled"].get<bool>();

  if (differentialCompressionJs.contains("Max Differences") == false) EXIT_WITH_ERROR("Script file missing 'Differential Compression / Max Differences' entry\n");
  if (differentialCompressionJs["Max Differences"].is_number() == false) EXIT_WITH_ERROR("Script file 'Differential Compression / Max Differences' entry is not a number\n");
  const auto differentialCompressionMaxDifferences = differentialCompressionJs["Max Differences"].get<size_t>();

  if (differentialCompressionJs.contains("Use Zlib") == false) EXIT_WITH_ERROR("Script file missing 'Differential Compression / Use Zlib' entry\n");
  if (differentialCompressionJs["Use Zlib"].is_boolean() == false) EXIT_WITH_ERROR("Script file 'Differential Compression / Use Zlib' entry is not a boolean\n");
  const auto differentialCompressionUseZlib = differentialCompressionJs["Use Zlib"].get<bool>();

  // Creating emulator instance
  NESInstance e;

  // Setting controller types
  e.setController1Type(controller1Type);
  e.setController2Type(controller2Type);

  // Loading ROM File
  std::string romFileData;
  if (loadStringFromFile(romFileData, romFilePath) == false) EXIT_WITH_ERROR("Could not rom file: %s\n", romFilePath.c_str());
  e.loadROM((uint8_t*)romFileData.data(), romFileData.size());

  // Calculating ROM SHA1
  auto romSHA1 = SHA1::GetHash((uint8_t *)romFileData.data(), romFileData.size());

  // If an initial state is provided, load it now
  if (initialStateFilePath != "")
  {
    std::string stateFileData;
    if (loadStringFromFile(stateFileData, initialStateFilePath) == false) EXIT_WITH_ERROR("Could not initial state file: %s\n", initialStateFilePath.c_str());
    e.deserializeState((uint8_t*)stateFileData.data());
  }
  
  // Disabling requested blocks from state serialization
  for (const auto& block : stateDisabledBlocks) e.disableStateBlock(block);

  // Disable rendering
  e.disableRendering();

  // Getting state size
  const auto stateSize = e.getStateSize();

  // Getting differential state size
  const auto fixedDiferentialStateSize = e.getDifferentialStateSize();
  const auto fullDifferentialStateSize = fixedDiferentialStateSize + differentialCompressionMaxDifferences;

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
  printf("[] Running Script:                         '%s'\n", scriptFilePath.c_str());
  printf("[] Cycle Type:                             '%s'\n", cycleType.c_str());
  printf("[] Emulation Core:                         '%s'\n", emulationCoreName.c_str());
  printf("[] ROM File:                               '%s'\n", romFilePath.c_str());
  printf("[] Controller Types:                       '%s' / '%s'\n", controller1Type.c_str(), controller2Type.c_str());
  printf("[] ROM Hash:                               'SHA1: %s'\n", romSHA1.c_str());
  printf("[] Sequence File:                          '%s'\n", sequenceFilePath.c_str());
  printf("[] Sequence Length:                        %lu\n", sequenceLength);
  printf("[] State Size:                             %lu bytes - Disabled Blocks:  [ %s ]\n", e.getStateSize(), stateDisabledBlocksOutput.c_str());
  printf("[] Use Differential Compression:           %s\n", differentialCompressionEnabled ? "true" : "false");
  if (differentialCompressionEnabled == true) 
  { 
  printf("[]   + Max Differences:                    %lu\n", differentialCompressionMaxDifferences);    
  printf("[]   + Use Zlib:                           %s\n", differentialCompressionUseZlib ? "true" : "false");
  printf("[]   + Fixed Diff State Size:              %lu\n", fixedDiferentialStateSize);
  printf("[]   + Full Diff State Size:               %lu\n", fullDifferentialStateSize);
  }
  printf("[] ********** Running Test **********\n");

  fflush(stdout);

  // Serializing initial state
  uint8_t *currentState = (uint8_t *)malloc(stateSize);
  e.serializeState(currentState);

  // Serializing differential state data (in case it's used)
  uint8_t *differentialStateData = nullptr;
  size_t differentialStateMaxSizeDetected = 0;
  if (differentialCompressionEnabled == true) 
  {
    differentialStateData = (uint8_t *)malloc(fullDifferentialStateSize);
    differentialStateMaxSizeDetected = e.serializeDifferentialState(differentialStateData, currentState, fullDifferentialStateSize, differentialCompressionUseZlib);
  }

  // Check whether to perform each action
  bool doPreAdvance = cycleType == "Full";
  bool doDeserialize = cycleType == "Rerecord" || cycleType == "Full";
  bool doSerialize = cycleType == "Rerecord" || cycleType == "Full";

  // Actually running the sequence
  auto t0 = std::chrono::high_resolution_clock::now();
  for (const std::string &input : sequence)
  {
    if (doPreAdvance == true) e.advanceState(input);
    
    if (doDeserialize == true)
    {
      if (differentialCompressionEnabled == true)  e.deserializeDifferentialState(differentialStateData, currentState, differentialCompressionUseZlib);
      if (differentialCompressionEnabled == false) e.deserializeState(currentState);
    } 
    
    e.advanceState(input);

    if (doSerialize == true)
    {
      if (differentialCompressionEnabled == true)  differentialStateMaxSizeDetected = std::max(differentialStateMaxSizeDetected, e.serializeDifferentialState(differentialStateData, currentState, fullDifferentialStateSize, differentialCompressionUseZlib));
      if (differentialCompressionEnabled == false) e.serializeState(currentState);
    } 
  }
  auto tf = std::chrono::high_resolution_clock::now();

  // Calculating running time
  auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
  double elapsedTimeSeconds = (double)dt * 1.0e-9;

  // Calculating final state hash
  auto result = calculateStateHash(&e);

  // Creating hash string
  char hashStringBuffer[256];
  sprintf(hashStringBuffer, "0x%lX%lX", result.first, result.second);

  // Printing time information
  printf("[] Elapsed time:                           %3.3fs\n", (double)dt * 1.0e-9);
  printf("[] Performance:                            %.3f inputs / s\n", (double)sequenceLength / elapsedTimeSeconds);
  printf("[] Final State Hash:                       %s\n", hashStringBuffer);
  if (differentialCompressionEnabled == true)
  {
  printf("[] Differential State Max Size Detected:   %lu\n", differentialStateMaxSizeDetected);    
  }
  // If saving hash, do it now
  if (hashOutputFile != "") saveStringToFile(std::string(hashStringBuffer), hashOutputFile.c_str());

  // If reached this point, everything ran ok
  return 0;
}
