#pragma once

#include "CEntity.hpp"
#include "zeus/CTransform.hpp"

namespace urde {
struct SRiders;
class CScriptPlatform;
class CScriptActorRotate : public CEntity {
  zeus::CVector3f x34_rotation;
  float x40_maxTime;
  float x44_currentTime = 0.f;
  std::map<TUniqueId, zeus::CTransform> x48_actors;

  union {
    struct {
      bool x58_24_updateRotation : 1;
      bool x58_25_skipSpiderBallWaypoints : 1;
      bool x58_26_updateActors : 1;
      bool x58_27_updateOnCreation : 1;
    };
    u32 dummy = 0;
  };

  void UpdateActors(bool, CStateManager&);
  void UpdateSpiderBallWaypoints(CStateManager&);
  void UpdatePlatformRiders(CScriptPlatform&, const zeus::CTransform&, CStateManager&);
  void UpdatePlatformRiders(std::vector<SRiders>&, CScriptPlatform&, const zeus::CTransform&, CStateManager&);

public:
  CScriptActorRotate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CVector3f&, float, bool, bool, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
};

} // namespace urde
