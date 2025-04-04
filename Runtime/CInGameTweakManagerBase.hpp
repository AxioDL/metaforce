#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Formatting.hpp"

namespace metaforce {

class CTweakValue {
public:
  struct Audio {
    float x0_fadeIn, x4_fadeOut, x8_volume;
    std::string xc_fileName;
    CAssetId x1c_res;
    Audio() = default;
    Audio(float fadeIn, float fadeOut, float vol, std::string_view fileName, u32 handle)
    : x0_fadeIn(fadeIn), x4_fadeOut(fadeOut), x8_volume(vol), xc_fileName(fileName), x1c_res(handle) {}
    float GetFadeIn() const { return x0_fadeIn; }
    float GetFadeOut() const { return x4_fadeOut; }
    float GetVolume() const { return x8_volume; }
    std::string_view GetFileName() const { return xc_fileName; }
    CAssetId GetResId() const { return x1c_res; }
    static Audio None() { return Audio{0.f, 0.f, 0.f, "", 0}; }
  };
  enum class EType {};

private:
  EType x0_type;
  std::string x4_key;
  std::string x14_str;
  Audio x24_audio;
  union {
    u32 x44_int;
    float x44_flt;
  };

public:
  CTweakValue() = default;
  // CTweakValue(CTextInputStream&);
  // void PutTo(CTextOutStream&);
  std::string_view GetName() const { return x4_key; }
  std::string_view GetValueAsString() const;
  void SetValueFromString(std::string_view);
  const Audio& GetAudio() const { return x24_audio; }
  EType GetType() const { return x0_type; }
};

class CInGameTweakManagerBase {
protected:
  std::vector<CTweakValue> x0_values;

public:
  bool HasTweakValue(std::string_view name) const {
    return std::any_of(x0_values.cbegin(), x0_values.cend(),
                       [name](const auto& value) { return value.GetName() == name; });
  }

  const CTweakValue* GetTweakValue(std::string_view name) const {
    const auto iter = std::find_if(x0_values.cbegin(), x0_values.cend(),
                                   [name](const auto& value) { return value.GetName() == name; });
    if (iter == x0_values.cend()) {
      return nullptr;
    }
    return &*iter;
  }

  bool ReadFromMemoryCard(std::string_view name) { return true; }

  static std::string GetIdentifierForMidiEvent(CAssetId world, CAssetId area, std::string_view midiObj) {
    return fmt::format("World {} Area {} MidiObject: {}", world, area, midiObj);
  }
};

} // namespace metaforce
