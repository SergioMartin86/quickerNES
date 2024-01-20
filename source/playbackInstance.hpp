#pragma once

#include "emuInstance.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <hqn/hqn.h>
#include <hqn/hqn_gui_controller.h>
#include <string>
#include <unistd.h>
#include <utils.hpp>

#define _INVERSE_FRAME_RATE 16667

// Creating emulator instance
#ifdef _USE_QUICKNES
  typedef Nes_Emu emulator_t;
#endif

#ifdef _USE_QUICKERNES
  typedef quickerNES::Emu emulator_t;
#endif

struct stepData_t
{
  std::string input;
  uint8_t *stateData;
  hash_t hash;
};

class PlaybackInstance
{
  static const uint16_t image_width = 256;
  static const uint16_t image_height = 240;

  public:
  void addStep(const std::string &input)
  {
    stepData_t step;
    step.input = input;
    step.stateData = (uint8_t *)malloc(_emu->getFullStateSize());
    _emu->serializeFullState(step.stateData);
    step.hash = _emu->getStateHash();

    // Adding the step into the sequence
    _stepSequence.push_back(step);
  }

  // Initializes the playback module instance
  PlaybackInstance(EmuInstance *emu, const std::vector<std::string> &sequence, const std::string &overlayPath = "") : _emu(emu)
  {
    // Enabling emulation rendering
    _emu->enableRendering();

    // Loading Emulator instance HQN
    _hqnState.setEmulatorPointer(_emu->getInternalEmulatorPointer());
    static uint8_t video_buffer[image_width * image_height];
    _hqnState.m_emu->set_pixels(video_buffer, image_width + 8);

    // Building sequence information
    for (const auto &input : sequence)
    {
      // Adding new step
      addStep(input);

      // Advance state based on the input received
      _emu->advanceState(input);
    }

    // Adding last step with no input
    addStep("<End Of Sequence>");

    // Loading overlay, if provided
    if (overlayPath != "")
    {
      // Using overlay
      _useOverlay = true;

      // Loading overlay images
      std::string imagePath;

      imagePath = _overlayPath + std::string("/base.png");
      _overlayBaseSurface = IMG_Load(imagePath.c_str());
      if (_overlayBaseSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_a.png");
      _overlayButtonASurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonASurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_b.png");
      _overlayButtonBSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonBSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_select.png");
      _overlayButtonSelectSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonSelectSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_start.png");
      _overlayButtonStartSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonStartSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_left.png");
      _overlayButtonLeftSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonLeftSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_right.png");
      _overlayButtonRightSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonRightSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_up.png");
      _overlayButtonUpSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonUpSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());

      imagePath = _overlayPath + std::string("/button_down.png");
      _overlayButtonDownSurface = IMG_Load(imagePath.c_str());
      if (_overlayButtonDownSurface == NULL) EXIT_WITH_ERROR("[Error] Could not load image: %s, Reason: %s\n", imagePath.c_str(), SDL_GetError());
    }

    // Opening rendering window
    SDL_SetMainReady();

    // We can only call SDL_InitSubSystem once
    if (!SDL_WasInit(SDL_INIT_VIDEO))
      if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        EXIT_WITH_ERROR("Failed to initialize video: %s", SDL_GetError());

    // Creating HQN GUI
    _hqnGUI = hqn::GUIController::create(_hqnState);
    _hqnGUI->setScale(1);
  }

