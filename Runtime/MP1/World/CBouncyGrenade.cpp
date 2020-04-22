#include "Runtime/MP1/World/CBouncyGrenade.hpp"

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"

namespace urde::MP1 {
CBouncyGrenade::CBouncyGrenade(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                               const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& actParams,
                               TUniqueId parentId, const SBouncyGrenadeData& data, float velocity,
                               float explodePlayerDistance)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData), {EMaterialTypes::Projectile, EMaterialTypes::Solid},
                mData.GetBounds(), SMoverData{data.GetUnkStruct().GetMass()}, actParams, 0.3f, 0.1f)
, x258_data(data)
, x294_numBounces(data.GetNumBounces())
, x298_parentId(parentId)
, x2a0_elementGen1(std::make_unique<CElementGen>(g_SimplePool->GetObj({'PART', data.GetElementGenId1()})))
, x2a4_elementGen2(std::make_unique<CElementGen>(g_SimplePool->GetObj({'PART', data.GetElementGenId2()})))
, x2a8_elementGen3(std::make_unique<CElementGen>(g_SimplePool->GetObj({'PART', data.GetElementGenId3()})))
, x2ac_elementGen4(std::make_unique<CElementGen>(g_SimplePool->GetObj({'PART', data.GetElementGenId4()})))
, x2b0_explodePlayerDistance(explodePlayerDistance) {
  SetMomentumWR({0.f, 0.f, -GravityConstant() * GetMass()});
  SetVelocityWR(velocity * xf.frontVector());
  x2a0_elementGen1->SetParticleEmission(false);
  x2a4_elementGen2->SetParticleEmission(false);
  x2a8_elementGen3->SetParticleEmission(false);
  x2ac_elementGen4->SetParticleEmission(true);
  CMaterialFilter filter = GetMaterialFilter();
  filter.ExcludeList().Add(EMaterialTypes::Character);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(filter.IncludeList(), filter.ExcludeList()));
}

void CBouncyGrenade::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CActor::AddToRenderer(frustum, mgr);
  if (!x2b4_24_exploded) {
    g_Renderer->AddParticleGen(*x2ac_elementGen4);
    return;
  }
  const auto visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  if (visor == CPlayerState::EPlayerVisor::Combat || visor == CPlayerState::EPlayerVisor::Scan) {
    g_Renderer->AddParticleGen(*x2a0_elementGen1);
  } else if (visor == CPlayerState::EPlayerVisor::XRay || visor == CPlayerState::EPlayerVisor::Thermal) {
    g_Renderer->AddParticleGen(*x2a8_elementGen3);
  }
}

void CBouncyGrenade::CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr) {
  constexpr auto matList = CMaterialList{
      EMaterialTypes::Solid,
      EMaterialTypes::Ceiling,
      EMaterialTypes::Floor,
      EMaterialTypes::Character,
  };

  bool shouldExplode = false;
  if (x298_parentId != id) {
    const CEntity* const entity = mgr.GetObjectById(id);
    if (entity != nullptr) {
      if (TCastToConstPtr<CCollisionActor> actor = entity) {
        shouldExplode = actor->GetOwnerId() != x298_parentId;
      } else {
        shouldExplode = true;
      }
    }
  }
  if (shouldExplode) {
    Explode(mgr, id);
  } else {
    for (const auto& info : list) {
      if (info.GetMaterialLeft().SharesMaterials(matList)) {
        if (x294_numBounces == 0) {
          Explode(mgr, kInvalidUniqueId);
        } else {
          const zeus::CVector3f* normal = &info.GetNormalLeft();
          if (GetVelocity().dot(info.GetNormalLeft()) > 0.f) {
            normal = &info.GetNormalRight();
          }
          const zeus::CVector3f impulse =
              (x258_data.GetUnkStruct().GetSpeed() * GetConstantForce().magnitude()) * *normal;
          const zeus::CVector3f angle = -x258_data.GetUnkStruct().GetSpeed() * GetAngularMomentum();
          ApplyImpulseWR(impulse, angle);
          CSfxManager::AddEmitter(x258_data.GetBounceSfx(), GetTranslation(), zeus::skUp, false, false, 0x7f,
                                  GetAreaIdAlways());
          --x294_numBounces;
        }
        break;
      }
    }
  }
  CPhysicsActor::CollidedWith(id, list, mgr);
}

std::optional<zeus::CAABox> CBouncyGrenade::GetTouchBounds() const { return GetModelData()->GetBounds(GetTransform()); }

