#ifndef _CINGAMETWEAKMANAGER
#define _CINGAMETWEAKMANAGER

#include "Kyoto/SObjectTag.hpp"
#include "types.h"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CTweakValue {
public:
  struct Audio {
  public:
    Audio(float fadeIn, float fadeOut, float vol, const rstl::string& fileName, uint handle)
    : mFadeIn(fadeIn)
    , mFadeOut(fadeOut)
    , mVolume(vol)
    , mFileName(fileName)
    , mRes(handle) {}

    float GetFadeIn() const { return mFadeIn; }
    float GetFadeOut() const { return mFadeOut; }
    float GetVolume() const { return mVolume; }
    const rstl::string& GetFileName() const { return mFileName; }
    const CAssetId& GetResId() const { return mRes; }
    // static Audio None() { return Audio(0.f, 0.f, 0.f, "", 0); }

  private:
    float mFadeIn;
    float mFadeOut;
    float mVolume;
    rstl::string mFileName;
    CAssetId mRes;
  };
  enum EType {};

  CTweakValue();
  // CTweakValue(const rstl::string&, EType, const Audio&);
  // CTweakValue(CTextInputStream&);
  // void PutTo(CTextOutStream&);
  const rstl::string& GetName() const { return mKey; }
  const rstl::string& GetValueAsString() const;
  void SetValueFromString(const rstl::string&);
  const Audio& GetAudio() const { return mAudio; }
  EType GetType() const { return mType; }

private:
  EType mType;
  rstl::string mKey;
  rstl::string mStr;
  Audio mAudio;
  union {
    uint mInt;
    float mFlt;
  };
};

NESTED_CHECK_SIZEOF(CTweakValue, Audio, 0x20)
CHECK_SIZEOF(CTweakValue, 0x48)

class CInGameTweakManager {
public:
  CInGameTweakManager();
  ~CInGameTweakManager() {}

  bool HasTweakValue(const rstl::string& name) const;
  const CTweakValue* GetTweakValue(const rstl::string& name) const;
  bool ReadFromMemoryCard(const rstl::string&);
  rstl::vector< CAssetId > GetSongAssetsInWorld(CAssetId world) const;
  rstl::vector< CTweakValue > GetTweaksInWorld(CAssetId world) const;

  static rstl::string GetIdentifierForWorldDefaultMusic(CAssetId world);
  static rstl::string GetIdentifierForMusicEvent(CAssetId, const rstl::string&);

  static rstl::string GetIdentifierForMidiEvent(CAssetId world, CAssetId area,
                                                const rstl::string& midiObj);

private:
  rstl::vector< CTweakValue > mValues;
};

CHECK_SIZEOF(CInGameTweakManager, 0x10)

extern CInGameTweakManager* gpTweakManager;

#endif // _CINGAMETWEAKMANAGER
