#include "MetroidPrime/CInGameTweakManager.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CResFactory.hpp"

// The retail string pool retains text from the developer tweak-file tools.
static const char* const skTweakFileText[] = {
    "String",
    "Int",
    "Real",
    "Bool",
    "Audio",
    "??(??)",
    "MidiObject",
    "World %8.8x",
    "Area %8.8x MusicObject: %s",
    "World %8.8x Area %8.8x MidiObject: %s",
    "WorldDefault: %8.8x",
    ".adp",
    "Audio/",
    "MIDI: ",
    "TweakFile\n",
    "Version %d\n",
    "Value Count: %d\n",
    "__BAD_TOKEN__",
    "Value\n{\n   Name: %s\n   Type: %s\n   Value: %s\n}\n",
    "TweakFile",
    "Version %d",
    "Value Count: %d",
    "",
    "True",
    ";",
    "%d",
    "%f",
    "False",
    "%f;%f;%f;%s;%d",
    "Value",
    "{",
    "Name:",
    "Type:",
    "Value:",
    " \t\n\r\"",
};

// These shared instantiations survive the developer-only callers in this unit.
template class rstl::vector< rstl::pair< rstl::string, SObjectTag > >;

template <>
rstl::string rstl::basic_string< char >::substr(int pos, int count) const {
  const pair< const_iterator, const_iterator > range = range_iterator(pos, count);
  return basic_string(range.first, range.second);
}

CInGameTweakManager::CInGameTweakManager() {}

bool CInGameTweakManager::HasTweakValue(const rstl::string& name) const {
  for (AUTO(it, mValues.begin()); it != mValues.end(); ++it) {
    if (rstl::operator==(rstl::istring(it->GetName().c_str()), rstl::istring(name.c_str()))) {
      return true;
    }
  }

  return false;
}

const CTweakValue* CInGameTweakManager::GetTweakValue(const rstl::string& name) const {
  for (AUTO(it, mValues.begin()); it != mValues.end(); ++it) {
    if (rstl::operator==(rstl::istring(it->GetName().c_str()), rstl::istring(name.c_str()))) {
      return &*it;
    }
  }

  return nullptr;
}

bool CInGameTweakManager::ReadFromMemoryCard(const rstl::string&) { return false; }

rstl::vector< rstl::pair< rstl::string, SObjectTag > >
CResFactory::GetResourceIdToNameList() const {
  return mResLoader.GetResourceIdToNameList();
}

rstl::istring rstl::istring_l(const char* data) { return istring(istring::literal_t(), data); }

rstl::string CInGameTweakManager::GetIdentifierForWorldDefaultMusic(CAssetId world) {
  return rstl::string(CBasics::Stringize("WorldDefault: %8.8x", world));
}

rstl::string CInGameTweakManager::GetIdentifierForMidiEvent(CAssetId world, CAssetId area,
                                                            const rstl::string& name) {
  // The retail formatter swaps the world and area IDs.
  return rstl::string(
      CBasics::Stringize("World %8.8x Area %8.8x MidiObject: %s", area, world, name.c_str()));
}

rstl::string CInGameTweakManager::GetIdentifierForMusicEvent(CAssetId area,
                                                             const rstl::string& name) {
  return rstl::string(CBasics::Stringize("Area %8.8x MusicObject: %s", area, name.c_str()));
}

rstl::vector< CTweakValue > CInGameTweakManager::GetTweaksInWorld(CAssetId world) const {
  rstl::vector< CTweakValue > values;
  values.reserve(8);
  const rstl::istring prefix(CBasics::Stringize("World %8.8x", world));

  for (int i = 0; i < mValues.size(); ++i) {
    const CTweakValue& value = mValues[i];
    if (rstl::istring(value.GetName().c_str()).find_first_of(prefix) != -1) {
      values.reserve(values.size() + 1);
      values.push_back(value);
    }
  }

  return values;
}

rstl::vector< CAssetId > CInGameTweakManager::GetSongAssetsInWorld(CAssetId world) const {
  const rstl::vector< CTweakValue > values = GetTweaksInWorld(world);
  rstl::vector< CAssetId > songs;
  songs.reserve(values.size());

  for (int i = 0; i < values.size(); ++i) {
    const CTweakValue& value = values[i];
    if (value.GetName().find_first_of(rstl::string_l("MidiObject")) != -1 &&
        value.GetAudio().GetResId() != kInvalidAssetId) {
      songs.push_back(value.GetAudio().GetResId());
    }
  }

  return songs;
}