void CBouncyGrenade::Render(CStateManager& mgr) {
  if (!x2b4_24_exploded) {
    GetModelData()->Render(mgr, GetTransform(), nullptr, {0, 0, 3, zeus::skWhite});
  } else if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
    CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 75.f, zeus::skBlack);
    x2a4_elementGen2->Render();
    mgr.SetupFogForArea(GetAreaIdAlways());
  }
}

void CBouncyGrenade::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    const zeus::CTransform& orientation = GetTransform().getRotation();
    const zeus::CVector3f& translation = GetTranslation();
    const zeus::CVector3f& scale = GetModelData()->GetScale();
    auto UpdateElementGen = [ orientation, translation, scale, dt ](CElementGen & gen) constexpr {
      gen.SetOrientation(orientation);
      gen.SetGlobalTranslation(translation);
      gen.SetGlobalScale(scale);
      gen.Update(dt);
    };
    if (x2b4_24_exploded) {
      Stop();
      UpdateElementGen(*x2a0_elementGen1);
      UpdateElementGen(*x2a4_elementGen2);
      UpdateElementGen(*x2a8_elementGen3);
    } else {
      UpdateElementGen(*x2ac_elementGen4);
    }
    x29c_ += dt;
    if (x29c_ > 0.3f) {
      x2b4_25_ = true;
    }
    const zeus::CVector3f& playerDistance = mgr.GetPlayer().GetTranslation() +
                                            zeus::CVector3f{0.f, 0.f, 0.5f * mgr.GetPlayer().GetEyeHeight()} -
                                            translation;
    if (playerDistance.magSquared() < x2b0_explodePlayerDistance * x2b0_explodePlayerDistance) {
      Explode(mgr, kInvalidUniqueId);
    }
  }
  if (x2a0_elementGen1->IsSystemDeletable() && x2a4_elementGen2->IsSystemDeletable() &&
      x2a8_elementGen3->IsSystemDeletable()) {
    mgr.FreeScriptObject(GetUniqueId());
  }
}

void CBouncyGrenade::Touch(CActor& act, CStateManager& mgr) { CActor::Touch(act, mgr); }

void CBouncyGrenade::Explode(CStateManager& mgr, TUniqueId uid) {
  if (x2b4_24_exploded) {
    return;
  }

  x2b4_24_exploded = true;
  CSfxManager::AddEmitter(x258_data.GetExplodeSfx(), GetTranslation(), zeus::skUp, false, false, 0x7f,
                          GetAreaIdAlways());
  x2a0_elementGen1->SetParticleEmission(true);
  x2a4_elementGen2->SetParticleEmission(true);
  x2a8_elementGen3->SetParticleEmission(true);
  x2ac_elementGen4->SetParticleEmission(false);

  const CDamageInfo& dInfo = x258_data.GetDamageInfo();
  {
    bool isParent = x298_parentId == uid;
    if (TCastToConstPtr<CCollisionActor> actor = mgr.GetObjectById(uid)) {
      isParent = x298_parentId == actor->GetOwnerId();
    }
    if (uid != kInvalidUniqueId && !isParent) {
      mgr.ApplyDamage(GetUniqueId(), uid, GetUniqueId(), dInfo, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}),
                      zeus::skZero3f);
    }
  }

  const float radius = dInfo.GetRadius();
  if (radius > 1.f) {
    const zeus::CVector3f& pos = GetTranslation();
    const CMaterialFilter filter = CMaterialFilter::MakeInclude({EMaterialTypes::Player, EMaterialTypes::Character});
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, {pos - radius, pos + radius}, filter, nullptr);

    for (const auto& id : nearList) {
      bool isParent = x298_parentId == id;
      if (TCastToConstPtr<CCollisionActor> cActor = mgr.GetObjectById(id)) {
        isParent = x298_parentId == cActor->GetOwnerId();
      }
      if (isParent) {
        continue;
      }

      const auto* actor = static_cast<const CActor*>(mgr.GetObjectById(id));
      if (actor == nullptr) {
        continue;
      }

      const float magnitude = (actor->GetTranslation() - GetTranslation()).magnitude();
      if (radius <= magnitude) {
        continue;
      }

      float scale = (radius - magnitude) / radius;
      const CDamageInfo info{dInfo.GetWeaponMode(), scale * dInfo.GetDamage(), radius,
                             scale * dInfo.GetKnockBackPower()};
      mgr.ApplyDamage(GetUniqueId(), id, GetUniqueId(), info, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}),
                      zeus::skZero3f);
    }
  }
}
} // namespace urde::MP1
