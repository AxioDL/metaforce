#ifndef _CSCRIPTWORLDTELEPORTER
#define _CSCRIPTWORLDTELEPORTER

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CAnimationParameters.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CScriptWorldTeleporter : public CEntity {
public:
  CScriptWorldTeleporter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         bool active, CAssetId worldId, CAssetId areaId);
  CScriptWorldTeleporter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         bool active, CAssetId worldId, CAssetId areaId, CAssetId playerAncs,
                         uint charIdx, uint defaultAnim, const CVector3f& playerScale,
                         CAssetId platformModel, const CVector3f& platformScale,
                         CAssetId backgroundModel, const CVector3f& backgroundScale,
                         bool upElevator, ushort soundId, uchar volume, uchar panning);
  CScriptWorldTeleporter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         bool active, CAssetId worldId, CAssetId areaId, int soundId,
                         uchar volume, uchar panning, CAssetId fontId, CAssetId stringId,
                         bool fadeWhite, float charFadeIn, float charsPerSecond, float showDelay);
  ~CScriptWorldTeleporter();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void StartTransition(CStateManager&);

  bool GetFadeWhite() const { return mFadeWhite; }

private:
  enum ETeleporterType { kTT_NoTransition, kTT_Elevator, kTT_Text };

  CAssetId mWorldId;
  CAssetId mAreaId;
  ETeleporterType mType;     // = ETeleporterType::NoTransition;
  bool mUpElevator : 1;   // = false;
  bool mInTransition : 1; // = false;
  bool x40_26_ : 1;
  bool mFadeWhite : 1; // = false;
  float mCharFadeIn;      // = 0.1f;
  float mCharsPerSecond;  // = 8.0f;
  float mShowDelay;       // = 0.0f;
  CAnimationParameters mPlayerAnim;
  CVector3f mPlayerScale;
  CAssetId mPlatformModel;
  CVector3f mPlatformScale;
  CAssetId mBackgroundModel;
  CVector3f mBackgroundScale;
  ushort mSoundId; // = -1;
  uchar mVolume;   // = 0;
  uchar mPanning;  // = 0;
  CAssetId mFontId;
  CAssetId mStringId;
};

#endif // _CSCRIPTWORLDTELEPORTER
