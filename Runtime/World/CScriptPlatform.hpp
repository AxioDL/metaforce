#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CHealthInfo.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CFluidPlane;

struct SRiders {
  TUniqueId x0_uid;
  float x4_decayTimer;
  zeus::CTransform x8_transform;

  SRiders(TUniqueId id, float decayTimer, const zeus::CTransform& xf)
  : x0_uid(id), x4_decayTimer(decayTimer), x8_transform(xf) {}
};

class CScriptPlatform : public CPhysicsActor {
  // u32 x254_;
  TUniqueId x258_currentWaypoint = kInvalidUniqueId;
  TUniqueId x25a_targetWaypoint = kInvalidUniqueId;
  float x25c_currentSpeed;
  float x260_moveDelay = 0.f;
  float x264_collisionRecoverDelay = 0.f;
  float x268_fadeInTime = 0.f;
  float x26c_fadeOutTime = 0.f;
  zeus::CVector3f x270_dragDelta;
  zeus::CQuaternion x27c_rotDelta;
  CHealthInfo x28c_initialHealth;
  CHealthInfo x294_health;
  CDamageVulnerability x29c_damageVuln;
  std::optional<TLockedToken<CCollidableOBBTreeGroupContainer>> x304_treeGroupContainer;
  std::unique_ptr<CCollidableOBBTreeGroup> x314_treeGroup;
  std::vector<SRiders> x318_riders;
  std::vector<SRiders> x328_slavesStatic;
  std::vector<SRiders> x338_slavesDynamic;
  float x348_xrayAlpha;
  u32 x34c_maxRainSplashes;
  u32 x350_rainGenRate;
  TUniqueId x354_boundsTrigger = kInvalidUniqueId;
  union {
    struct {
      bool x356_24_dead : 1;
      bool x356_25_controlledAnimation : 1;
      bool x356_26_detectCollision : 1;
      bool x356_27_squishedRider : 1;
      bool x356_28_rainSplashes : 1;
      bool x356_29_setXrayDrawFlags : 1;
      bool x356_30_disableXrayAlpha : 1;
      bool x356_31_xrayFog : 1;
    };
    u32 x356_dummy = 0;
  };

  void DragSlave(CStateManager& mgr, rstl::reserved_vector<u16, 1024>& draggedSet, CActor* actor,
                 const zeus::CVector3f& delta);
  void DragSlaves(CStateManager& mgr, rstl::reserved_vector<u16, 1024>& draggedSet, const zeus::CVector3f& delta);
  static void DecayRiders(std::vector<SRiders>& riders, float dt, CStateManager& mgr);
  static void MoveRiders(CStateManager& mgr, float dt, bool active, std::vector<SRiders>& riders,
                         std::vector<SRiders>& collidedRiders, const zeus::CTransform& oldXf,
                         const zeus::CTransform& newXf, const zeus::CVector3f& dragDelta,
                         const zeus::CQuaternion& rotDelta);
  static rstl::reserved_vector<TUniqueId, 1024> BuildNearListFromRiders(CStateManager& mgr,
                                                                        const std::vector<SRiders>& movedRiders);

public:
  CScriptPlatform(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                  CModelData&& mData, const CActorParameters& actParms, const zeus::CAABox& aabb, float speed,
                  bool detectCollision, float xrayAlpha, bool active, const CHealthInfo& hInfo,
                  const CDamageVulnerability& dVuln,
                  std::optional<TLockedToken<CCollidableOBBTreeGroupContainer>>  dcln, bool rainSplashes,
                  u32 maxRainSplashes, u32 rainGenRate);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreThink(float, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void Render(CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  zeus::CTransform GetPrimitiveTransform() const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  zeus::CAABox GetSortingBounds(const CStateManager& mgr) const override;
  bool IsRider(TUniqueId id) const;
  bool IsSlave(TUniqueId id) const;
  std::vector<SRiders>& GetStaticSlaves() { return x328_slavesStatic; }
  const std::vector<SRiders>& GetStaticSlaves() const { return x328_slavesStatic; }
  std::vector<SRiders>& GetDynamicSlaves() { return x338_slavesDynamic; }
  const std::vector<SRiders>& GetDynamicSlaves() const { return x338_slavesDynamic; }
  bool HasComplexCollision() const { return x314_treeGroup.operator bool(); }
  void BuildSlaveList(CStateManager&);
  static void AddRider(std::vector<SRiders>& riders, TUniqueId riderId, const CPhysicsActor* ridee, CStateManager& mgr);
  void AddSlave(TUniqueId, CStateManager&);
  TUniqueId GetNext(TUniqueId, CStateManager&);
  TUniqueId GetWaypoint(CStateManager&);

  const CDamageVulnerability* GetDamageVulnerability() const override { return &x29c_damageVuln; }
  void SetDamageVulnerability(const CDamageVulnerability& vuln) { x29c_damageVuln = vuln; }
  CHealthInfo* HealthInfo(CStateManager&) override { return &x294_health; }
  void SetControlledAnimation(bool controlled) { x356_25_controlledAnimation = controlled; }
  void SetDisableXRayAlpha(bool val) { x356_30_disableXrayAlpha = val; }
  void SetXRayFog(bool val) { x356_31_xrayFog = val; }

  virtual void SplashThink(const zeus::CAABox&, const CFluidPlane&, float, CStateManager&) const;
  virtual zeus::CQuaternion Move(float, CStateManager&);
};
} // namespace urde
