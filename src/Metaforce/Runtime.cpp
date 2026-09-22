#include "Metaforce/Runtime.hpp"

#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Metaforce/Limiter.hpp"
#include "MetroidPrime/CArchitectureMessage.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CMemoryCardDriver.hpp"
#include "MetroidPrime/CStateSetterFlow.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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
int exitCode = 0;
Limiter limiter;

struct WarpOptions {
  unsigned int world;
  int area;
  std::optional< u64 > layerBits;
  std::vector< TEditorId > relays;
};

struct StartupOptions {
  std::optional< WarpOptions > warp;
  std::optional< int > saveSlot;
  std::unique_ptr< CMemoryCardDriver > card;
};

std::optional< StartupOptions > startup;

unsigned int ParseWarpNumber(const std::string& text, std::string_view name, unsigned int maximum) {
  try {
    unsigned int value;
    cxxopts::values::parse_value(text, value);
    if (value <= maximum) {
      return value;
    }
  } catch (const cxxopts::exceptions::exception&) {
  }
  throw cxxopts::exceptions::parsing(fmt::format("--warp: invalid {} '{}'", name, text));
}

WarpOptions ParseWarpOptions(const std::vector< std::string >& values) {
  if (values.size() < 2) {
    throw cxxopts::exceptions::parsing("--warp requires WORLD,AREA (comma-separated)");
  }
  WarpOptions result{
      .world = ParseWarpNumber(values[0], "world index", 8),
      .area = static_cast< int >(
          ParseWarpNumber(values[1], "area index", std::numeric_limits< int >::max())),
  };
  for (size_t i = 2; i < values.size(); ++i) {
    const auto& value = values[i];
    if (value.starts_with("0x") || value.starts_with("0X")) {
      if (result.relays.size() == 512) {
        throw cxxopts::exceptions::parsing("--warp accepts at most 512 memory relays");
      }
      result.relays.emplace_back(
          ParseWarpNumber(value, "memory relay", std::numeric_limits< unsigned int >::max()));
    } else {
      if (value.empty() || value.size() > 64 ||
          value.find_first_not_of("01") != std::string::npos) {
        throw cxxopts::exceptions::parsing("--warp layer bits must contain 1 to 64 binary digits");
      }
      u64 bits = result.layerBits.value_or(0);
      for (size_t layer = 0; layer < value.size(); ++layer) {
        if (value[layer] == '1') {
          bits |= u64(1) << layer;
        }
      }
      result.layerBits = bits;
    }
  }
  return result;
}

bool FailStartup(const std::string& message) {
  Log.error("{}", message);
  startup->card.reset();
  exitCode = 1;
  shouldTerminate = true;
  return false;
}

CAssetId FindWarpWorld(unsigned int index) {
  const std::string filename = fmt::format("{}{}.pak", gpTweakGame->GetWorldPrefix().data(), index);
  auto& loader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < loader.GetPakCount(); ++i) {
    const auto* pak = loader.GetPakFile(i);
    if (!pak->IsWorldPak() || pak->GetDvdFile().GetFilename().data() != filename) {
      continue;
    }
    for (const auto& entry : pak->GetStringToObjectList()) {
      if (entry.second.GetType() == 'MLVL') {
        return entry.second.GetId();
      }
    }
  }
  return kInvalidAssetId;
}

bool ApplyWarp(const WarpOptions& warp, bool loadedSave) {
  const CAssetId worldId = FindWarpWorld(warp.world);
  if (worldId == kInvalidAssetId || !gpMemoryCard->HasSaveWorldMemory(worldId)) {
    return FailStartup(fmt::format("--warp: world {} was not found on this disc", warp.world));
  }
  const auto& worldMemory = gpMemoryCard->GetSaveWorldMemory(worldId);
  const auto& areaLayers = worldMemory.GetDefaultLayerStates();
  if (warp.area >= areaLayers.size()) {
    return FailStartup(fmt::format("--warp: area {} is out of range for world {} ({} areas)",
                                   warp.area, warp.world, areaLayers.size()));
  }
  const int layerCount = areaLayers[warp.area].m_layerCount;
  if (warp.layerBits &&
      (layerCount > 64 || (layerCount < 64 && (*warp.layerBits >> layerCount) != 0))) {
    return FailStartup(fmt::format("--warp: area {} has {} layers", warp.area, layerCount));
  }
  if (!warp.relays.empty()) {
    TLockedToken< CWorldSaveGameInfo > saveWorld =
        gpSimplePool->GetObj(SObjectTag('SAVW', worldMemory.GetSaveWorldAssetId()));
    for (const auto relay : warp.relays) {
      if (saveWorld->GetRelayIndex(relay) < 0) {
        return FailStartup(fmt::format("--warp: memory relay 0x{:08X} is not in world {}",
                                       relay.value, warp.world));
      }
    }
  }

  if (!loadedSave) {
    gpMain->ResetGameState();
    gpGameState->GameOptions().ResetToDefaults();
  }
  gpGameState->SetCurrentWorldId(worldId);
  auto& world = gpGameState->StateForWorld(worldId);
  world.SetAreaId(TAreaId(warp.area));
  world.SetDesiredAreaAssetId(kInvalidAssetId);
  if (warp.layerBits) {
    for (int layer = 0; layer < layerCount; ++layer) {
      world.GetLayerState()->SetLayerActive(TAreaId(warp.area), TLayerId(layer),
                                            ((*warp.layerBits >> layer) & 1) != 0);
    }
  }
  for (const auto relay : warp.relays) {
    world.Mailbox()->AddMsg(relay);
  }
  Log.info("Warping to world {} (0x{:08X}), area {}", warp.world, worldId, warp.area);
  return true;
}

