#include "Metaforce/Runtime.hpp"

#include "Kyoto/Basics/COsContext.hpp"
#include "MetroidPrime/CMain.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <aurora/aurora.h>
#include <aurora/dvd.h>
#include <aurora/event.h>
#include <borealis/app_info.hpp>
#include <borealis/aurora_log.h>
#include <borealis/cli.hpp>
#include <borealis/crash.hpp>
#include <borealis/data.hpp>
#include <borealis/io.hpp>
#include <borealis/log.hpp>
#include <borealis/presentation.hpp>

#include <SDL3/SDL_keycode.h>
#include <dolphin/pad.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#endif

namespace cxxopts::values {
template <>
void parse_value(const std::string& text, AuroraBackend& value) {
  if (text == "auto") {
    value = BACKEND_AUTO;
  } else if (text == "d3d11") {
    value = BACKEND_D3D11;
  } else if (text == "d3d12") {
    value = BACKEND_D3D12;
  } else if (text == "metal") {
    value = BACKEND_METAL;
  } else if (text == "vulkan") {
    value = BACKEND_VULKAN;
  } else if (text == "opengl") {
    value = BACKEND_OPENGL;
  } else if (text == "opengles") {
    value = BACKEND_OPENGLES;
  } else if (text == "webgpu") {
    value = BACKEND_WEBGPU;
  } else if (text == "null") {
    value = BACKEND_NULL;
  } else {
    throw exceptions::incorrect_argument_type{text};
  }
}
} // namespace cxxopts::values