  // Function to render frame
  void renderFrame(const size_t stepId)
  {
    // Checking the required step id does not exceed contents of the sequence
    if (stepId > _stepSequence.size()) EXIT_WITH_ERROR("[Error] Attempting to render a step larger than the step sequence");

    // Getting step information
    const auto &step = _stepSequence[stepId];

    // Pointer to overlay images (NULL if unused)
    SDL_Surface *overlayButtonASurface = NULL;
    SDL_Surface *overlayButtonBSurface = NULL;
    SDL_Surface *overlayButtonSelectSurface = NULL;
    SDL_Surface *overlayButtonStartSurface = NULL;
    SDL_Surface *overlayButtonLeftSurface = NULL;
    SDL_Surface *overlayButtonRightSurface = NULL;
    SDL_Surface *overlayButtonUpSurface = NULL;
    SDL_Surface *overlayButtonDownSurface = NULL;

    // Load correct overlay images, if using overlay
    if (_useOverlay == true)
    {
      if (step.input.find("A") != std::string::npos) overlayButtonASurface = _overlayButtonASurface;
      if (step.input.find("B") != std::string::npos) overlayButtonBSurface = _overlayButtonBSurface;
      if (step.input.find("S") != std::string::npos) overlayButtonSelectSurface = _overlayButtonSelectSurface;
      if (step.input.find("T") != std::string::npos) overlayButtonStartSurface = _overlayButtonStartSurface;
      if (step.input.find("L") != std::string::npos) overlayButtonLeftSurface = _overlayButtonLeftSurface;
      if (step.input.find("R") != std::string::npos) overlayButtonRightSurface = _overlayButtonRightSurface;
      if (step.input.find("U") != std::string::npos) overlayButtonUpSurface = _overlayButtonUpSurface;
      if (step.input.find("D") != std::string::npos) overlayButtonDownSurface = _overlayButtonDownSurface;
    }

    // Since we do not store the blit information (too much memory), we need to load the previous frame and re-run the input

    // If its the first step, then simply reset
    if (stepId == 0) _emu->doHardReset();

    // Else we load the previous frame
    if (stepId > 0)
    {
      const auto stateData = getStateData(stepId - 1);
      _emu->deserializeFullState(stateData);
      _emu->advanceState(getStateInput(stepId - 1));
    }

    // Updating image
    int32_t curBlit[BLIT_SIZE];
    saveBlit(_emu->getInternalEmulatorPointer(), curBlit, hqn::HQNState::NES_VIDEO_PALETTE, 0, 0, 0, 0);
    _hqnGUI->update_blit(curBlit, _overlayBaseSurface, overlayButtonASurface, overlayButtonBSurface, overlayButtonSelectSurface, overlayButtonStartSurface, overlayButtonLeftSurface, overlayButtonRightSurface, overlayButtonUpSurface, overlayButtonDownSurface);
  }

  size_t getSequenceLength() const
  {
    return _stepSequence.size();
  }

  const std::string getInput(const size_t stepId) const
  {
    // Checking the required step id does not exceed contents of the sequence
    if (stepId > _stepSequence.size()) EXIT_WITH_ERROR("[Error] Attempting to render a step larger than the step sequence");

    // Getting step information
    const auto &step = _stepSequence[stepId];

    // Returning step input
    return step.input;
  }

  const uint8_t *getStateData(const size_t stepId) const
  {
    // Checking the required step id does not exceed contents of the sequence
    if (stepId > _stepSequence.size()) EXIT_WITH_ERROR("[Error] Attempting to render a step larger than the step sequence");

    // Getting step information
    const auto &step = _stepSequence[stepId];

    // Returning step input
    return step.stateData;
  }

  const hash_t getStateHash(const size_t stepId) const
  {
    // Checking the required step id does not exceed contents of the sequence
    if (stepId > _stepSequence.size()) EXIT_WITH_ERROR("[Error] Attempting to render a step larger than the step sequence");

    // Getting step information
    const auto &step = _stepSequence[stepId];

    // Returning step input
    return step.hash;
  }

  const std::string getStateInput(const size_t stepId) const
  {
    // Checking the required step id does not exceed contents of the sequence
    if (stepId > _stepSequence.size()) EXIT_WITH_ERROR("[Error] Attempting to render a step larger than the step sequence");

    // Getting step information
    const auto &step = _stepSequence[stepId];

    // Returning step input
    return step.input;
  }

  private:
  // Internal sequence information
  std::vector<stepData_t> _stepSequence;

  // Storage for the HQN state
  hqn::HQNState _hqnState;

  // Storage for the HQN GUI controller
  hqn::GUIController *_hqnGUI;

  // Pointer to the contained emulator instance
  EmuInstance *const _emu;

  // Flag to store whether to use the button overlay
  bool _useOverlay = false;

  // Overlay info
  std::string _overlayPath;
  SDL_Surface *_overlayBaseSurface = NULL;
  SDL_Surface *_overlayButtonASurface;
  SDL_Surface *_overlayButtonBSurface;
  SDL_Surface *_overlayButtonSelectSurface;
  SDL_Surface *_overlayButtonStartSurface;
  SDL_Surface *_overlayButtonLeftSurface;
  SDL_Surface *_overlayButtonRightSurface;
  SDL_Surface *_overlayButtonUpSurface;
  SDL_Surface *_overlayButtonDownSurface;
};
