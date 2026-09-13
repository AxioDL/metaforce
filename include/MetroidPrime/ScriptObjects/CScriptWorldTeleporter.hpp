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

  bool GetFadeWhite() const { return x40_27_fadeWhite; }

private:
  enum ETeleporterType { kTT_NoTransition, kTT_Elevator, kTT_Text };

  CAssetId x34_worldId;
  CAssetId x38_areaId;
  ETeleporterType x3c_type;     // = ETeleporterType::NoTransition;
  bool x40_24_upElevator : 1;   // = false;
  bool x40_25_inTransition : 1; // = false;
  bool x40_26_ : 1;
  bool x40_27_fadeWhite : 1; // = false;
  float x44_charFadeIn;      // = 0.1f;
  float x48_charsPerSecond;  // = 8.0f;
  float x4c_showDelay;       // = 0.0f;
  CAnimationParameters x50_playerAnim;
  CVector3f x5c_playerScale;
  CAssetId x68_platformModel;
  CVector3f x6c_platformScale;
  CAssetId x78_backgroundModel;
  CVector3f x7c_backgroundScale;
  ushort x88_soundId; // = -1;
  uchar x8a_volume;   // = 0;
  uchar x8b_panning;  // = 0;
  CAssetId x8c_fontId;
  CAssetId x90_stringId;
};

#endif // _CSCRIPTWORLDTELEPORTER
