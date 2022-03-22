#include "CMemoryCardSys.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"
#include <Runtime/CBasics.hpp>
#include <SDL_filesystem.h>

namespace metaforce {

static std::optional<std::string> GetPrefPath(const char* app) {
  char* path = SDL_GetPrefPath(nullptr, app);
  if (path == nullptr) {
    return {};
  }
  std::string str{path};
  SDL_free(path);
  return str;
}

std::string CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    const auto dolphinPath = GetPrefPath("dolphin-emu");
    if (!dolphinPath) {
      return {};
    }
    auto path = *dolphinPath;
    path += fmt::format(FMT_STRING("GC/MemoryCard{:c}.USA.raw"), slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');

    CBasics::Sstat theStat{};
    if (CBasics::Stat(path.c_str(), &theStat) != 0 || !S_ISREG(theStat.st_mode)) {
      /* legacy case for older dolphin versions */
      const char* home = getenv("HOME");
      if (home == nullptr || home[0] != '/') {
        return {};
      }

      path = home;
#ifndef __APPLE__
      path += fmt::format(FMT_STRING("/.dolphin-emu/GC/MemoryCard{:c}.USA.raw"),
                          slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
#else
      path += fmt::format(FMT_STRING("/Library/Application Support/Dolphin/GC/MemoryCard{:c}.USA.raw"),
                          slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
#endif
      if (CBasics::Stat(path.c_str(), &theStat) != 0 || !S_ISREG(theStat.st_mode)) {
        return {};
      }
    }

    return path;
  }
  return {};
}

std::string CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot, bool dolphin) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    if (dolphin) {
      const auto dolphinPath = GetPrefPath("dolphin-emu");
      if (!dolphinPath) {
        return {};
      }
      auto path = *dolphinPath + "GC";
      int ret = mkdir(path.c_str(), 0755);
      if (ret != 0 && errno != EEXIST) {
        return {};
      }

      path += fmt::format(FMT_STRING("/MemoryCard{:c}.USA.raw"), slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
      auto* file = fopen(path.c_str(), "wbe");
      if (file != nullptr) {
        fclose(file);
        return path;
      }
    } else {
      std::string path = _GetDolphinCardPath(slot);
      if (path.find('/') == std::string::npos) {
        auto basePath = GetPrefPath("metaforce");
        if (!basePath) {
          return {};
        }
        path = *basePath + _GetDolphinCardPath(slot);
      }
      std::string tmpPath = path.substr(0, path.find_last_of('/'));
      int ret = mkdir(tmpPath.c_str(), 0755);
      if (ret != 0 && ret != EEXIST) {
        return {};
      }
      auto* file = fopen(path.c_str(), "wbe");
      if (file != nullptr) {
        fclose(file);
        return path;
      }
    }
  }
  return {};
}

} // namespace metaforce