bool UpdateStartup() {
  if (shouldTerminate) {
    return false;
  }
  const bool loadedSave = startup->saveSlot.has_value();
  if (loadedSave) {
    if (!startup->card) {
      startup->card = std::make_unique< CMemoryCardDriver >(
          CMemoryCardSys::kCS_SlotA, kInvalidAssetId, kInvalidAssetId, kInvalidAssetId, true);
      startup->card->StartCardProbe();
    }
    auto& card = *startup->card;
    card.Update();
    const auto state = card.GetState();
    if (state == kS_CardCheckDone) {
      card.IndexFiles();
      return false;
    }
    if (state == kS_NoCard) {
      return FailStartup("--load-save: no memory card is available in slot A");
    }
    const auto error = card.GetError();
    if (state != kS_Ready) {
      if (error == CMemoryCardDriver::kE_FileMissing) {
        return FailStartup("--load-save: no Metroid Prime save file was found on memory card A");
      }
      if (error != CMemoryCardDriver::kE_OK ||
          (state != kS_CardProbe && !CMemoryCardDriver::IsCardBusy(state))) {
        return FailStartup(
            fmt::format("--load-save: failed to read memory card A (state {}, error {})",
                        static_cast< int >(state), static_cast< int >(error)));
      }
      return false;
    }
    const int slot = *startup->saveSlot;
    if (card.GetGameFileStateInfo(slot) == nullptr) {
      return FailStartup(fmt::format("--load-save: save slot {} is empty", slot + 1));
    }
    card.BuildNewFileSlot(slot);
    startup->card.reset();
    Log.info("Loaded save slot {}", slot + 1);
  }
  if (startup->warp && !ApplyWarp(*startup->warp, loadedSave)) {
    return false;
  }
  gpGameState->GameOptions().EnsureOptions();
  gpGameState->WriteBackupBuf();
  startup.reset();
  return true;
}

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
      "backend", "Graphics backend to use (auto, d3d11, d3d12, metal, vulkan, opengl, opengles)",
      cxxopts::value< AuroraBackend >()->default_value("auto"))(
      "warp", "Start at WORLD,AREA[,LAYERBITS][,0xRELAY...]",
      cxxopts::value< std::vector< std::string > >(),
      "WORLD,AREA,...")("load-save", "Load save slot (1-3)", cxxopts::value< int >(), "N");
  options.parse_positional("dvd");
  options.positional_help("<disc image>");
  options.allow_unrecognised_options();

  cxxopts::ParseResult args;
  borealis::cli::StandardOptions standardOptions;
  try {
    args = options.parse(argc, argv);
    standardOptions = borealis::cli::parse(args);
    if (args.count("warp") > 1 || args.count("load-save") > 1) {
      throw cxxopts::exceptions::parsing("--warp and --load-save may each be specified only once");
    }
    if (args.count("warp") || args.count("load-save")) {
      startup.emplace();
      if (args.count("warp")) {
        startup->warp = ParseWarpOptions(args["warp"].as< std::vector< std::string > >());
      }
      if (args.count("load-save")) {
        const int slot = args["load-save"].as< int >();
        if (slot < 1 || slot > 3) {
          throw cxxopts::exceptions::parsing("--load-save must be a save slot from 1 to 3");
        }
        startup->saveSlot = slot - 1;
      }
    }
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
  startup.reset();
  aurora_dvd_close();
  aurora_shutdown();
  borealis::log::shutdown();
}

int GetExitCode() { return exitCode; }

bool HasStartupRequest() { return startup.has_value(); }

bool BeginFrame() {
  limiter.Sleep(16670000);
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

CStateSetterFlow::~CStateSetterFlow() { metaforce::startup.reset(); }

CIOWin::EMessageReturn CStateSetterFlow::OnMessage(const CArchitectureMessage& message,
                                                   CArchitectureQueue&) {
  if (message.GetType() != kAM_TimerTick) {
    return kMR_Exit;
  }
  if (metaforce::HasStartupRequest()) {
    return metaforce::UpdateStartup() ? kMR_RemoveIOWinAndExit : kMR_Exit;
  }
  gpMain->RefreshGameState();
  return kMR_RemoveIOWinAndExit;
}
