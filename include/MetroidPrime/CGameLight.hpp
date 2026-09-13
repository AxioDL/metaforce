#ifndef _CGAMELIGHT
#define _CGAMELIGHT

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Graphics/CLight.hpp"

class CGameLight : public CActor {
public:
  CGameLight(const TUniqueId uid, const TAreaId aid, const bool active, const rstl::string& name,
             const CTransform4f& xf, const TUniqueId parentId, const CLight& light,
             const CAssetId sourceId, const uint priority, const float lifeTime);
  ~CGameLight();

  void SetLight(const CLight& light);
  CLight GetLight() const;
  TUniqueId GetParentId() const { return xe8_parentId; }
  void Think(float dt, CStateManager& mgr) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void SetLightPriorityAndId();

private:
  TUniqueId xe8_parentId;
  CLight xec_light;
  uint x13c_lightId;
  uint x140_priority;
  float x144_lifeTime;
};
CHECK_SIZEOF(CGameLight, 0x148)

#endif // _CGAMELIGHT
