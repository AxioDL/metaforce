#pragma once

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <zeus/CFrustum.hpp>

#include "Runtime/rstl.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"

namespace metaforce {
class CCollisionPrimitive;
class CCollisionInfoList;
class CCollisionInfo;

class CNewFlameThrower : public CGameProjectile {
  struct Contact {
    Contact(float contact, u32 remainingTime) : contact(contact), remainingTime(remainingTime) {}
    float contact;
    u32 remainingTime;
    bool operator<(Contact const& o) const { return contact < o.contact; }
  };
  struct Cube {
    Cube(zeus::CVector3f center, float bounds) : center(center), bounds(bounds) {}
    zeus::CVector3f center;
    float bounds;
  };
  enum class EFlameState {
    Default,
    FireStart,
    FireActive,
    FireStopTimer,
    FireWaitForParticlesDone
  };

  CRandom16 x2e8_rand{99};
  float x2ec_particlesDoneTimer = 0.f;
  float x2f0_flamesDoneTimer = 0.f;
  zeus::CVector3f x2f4_lastParticleCollisionLoc;
  bool x300_wasPointAdded = false;
  TCachedToken<CGenDescription> x304_mainFire;
  TCachedToken<CGenDescription> x310_mainSmoke;
  TCachedToken<CGenDescription> x31c_secondarySmoke;
  TCachedToken<CGenDescription> x328_secondaryFire;
  TCachedToken<CGenDescription> x334_secondarySparks;
  TCachedToken<CSwooshDescription> x340_swooshCenter;
  TCachedToken<CSwooshDescription> x34c_swooshFire;
  std::unique_ptr<CElementGen> x358_mainFireGen;
  std::unique_ptr<CElementGen> x35c_mainSmokeGen;
  std::unique_ptr<CElementGen> x360_secondarySmokeGen;
  std::unique_ptr<CElementGen> x364_secondaryFireGen;
  std::unique_ptr<CElementGen> x368_secondarySparksGen;
  std::unique_ptr<CParticleSwoosh> x36c_swooshCenterGen;
  std::unique_ptr<CParticleSwoosh> x370_swooshFireGen;
  EFlameState x374_flameState = EFlameState::Default;
  TAreaId x378_currentLitArea = kInvalidAreaId;
  bool x37c_24_renderAuxEffects : 1 = false;
  bool x37c_25_firing : 1 = false;
  bool x37c_26_runningSlowish : 1 = false;
  bool x37c_27_newPointAdded : 1 = true;
  bool x37c_28_activeLighting : 1 = false;
  rstl::reserved_vector<std::vector<Contact>, 3> x380_flameContactPoints;
  int x3b4_numSmokeParticlesSpawned = 0;
  rstl::reserved_vector<TUniqueId, 4> x3b8_lightIds;

  // std::array<std::unique_ptr<CTexturedQuadFilter>, 5> beam_filters;

  void DeleteLightObjects(CStateManager& mgr);
  void CreateLightObjects(CStateManager& mgr);
  void EnableFx(CStateManager& mgr);
  void UpdateLights(CStateManager& mgr);
  bool UpdateParticleCollisions(float dt, CStateManager &mgr,
		rstl::reserved_vector<Cube, 32> &collisions_out);
  bool CanDamageActor(CActor &hit_actor, CStateManager &mgr);
  void AddContactPoint(CCollisionInfo const& cinfo, u32 time);
  int SortAndFindOverlappingPoints(Cube const& box);
  bool FindCollisionInNearList(CStateManager &mgr, rstl::reserved_vector<TUniqueId, kMaxEntities> const &near_list,
                               CCollisionPrimitive const& coll, TUniqueId &first_coll_out,
                               CCollisionInfoList& collisions);
  void DecrementContactPointTimers();
  void SetLightsActive(CStateManager &mgr, bool active);
  void UpdateFlameState(float dt, CStateManager &mgr);
  void SetWorldLighting(CStateManager &mgr, TAreaId area, float speed, float target);
  // void RenderParticles(std::array<CElementGen *, 5> const& elem_gens);

  // void LoadParticleGenQuads();
  bool loaded_textures = false;

public:
  // Resinfo:
  //  NFTMainFire
  //  NFTMainSmoke
  //  NFTSwooshCenter
  //  NFTSwooshFire
  //  NFTSecondarySmoke
  //  NFTSecondaryFire
  //  NFTSecondarySparks
  //  <invalid>
  DEFINE_ENTITY
  CNewFlameThrower(const TToken<CWeaponDescription>& desc, std::string_view name, EWeaponType wType,
                   const std::array<CAssetId, 8>& resInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                   const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner, EProjectileAttrib attribs);
  void StartFiring(const zeus::CTransform& xf, CStateManager& mgr);
  bool CanRenderAuxEffects() const { return x37c_24_renderAuxEffects; }
  bool IsFiring() const { return x37c_25_firing; }
  bool AreEffectsFinished() const;
  void UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr);
  void Reset(CStateManager& mgr, bool deactivate);
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager &mgr) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override { return {}; }
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager &mgr) override;
  void AddToRenderer(zeus::CFrustum const& planes, CStateManager& mgr) override;
};

} // namespace metaforce
