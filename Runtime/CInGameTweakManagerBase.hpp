#pragma once

#include <string>
#include <vector>

#include "RetroTypes.hpp"

namespace urde {

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
    for (const CTweakValue& val : x0_values)
      if (val.GetName() == name)
        return true;
    return false;
  }

  const CTweakValue* GetTweakValue(std::string_view name) const {
    for (const CTweakValue& val : x0_values)
      if (val.GetName() == name)
        return &val;
    return nullptr;
  }

  bool ReadFromMemoryCard(std::string_view name) { return true; }

  static std::string GetIdentifierForMidiEvent(CAssetId world, CAssetId area, std::string_view midiObj) {
    return fmt::format(fmt("World {} Area {} MidiObject: {}"), world, area, midiObj);
  }
};

} // namespace urde