namespace metaforce {
namespace {
constexpr borealis::Log Log{"metaforce"};
constexpr borealis::AppInfo AppInfo{
    .appName = "Metaforce",
};

std::optional< borealis::data::Manager > dataManager;
bool shouldTerminate = false;

#if defined(_WIN32)
void ShowConsole() {
  if (GetConsoleWindow() != nullptr) {
    return;
  }
  if (!AttachConsole(ATTACH_PARENT_PROCESS) && !AllocConsole()) {
    Log.warn("Failed to open console (Windows error {})", GetLastError());
    return;
  }
  FILE* stream;
  if (_fileno(stdout) < 0 || _get_osfhandle(_fileno(stdout)) < 0) {
    freopen_s(&stream, "CONOUT$", "w", stdout);
  }
  if (_fileno(stderr) < 0 || _get_osfhandle(_fileno(stderr)) < 0) {
    freopen_s(&stream, "CONOUT$", "w", stderr);
  }
}
#endif

void LoadDefaultKeyBindings() {
  u32 bindingCount = 0;
  if (PADGetKeyButtonBindings(PAD_CHAN0, &bindingCount) != nullptr) {
    return;
  }

  PADKeyButtonBinding buttons[PAD_BUTTON_COUNT] = {
      {SDL_SCANCODE_SPACE, PAD_BUTTON_A},      {SDL_SCANCODE_LSHIFT, PAD_BUTTON_B},
      {SDL_SCANCODE_F, PAD_BUTTON_X},          {SDL_SCANCODE_R, PAD_BUTTON_Y},
      {SDL_SCANCODE_RETURN, PAD_BUTTON_START}, {SDL_SCANCODE_TAB, PAD_TRIGGER_Z},
      {SDL_SCANCODE_Q, PAD_TRIGGER_L},         {SDL_SCANCODE_E, PAD_TRIGGER_R},
      {SDL_SCANCODE_UP, PAD_BUTTON_UP},        {SDL_SCANCODE_DOWN, PAD_BUTTON_DOWN},
      {SDL_SCANCODE_LEFT, PAD_BUTTON_LEFT},    {SDL_SCANCODE_RIGHT, PAD_BUTTON_RIGHT},
  };
  PADKeyAxisBinding axes[PAD_AXIS_COUNT] = {
      {SDL_SCANCODE_D, PAD_AXIS_LEFT_X_POS, 1},  {SDL_SCANCODE_A, PAD_AXIS_LEFT_X_NEG, 1},
      {SDL_SCANCODE_W, PAD_AXIS_LEFT_Y_POS, 1},  {SDL_SCANCODE_S, PAD_AXIS_LEFT_Y_NEG, 1},
      {SDL_SCANCODE_L, PAD_AXIS_RIGHT_X_POS, 1}, {SDL_SCANCODE_J, PAD_AXIS_RIGHT_X_NEG, 1},
      {SDL_SCANCODE_I, PAD_AXIS_RIGHT_Y_POS, 1}, {SDL_SCANCODE_K, PAD_AXIS_RIGHT_Y_NEG, 1},
      {SDL_SCANCODE_Q, PAD_AXIS_TRIGGER_L, 0},   {SDL_SCANCODE_E, PAD_AXIS_TRIGGER_R, 0},
  };

  PADSetKeyButtonBindings(PAD_CHAN0, buttons);
  PADSetKeyAxisBindings(PAD_CHAN0, axes);
  PADSetKeyboardActive(PAD_CHAN0, TRUE);
}
} // namespace

int Initialize(int argc, char** argv) {
  cxxopts::Options options(std::string(AppInfo.appName),
                           "A native reimplementation of Metroid Prime");
  borealis::cli::add_standard_options(options);
  options.add_options()("h,help", "Print usage")(
      "dvd", "Path to game disc image", cxxopts::value< std::string >()->default_value("game.rvz"))(
      "backend",
      "Graphics API backend to use (auto, d3d11, d3d12, metal, vulkan, opengl, opengles)",
      cxxopts::value< AuroraBackend >()->default_value("auto"));
  options.parse_positional("dvd");
  options.positional_help("<disc image>");
  options.allow_unrecognised_options();

  cxxopts::ParseResult args;
  borealis::cli::StandardOptions standardOptions;
  try {
    args = options.parse(argc, argv);
    standardOptions = borealis::cli::parse(args);
  } catch (const cxxopts::exceptions::exception& e) {
    fprintf(stderr, "Error: %s\nUse --help for usage.\n", e.what());
    return 1;
  }

#if defined(_WIN32)
  if (standardOptions.console) {
    ShowConsole();
  }
#endif
  if (args.count("help")) {
    printf("%s\n", options.help().c_str());
    exit(0);
  }

  dataManager.emplace(AppInfo);
  const auto dataStatus = dataManager->initialize(standardOptions.userDir);
  if (!dataStatus && dataStatus.code != borealis::data::ErrorCode::MigrationIncomplete) {
    const auto path = borealis::io::fs_path_to_string(dataStatus.path);
    const auto error =
        dataStatus.systemError ? dataStatus.systemError.message() : "Data directory is unavailable";
    fprintf(stderr, "Error: Failed to initialize data directory '%s': %s\n", path.c_str(),
            error.c_str());
    return 1;
  }

  const auto& paths = dataManager->paths();
  borealis::log::Options logOptions{
      .level = borealis::LogLevel::Debug,
      .fileDirectory = paths.cachePath / "logs",
      .filePrefix = "metaforce",
  };
  standardOptions.apply_to(logOptions);
  borealis::log::init(logOptions);
  borealis::crash::install();

  if (dataStatus.code == borealis::data::ErrorCode::MigrationIncomplete) {
    Log.warn("Data migration from '{}' is incomplete; will retry next launch",
             borealis::io::fs_path_to_string(dataStatus.path));
  }

  const auto userPath = borealis::io::fs_path_to_string(paths.userPath);
  const auto cachePath = borealis::io::fs_path_to_string(paths.cachePath);
  Log.info("User directory: {}", userPath);
  Log.info("Cache directory: {}", cachePath);

  const auto discPath = args["dvd"].as< std::string >();
  if (!aurora_dvd_open(discPath.c_str())) {
    Log.error("Failed to open disc image '{}'", discPath);
    borealis::log::shutdown();
    return 1;
  }

  const AuroraConfig config{
      .appName = AppInfo.appName.data(),
      .userPath = userPath.c_str(),
      .cachePath = cachePath.c_str(),
      .desiredBackend = args["backend"].as< AuroraBackend >(),
      .vsync = true,
      .allowJoystickBackgroundEvents = true,
      .windowPosX = -1,
      .windowPosY = -1,
      .windowWidth = 1280,
      .windowHeight = 960,
      .logCallback = borealis::log::aurora_callback(),
      .logLevel = borealis::log::to_aurora_level(logOptions.level),
  };
  aurora_initialize(argc, argv, &config);
  borealis::presentation::set_preferred_frame_rate(60.f);
  COsContext::mProgressiveMode = true;

  if (!PADInit()) {
    Log.error("PADInit() failed");
    Shutdown();
    return 1;
  }
  LoadDefaultKeyBindings();
  return 0;
}

void Shutdown() {
  aurora_dvd_close();
  aurora_shutdown();
  borealis::log::shutdown();
}

bool BeginFrame() {
  for (const AuroraEvent* event = aurora_update(); event && event->type != AURORA_NONE; ++event) {
    if (event->type == AURORA_EXIT) {
      shouldTerminate = true;
      return false;
    }
  }
  return aurora_begin_frame();
}

void EndFrame() { aurora_end_frame(); }

} // namespace metaforce

bool CMain::CheckTerminate() { return metaforce::shouldTerminate; }
