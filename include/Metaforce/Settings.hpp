#pragma once

#include <borealis/config.hpp>

#include <cstdint>
#include <string>

namespace metaforce {

using borealis::config::Var;

// Persistent user settings. Keys and defaults live in Settings.cpp.
struct Settings {
  struct Video {
    Var< bool > fullscreen;
    Var< bool > lockAspectRatio;
  } video;

  struct Interface {
    Var< int > scale;
    Var< bool > sounds;
  } ui;

  // Values edited by the controls demo; nothing reads them and they aren't saved.
  struct Demo {
    Var< bool > scanVisor;
    Var< bool > hintSystem;
    Var< bool > hardMode;
    Var< int > energyTanks;
    Var< int > visorOpacity;
    Var< std::string > saveName;
    Var< int > suit;
    Var< int > beam;
    Var< uint32_t > upgrades;
    Var< int > logbookEntry;
  } demo;
};

Settings& GetSettings();

} // namespace metaforce
