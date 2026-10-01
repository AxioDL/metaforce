#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace metaforce::ui {

// TODO: temp cvar system until I impl borealis::config
template < typename T >
class RuntimeVar {
public:
  explicit RuntimeVar(T defaultValue)
  : mValue(defaultValue), mDefaultValue(std::move(defaultValue)) {}

  const T& getValue() const { return mValue; }
  const T& getDefaultValue() const { return mDefaultValue; }
  void setValue(T value) { mValue = std::move(value); }
  operator const T&() const { return mValue; }

private:
  T mValue;
  T mDefaultValue;
};

struct RuntimeConfig {
  struct Video {
    RuntimeVar< bool > fullscreen{false};
    RuntimeVar< bool > lockAspectRatio{false};
  } video;

  struct Interface {
    RuntimeVar< int > scale{100};
    RuntimeVar< bool > sounds{true};
  } ui;

  // Values edited by the controls demo; nothing reads them.
  struct Demo {
    RuntimeVar< bool > scanVisor{true};
    RuntimeVar< bool > hintSystem{true};
    RuntimeVar< bool > hardMode{false};
    RuntimeVar< int > energyTanks{6};
    RuntimeVar< int > visorOpacity{100};
    RuntimeVar< std::string > saveName{"Samus"};
    RuntimeVar< int > suit{0};
    RuntimeVar< int > beam{0};
    RuntimeVar< uint32_t > upgrades{0b0011};
    RuntimeVar< int > logbookEntry{-1};
  } demo;
};

RuntimeConfig& GetRuntimeConfig();

} // namespace metaforce::ui
