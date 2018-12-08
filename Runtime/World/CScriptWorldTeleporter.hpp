#pragma once

#include "CEntity.hpp"
#include "CAnimationParameters.hpp"
#include "zeus/CVector3f.hpp"

namespace urde {

class CScriptWorldTeleporter : public CEntity {
  enum class ETeleporterType { NoTransition, Elevator, Text };

  CAssetId x34_worldId;
  CAssetId x38_areaId;
  ETeleporterType x3c_type = ETeleporterType::NoTransition;
  union {
    struct {
      bool x40_24_upElevator : 1;
      bool x40_25_inTransition : 1;
      bool x40_27_fadeWhite : 1;
    };
    u8 _dummy = 0;
  };

  float x44_charFadeIn = 0.1f;
  float x48_charsPerSecond = 8.0f;
  float x4c_showDelay = 0.0f;
  CAnimationParameters x50_playerAnim;
  zeus::CVector3f x5c_playerScale;
  CAssetId x68_platformModel;
  zeus::CVector3f x6c_platformScale;
  CAssetId x78_backgroundModel;
  zeus::CVector3f x7c_backgroundScale;
  u16 x88_soundId = -1;
  u8 x8a_volume = 0;
  u8 x8b_panning = 0;
  CAssetId x8c_fontId;
  CAssetId x90_stringId;

public:
  CScriptWorldTeleporter(TUniqueId, std::string_view, const CEntityInfo&, bool, CAssetId, CAssetId);
  CScriptWorldTeleporter(TUniqueId, std::string_view, const CEntityInfo&, bool, CAssetId, CAssetId, u16, u8, u8,
                         CAssetId, CAssetId, bool, float, float, float);
  CScriptWorldTeleporter(TUniqueId, std::string_view, const CEntityInfo&, bool, CAssetId, CAssetId, CAssetId, u32, u32,
                         const zeus::CVector3f&, CAssetId, const zeus::CVector3f&, CAssetId, const zeus::CVector3f&,
                         bool, u16, u8, u8);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
  void StartTransition(CStateManager&);
};
} // namespace urde
