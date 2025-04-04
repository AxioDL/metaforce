#include "Runtime/Weapon/CNewFlameThrower.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionInfoList.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"
#include "TCastTo.hpp"
#include "Runtime/Collision/CCollisionPrimitive.hpp"

#include "Runtime/World/CPatterned.hpp"
#include "Runtime/MP1/World/CFlickerBat.hpp"
#include "Runtime/World/CSnakeWeedSwarm.hpp"
#include "Runtime/Collision/CMetroidAreaCollider.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"
#include "Runtime/World/CScriptTrigger.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"

namespace metaforce {
namespace {
constexpr CMaterialFilter skExcludeProjectilePassthrough =
    CMaterialFilter::MakeExclude(EMaterialTypes::ProjectilePassthrough);
}

CNewFlameThrower::CNewFlameThrower(const TToken<CWeaponDescription>& desc, std::string_view name, EWeaponType wType,
                                   const std::array<CAssetId, 8>& resInfo, const zeus::CTransform& xf,
                                   EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
                                   TUniqueId owner, EProjectileAttrib attribs)
: CGameProjectile(false, desc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId, attribs, false,
                  zeus::skOne3f, {}, -1, false)
, x304_mainFire(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[0]}))
, x310_mainSmoke(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[1]}))
, x31c_secondarySmoke(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[4]}))
, x328_secondaryFire(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[5]}))
, x334_secondarySparks(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), resInfo[6]}))
, x340_swooshCenter(g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), resInfo[2]}))
, x34c_swooshFire(g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), resInfo[3]})) {
  x304_mainFire.GetObj();
  x310_mainSmoke.GetObj();
  x31c_secondarySmoke.GetObj();
  x328_secondaryFire.GetObj();
  x334_secondarySparks.GetObj();
  x340_swooshCenter.GetObj();
  x34c_swooshFire.GetObj();
  x380_flameContactPoints.resize(3);
}

void CNewFlameThrower::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);

  TAreaId cur_area_id = mgr.GetWorld()->GetCurrentAreaId();
  mgr.SetActorAreaId(*this, cur_area_id);

  for (TUniqueId& light_id : x3b8_lightIds) {
    CEntity* light = mgr.ObjectById(light_id);
    if (light == nullptr) {
      light_id = kInvalidUniqueId;
    } else {
      mgr.SetActorAreaId(*reinterpret_cast<CActor*>(light), cur_area_id);
    }
  }
}

void CNewFlameThrower::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CNewFlameThrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Deleted) {
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    DeleteLightObjects(mgr);
    SetWorldLighting(mgr, mgr.GetPlayer().GetAreaIdAlways(), 4.f, 1.f);
  } else if (msg == EScriptObjectMessage::Registered) {
    xe6_27_thermalVisorFlags = 2; // Thermal hot
    Think(1.f / 60.f, mgr);
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
  }

  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

void CNewFlameThrower::DeleteLightObjects(CStateManager& mgr) {
  for (TUniqueId const& id : x3b8_lightIds) {
    mgr.FreeScriptObject(id);
  }
  x3b8_lightIds.clear();
}

void CNewFlameThrower::AddToRenderer(zeus::CFrustum const& planes, CStateManager& mgr) {
  zeus::CAABox sorting_bounds = GetSortingBounds(mgr);
  EnsureRendered(mgr, x34_transform.origin, sorting_bounds);
}

void CNewFlameThrower::CreateLightObjects(CStateManager& mgr) {
  DeleteLightObjects(mgr);
  for (int i = 0; i < 4; ++i) {
    TUniqueId uid = mgr.AllocateUniqueId();
    CLight lObj = x358_mainFireGen->GetLight();
    CGameLight* light = new CGameLight(uid, GetAreaId(), false, "FlamethrowerLight", zeus::CTransform(), x8_uid, lObj,
                                       u32(reinterpret_cast<uintptr_t>(this) + (i & 0x1)), 0, 0.f);
    mgr.AddObject(light);
    x3b8_lightIds.push_back(uid);
  }
}

void CNewFlameThrower::EnableFx(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  x358_mainFireGen = std::make_unique<CElementGen>(x304_mainFire);
  x35c_mainSmokeGen = std::make_unique<CElementGen>(x310_mainSmoke);
  x360_secondarySmokeGen = std::make_unique<CElementGen>(x31c_secondarySmoke);
  x364_secondaryFireGen = std::make_unique<CElementGen>(x328_secondaryFire);
  x368_secondarySparksGen = std::make_unique<CElementGen>(x334_secondarySparks);
  x36c_swooshCenterGen = std::make_unique<CParticleSwoosh>(x340_swooshCenter, 0);
  x36c_swooshCenterGen->SetRenderGaps(true);
  x370_swooshFireGen = std::make_unique<CParticleSwoosh>(x34c_swooshFire, 0);
  x370_swooshFireGen->SetRenderGaps(true);
  if (x358_mainFireGen && x358_mainFireGen->SystemHasLight() && x3b8_lightIds.empty())
    CreateLightObjects(mgr);
}

void CNewFlameThrower::StartFiring(const zeus::CTransform& xf, CStateManager& mgr) {
  SetActive(true);
  x37c_25_firing = true;
  x37c_24_renderAuxEffects = true;
  x374_flameState = EFlameState::FireStart;
  EnableFx(mgr);
}

bool CNewFlameThrower::AreEffectsFinished() const {
  if (x358_mainFireGen && x358_mainFireGen->GetParticleCount() != 0)
    return false;
  if (x35c_mainSmokeGen && x35c_mainSmokeGen->GetParticleCount() != 0)
    return false;
  if (x360_secondarySmokeGen && x360_secondarySmokeGen->GetParticleCount() != 0)
    return false;
  if (x364_secondaryFireGen && x364_secondaryFireGen->GetParticleCount() != 0)
    return false;
  return !(x368_secondarySparksGen && x368_secondarySparksGen->GetParticleCount() != 0);
}

/* Used for old CNewFlameThrower::RenderParticles

void CNewFlameThrower::LoadParticleGenQuads() {
  if (!loaded_textures) {
    beam_filters[0] = std::make_unique<CTexturedQuadFilter>(
        EFilterType::Add, x358_mainFireGen->GetLoadedDesc()->x54_x40_TEXR->GetValueTexture(0));
    beam_filters[1] = std::make_unique<CTexturedQuadFilter>(
        EFilterType::Add, x35c_mainSmokeGen->GetLoadedDesc()->x54_x40_TEXR->GetValueTexture(0));
    beam_filters[2] = std::make_unique<CTexturedQuadFilter>(
        EFilterType::Add, x360_secondarySmokeGen->GetLoadedDesc()->x54_x40_TEXR->GetValueTexture(0));
    beam_filters[3] = std::make_unique<CTexturedQuadFilter>(
        EFilterType::Add, x364_secondaryFireGen->GetLoadedDesc()->x54_x40_TEXR->GetValueTexture(0));
    beam_filters[4] = std::make_unique<CTexturedQuadFilter>(
        EFilterType::Add, x368_secondarySparksGen->GetLoadedDesc()->x54_x40_TEXR->GetValueTexture(0));
    loaded_textures = true;
  }
}
*/

void CNewFlameThrower::Render(CStateManager& mgr) {
  if (x30_24_active) {
    x36c_swooshCenterGen->Render();
    x370_swooshFireGen->Render();

    x368_secondarySparksGen->Render();
    x364_secondaryFireGen->Render();
    x360_secondarySmokeGen->Render();
    x35c_mainSmokeGen->Render();
    x358_mainFireGen->Render();

    /*RenderBeam({x358_mainFireGen.get(), x35c_mainSmokeGen.get(), x360_secondarySmokeGen.get(),
                x364_secondaryFireGen.get(), x368_secondarySparksGen.get()});*/
  }
}

/* Removed this, source function was painfully similar to CElementGen::RenderParticles

void CNewFlameThrower::RenderParticles(std::array<CElementGen*, 5> const& elem_gens) {
  LoadParticleGenQuads();
  zeus::CTransform xf(CGraphics::g_ViewMatrix);
  zeus::CTransform xf2 = xf;
  xf2.origin = zeus::skZero3f;
  zeus::CTransform xf3 = xf2.inverse();
  zeus::CTransform xf4 = xf3;
  // CGraphics::SetModelMatrix(xf2);
  // CGraphics::SetCullMode(ERglCullMode::None);
  // CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
  // CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

  // TODO: check sMoveRedToAlphaBuffer

  // CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
  int total_particles = 0;
  for (CElementGen* elem : elem_gens) {
    total_particles += elem->GetParticleCount();
  }

  // This is... something isn't it
  struct ParticleElement {
    u16 elem_gen_idx;
    u16 part_idx;
    zeus::CVector3f vec;
  };

  auto* translated_sorted_particles =
      reinterpret_cast<ParticleElement*>(_alloca(sizeof(ParticleElement) * total_particles));
  int active_particle_count = 0;

  for (int i = 0; i < elem_gens.size(); i++) {
    CElementGen* elem = elem_gens[i];

    int iter_count = elem->GetParticleCount();
    for (int j = 0; j < iter_count; j++) {
      CParticle& part = elem->GetParticles()[j];
      if (part.x0_endFrame == -1) {
        continue;
      }

      // ????
      // translated_sorted_particles[part_idx].vec = xf4 * part.x4_pos;
      translated_sorted_particles[active_particle_count].vec =
          xf4 * (((part.x4_pos - part.x10_prevPos) * elem->GetTimeDeltaScale()) + part.x10_prevPos);
      translated_sorted_particles[active_particle_count].elem_gen_idx = static_cast<u16>(i);
      translated_sorted_particles[active_particle_count].part_idx = static_cast<u16>(j);

      active_particle_count++;
    }
  }
  Log.report(logvisor::Info, "Active particle count (render count) {}", active_particle_count);
  std::sort(translated_sorted_particles, translated_sorted_particles + active_particle_count,
            [](ParticleElement const& l, ParticleElement const& r) { return l.vec.y() > r.vec.y(); });

  int last_gen_idx = 0xffff;
  CElementGen* cur_gen = nullptr;
  CGenDescription* gen_desc = nullptr;
  u32 emitter_time = 0;

  for (int i = 0; i < active_particle_count; i++) {
    ParticleElement* pe = translated_sorted_particles + i;
    if (pe->elem_gen_idx != last_gen_idx) {
      cur_gen = elem_gens[pe->elem_gen_idx];

      emitter_time = cur_gen->GetEmitterTime();
      gen_desc = cur_gen->GetLoadedDesc();
      if (CElementGen::sMoveRedToAlphaBuffer) {
        // TEV stuff
      } else {
        if (gen_desc->x44_26_x30_26_AAPH) {
          // CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One,
          //                         ERglLogicOp::Clear);
        } else {
          // CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
          //                         ERglLogicOp::Clear);
        }
      }
      // more TEV stuff
      last_gen_idx = pe->elem_gen_idx;
    }
    CParticle& part = cur_gen->GetParticles()[pe->part_idx];

    u32 elapsed_time = emitter_time - part.x28_startFrame - 1;
    CParticleGlobals::instance()->SetParticleLifetime(part.x0_endFrame - part.x28_startFrame);
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(elapsed_time);
    SUVElementSet uvs;

    xf3 = xf2.inverse();
    zeus::CTransform system_camera_matrix = xf3 * cur_gen->x22c_globalOrientation;
    xf3 = ((zeus::CTransform::Translate(cur_gen->xe8_globalTranslation) * cur_gen->x10c_globalScaleTransform) * xf3) *
          cur_gen->x178_localScaleTransform;
    g_Renderer->SetModelMatrix(xf3);

    gen_desc->x54_x40_TEXR->GetValueUV(elapsed_time, uvs);
    float ang = zeus::degToRad(part.x30_lineWidthOrRota);
    float size = part.x2c_lineLengthOrSize * 0.5f;
    float sin_extent = sinf(ang) * size;
    float cos_extent = cosf(ang) * size;
    std::array<CTexturedQuadFilter::Vert, 4> vertices;
    zeus::CVector3f simd_vec(sin_extent + cos_extent, 0, cos_extent - sin_extent);
    vertices[0].m_pos = pe->vec + zeus::CVector3f(sin_extent + cos_extent, 0, cos_extent - sin_extent);
    vertices[0].m_uv = zeus::CVector2f(uvs.xMax, uvs.yMax);

    vertices[1].m_pos = pe->vec - zeus::CVector3f(sin_extent - cos_extent, 0, sin_extent + cos_extent);
    vertices[1].m_uv = zeus::CVector2f(uvs.xMin, uvs.yMax);

    vertices[2].m_pos = pe->vec - zeus::CVector3f(sin_extent + cos_extent, 0, cos_extent - sin_extent);
    vertices[2].m_uv = zeus::CVector2f(uvs.xMin, uvs.yMin);

    vertices[3].m_pos = pe->vec + zeus::CVector3f(sin_extent - cos_extent, 0, sin_extent + cos_extent);
    vertices[3].m_uv = zeus::CVector2f(uvs.xMin, uvs.yMax);

    beam_filters[pe->elem_gen_idx]->drawVerts(part.x34_color, vertices);
  }
}
*/

void CNewFlameThrower::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {
  if (!x30_24_active) {
    return;
  }
  float active_time =
      0; // g_Main.x118_avgTickTimePerSec + g_Main.x11c_avgDrawTimePerSec; omitting this until handled by URDE
  x37c_26_runningSlowish = (active_time > 0.65f);
  UpdateFlameState(dt, mgr);
  zeus::CVector3f org = xf.origin;
  zeus::CTransform rot = xf.getRotation();
  zeus::CTransform rot_copy(rot);

  x358_mainFireGen->SetTranslation(org);
  x358_mainFireGen->SetOrientation(rot_copy);
  x36c_swooshCenterGen->SetTranslation(org);
  x36c_swooshCenterGen->SetOrientation(rot_copy);
  x370_swooshFireGen->SetTranslation(org);
  x370_swooshFireGen->SetOrientation(rot_copy);

  float particle_rate = (x37c_26_runningSlowish ? 1.f : 0.5f);
  x358_mainFireGen->SetGeneratorRate(particle_rate);
  x358_mainFireGen->Update(dt);
  x35c_mainSmokeGen->Update(dt);
  x360_secondarySmokeGen->Update(dt);
  x364_secondaryFireGen->Update(dt);
  x368_secondarySparksGen->Update(dt);
  x36c_swooshCenterGen->Update(dt);
  x370_swooshFireGen->Update(dt);
  rstl::reserved_vector<Cube, 32> collision_list;

  UpdateParticleCollisions(dt, mgr, collision_list);
  if (collision_list.size() > 0) {
    for (auto& swoosh : x36c_swooshCenterGen->GetSwooshes()) {
      for (auto& cube : collision_list) {
        float dpos = (cube.center - swoosh.xc_translation).magSquared();
        if (dpos < (cube.bounds * cube.bounds)) {
          swoosh.x0_active = false;
        }
      }
    }
    for (auto& particle : x358_mainFireGen->GetParticles()) {
      for (auto& cube : collision_list) {
        float dpos = (cube.center - particle.x4_pos).magSquared();
        if (dpos < (cube.bounds * cube.bounds)) {
          particle.x0_endFrame = -1;
        }
      }
    }
    for (auto& particle : x35c_mainSmokeGen->GetParticles()) {
      for (auto& cube : collision_list) {
        float dpos = (cube.center - particle.x4_pos).magSquared();
        if (dpos < (cube.bounds * cube.bounds)) {
          particle.x0_endFrame = -1;
        }
      }
    }
  }
  if (x374_flameState == EFlameState::FireActive) {
    int free_space = x36c_swooshCenterGen->GetSwooshes().capacity() - x36c_swooshCenterGen->GetSwooshes().size();
    if (free_space < 4) {
      const int swoosh_count = static_cast<int>(x36c_swooshCenterGen->GetSwooshes().size());
      int quarter_behind_cur = ((((swoosh_count / 2) * 3) / 2) + x36c_swooshCenterGen->GetCurParticle()) % swoosh_count;
      if (x36c_swooshCenterGen->GetSwooshes()[quarter_behind_cur].x0_active) {
        // Need better names here
        int next_idx = (quarter_behind_cur + 1) % swoosh_count;
        auto& swoosh_1 = x36c_swooshCenterGen->GetSwooshes()[quarter_behind_cur];
        auto& swoosh_2 = x36c_swooshCenterGen->GetSwooshes()[next_idx];
        zeus::CVector3f delta = swoosh_1.xc_translation - swoosh_2.xc_translation;
        float f0 = delta.dot(swoosh_1.x38_orientation.frontVector());
        zeus::CVector3f unk = delta - (swoosh_1.x38_orientation.frontVector() * f0);
        float tmp = std::clamp(unk.magnitude() * 30.f, 1.f, x37c_26_runningSlowish ? 2.f : 4.f);

        x3b4_numSmokeParticlesSpawned = std::max(static_cast<int>(round(tmp)), x3b4_numSmokeParticlesSpawned - 1);

        x35c_mainSmokeGen->SetTranslation(swoosh_1.xc_translation);
        x35c_mainSmokeGen->SetOrientation(swoosh_1.x38_orientation);
        if (x3b4_numSmokeParticlesSpawned > 0) {
          x35c_mainSmokeGen->ForceParticleCreation(x3b4_numSmokeParticlesSpawned);
        }
      }
    }
  }
  UpdateLights(mgr);
}

// TODO: lights don't actually light
void CNewFlameThrower::UpdateLights(CStateManager& mgr) {
  CGlobalRandom rand(x2e8_rand);

  int lit_particle_step = std::max(2, static_cast<int>(x370_swooshFireGen->GetSwooshes().size() / 4));
  int prev_particle = (x370_swooshFireGen->GetCurParticle() - 1) % x370_swooshFireGen->GetSwooshes().size();

  int lit_particle_offset = 0;
  const int fire_swoosh_count = static_cast<int>(x370_swooshFireGen->GetSwooshes().size());
  for (auto& light_id : x3b8_lightIds) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(light_id)) {
      bool should_light = true;
      if (fire_swoosh_count <= lit_particle_offset) {
        should_light = false;
      }
      auto& light_swoosh = x370_swooshFireGen->GetSwooshes()[(lit_particle_offset + prev_particle) % fire_swoosh_count];
      if (!light_swoosh.x0_active) {
        should_light = false;
      }
      light->SetActive(should_light);
      if (!should_light) {
        lit_particle_offset += lit_particle_step;
        continue;
      }
      CLight light_data = x358_mainFireGen->GetLight();
      if (x304_mainFire.GetObj()->x104_xf0_LCLR.get()) {
        s32 rand_int = x2e8_rand.Range(0, 16);
        CParticleGlobals::instance()->SetEmitterTime(rand_int);
        zeus::CColor out_color(0xffff00ff);
        x304_mainFire.GetObj()->x104_xf0_LCLR->GetValue(rand_int, out_color);
        light_data.SetColor(out_color);
      }
      if (x304_mainFire.GetObj()->x108_xf4_LINT.get()) {
        s32 rand_int = x2e8_rand.Range(0, 16);
        CParticleGlobals::instance()->SetEmitterTime(rand_int);
        float out_const_attenuation = 1.f;
        x304_mainFire.GetObj()->x108_xf4_LINT->GetValue(rand_int, out_const_attenuation);
        light_data.SetAngleAttenuation(out_const_attenuation, 0.f, 0.f);
      }
      light->SetLight(light_data);
      light->SetTranslation(light_swoosh.xc_translation);
      lit_particle_offset += lit_particle_step;
    }
  }
}

bool CNewFlameThrower::UpdateParticleCollisions(float dt, CStateManager& mgr,
                                                rstl::reserved_vector<Cube, 32>& collisions_out) {
  x300_wasPointAdded = false;
  bool any_particle_collisions = false;
  EntityList near_list_cache;
  // rstl::reserved_vector<rstl::reserved_vector<, ?>, ?> unk_rstl_vec; // inner vectors of size 0x90c, never used
  // though
  CCollisionInfoList cached_cinfo;
  auto& swoosh_vec = x370_swooshFireGen->GetSwooshes();
  const int swoosh_count = static_cast<int>(swoosh_vec.size());
  const int tmp = swoosh_count / 4;
  const int batch_process_size = tmp < 6 ? 6 : tmp;
  const int prev_particle = ((swoosh_count + x370_swooshFireGen->GetCurParticle()) - 1) % swoosh_count;
  int i = 0;
  while (i < swoosh_count) {
    int batch_start = i;
    int batch_end = i + batch_process_size;
    if (batch_end >= swoosh_count) {
      batch_end = swoosh_count;
    }
    float batch_highest_speed = 0.f;
    bool processed_swoosh_in_batch = false;
    zeus::CAABox batch_particle_bounds;
    // Accumulate bounds of this batch and its max speed
    for (int j = batch_start; j < batch_end; j++) {
      if (!swoosh_vec[j].x0_active) {
        continue;
      }
      if (!processed_swoosh_in_batch) {
        // Retro assigned MIN_FLOAT and MAX_FLOAT, but this should suffice
        batch_particle_bounds = zeus::CAABox();
      }
      processed_swoosh_in_batch = true;
      batch_particle_bounds.accumulateBounds(swoosh_vec[j].xc_translation);
      float swoosh_speed_sq = swoosh_vec[j].x74_velocity.magSquared();
      if (batch_highest_speed < swoosh_speed_sq) {
        batch_highest_speed = swoosh_speed_sq;
      }
    }
    batch_highest_speed = sqrtf(batch_highest_speed) + 0.1f;
    batch_particle_bounds.accumulateBounds(batch_particle_bounds.min - zeus::CVector3f(batch_highest_speed));
    batch_particle_bounds.accumulateBounds(batch_particle_bounds.max + zeus::CVector3f(batch_highest_speed));

    near_list_cache.clear();

    CMaterialFilter near_list_filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(EMaterialTypes::Solid), CMaterialList(EMaterialTypes::ProjectilePassthrough));
    mgr.BuildNearList(near_list_cache, batch_particle_bounds, near_list_filter, mgr.Player());
    CAreaCollisionCache coll_cache(batch_particle_bounds);
    CGameCollision::BuildAreaCollisionCache(mgr, coll_cache);

    for (int j = batch_start; j < batch_end; j++) {
      if (j == prev_particle || !swoosh_vec[j].x0_active) {
        continue;
      }
      auto& cur_swoosh = swoosh_vec[j];

      CCollidableSphere coll_prim(zeus::CSphere(cur_swoosh.xc_translation, batch_highest_speed),
                                  CMaterialList(EMaterialTypes::Solid));
      CCollisionInfoList coll_info_out;
      bool collided_static = CGameCollision::DetectStaticCollision_Cached(
          mgr, coll_cache, coll_prim, zeus::CTransform(), skExcludeProjectilePassthrough, coll_info_out);

      TUniqueId first_actor_hit = kInvalidUniqueId;
      bool collided_other = FindCollisionInNearList(mgr, near_list_cache, coll_prim, first_actor_hit, coll_info_out);

      if ((collided_static || collided_other) && coll_info_out.GetCount() > 0) {
        cur_swoosh.x0_active = false;
        any_particle_collisions = true;
        cached_cinfo.Clear();
        coll_info_out.AccumulateNewContactsInto(cached_cinfo);
        int k = 0;
        zeus::CVector3f last_added_pt = zeus::skZero3f;
        for (CCollisionInfo const& info : cached_cinfo) {
          if (k > 3) {
            break;
          }
          float overlap_range = x37c_26_runningSlowish ? 1 : 0.75f;
          int num_overlap = SortAndFindOverlappingPoints(Cube{info.GetPoint(), overlap_range});
          const int max_overlapping = (x37c_26_runningSlowish ? 2 : 3);
          if (num_overlap < max_overlapping) {
            AddContactPoint(info, 10);
            zeus::CTransform xf = zeus::lookAt(zeus::skZero3f, info.GetNormalLeft(), zeus::skUp);
            x360_secondarySmokeGen->SetOrientation(xf);
            x364_secondaryFireGen->SetOrientation(xf);
            x368_secondarySparksGen->SetOrientation(xf);
            x360_secondarySmokeGen->SetTranslation(info.GetPoint());
            x364_secondaryFireGen->SetTranslation(info.GetPoint());
            x368_secondarySparksGen->SetTranslation(info.GetPoint());

            x360_secondarySmokeGen->ForceParticleCreation(1);
            x364_secondaryFireGen->ForceParticleCreation(max_overlapping);
            x368_secondarySparksGen->ForceParticleCreation(x37c_26_runningSlowish ? 3 : 5);
            if (x37c_26_runningSlowish) {
              break;
            }
            last_added_pt = info.GetPoint();
          }
          k++;
        }
        if (!x37c_26_runningSlowish && !x300_wasPointAdded) {
          float dist_between = (x2f4_lastParticleCollisionLoc - last_added_pt).magSquared();
          if (cached_cinfo.GetCount() < 3 || dist_between > 3.0f) {
            zeus::CVector3f avg_loc = (x2f4_lastParticleCollisionLoc + last_added_pt) * 0.5f;
            x364_secondaryFireGen->SetTranslation(avg_loc);
            x364_secondaryFireGen->ForceParticleCreation(2);
          }
        }

        x2f4_lastParticleCollisionLoc = last_added_pt;
        x300_wasPointAdded = true;

        if (first_actor_hit != kInvalidUniqueId) {
          if (TCastToPtr<CActor> act = mgr.ObjectById(first_actor_hit)) {
            if (CanDamageActor(*act, mgr)) {
              CDamageInfo dmg_info(x12c_curDamageInfo, dt);
              mgr.ApplyDamage(x8_uid, act->GetUniqueId(), xec_ownerId, dmg_info, xf8_filter,
                              cur_swoosh.x74_velocity.normalized());
            }
          }
        }
        CDamageInfo dmg_info(x12c_curDamageInfo, dt);
        mgr.ApplyDamageToWorld(xec_ownerId, *this, cur_swoosh.xc_translation, dmg_info, xf8_filter);
        collisions_out.push_back({cur_swoosh.xc_translation, batch_highest_speed});
        if (collisions_out.size() == 32) {
          cached_cinfo.Clear();
          coll_info_out.Clear();
          near_list_cache.clear();
          return true;
        }
        cached_cinfo.Clear();
      }
      coll_info_out.Clear();
    }
    near_list_cache.clear();
    mgr.BuildNearList(near_list_cache, batch_particle_bounds,
                      CMaterialFilter::MakeInclude(CMaterialList(EMaterialTypes::NonSolidDamageable)), mgr.Player());
    for (TUniqueId const& uid : near_list_cache) {
      if (TCastToPtr<CSnakeWeedSwarm> sw = mgr.ObjectById(uid)) {
        for (int j = batch_start; j < batch_end; j++) {
          if (j == prev_particle || !swoosh_vec[j].x0_active) {
            continue;
          }
          float damage_radius_multiplier = batch_highest_speed < 1.f ? 1 : batch_highest_speed;
          sw->HandleRadiusDamage(damage_radius_multiplier * sw->GetWeaponDamageRadius(), mgr,
                                 swoosh_vec[j].xc_translation);
        }
      }
    }
    for (int j = batch_start; j < batch_end; j++) {
      if (j == prev_particle || !swoosh_vec[j].x0_active) {
        continue;
      }
      auto& cur_swoosh = swoosh_vec[j];

      CCollidableSphere coll_prim(zeus::CSphere(cur_swoosh.xc_translation, batch_highest_speed),
                                  CMaterialList(EMaterialTypes::Solid));
      CCollisionInfoList coll_info_out;
      TUniqueId first_actor_hit;

      FindCollisionInNearList(mgr, near_list_cache, coll_prim, first_actor_hit, coll_info_out);
      if (first_actor_hit != kInvalidUniqueId) {
        if (TCastToPtr<CActor> act = mgr.ObjectById(first_actor_hit)) {
          if (CanDamageActor(*act, mgr)) {
            CDamageInfo dmg_info(x12c_curDamageInfo, dt);
            mgr.ApplyDamage(x8_uid, act->GetUniqueId(), xec_ownerId, dmg_info, xf8_filter,
                            cur_swoosh.x74_velocity.normalized());
          }
        }
      }
      coll_info_out.Clear();
    }
    i += batch_process_size;
  }
  DecrementContactPointTimers();
  near_list_cache.clear();
  return any_particle_collisions;
}

bool CNewFlameThrower::CanDamageActor(CActor& hit_actor, CStateManager& mgr) {
  CDamageVulnerability const* vuln = hit_actor.GetDamageVulnerability();
  if (vuln->GetVulnerability(x12c_curDamageInfo.GetWeaponMode(), false) == EVulnerability::PassThrough) {
    return false;
  }
  if (TCastToPtr<CScriptTrigger> trigger = hit_actor) {
    CProjectileTouchResult res = CanCollideWithTrigger(*trigger, mgr);
    return res.GetActorId() != kInvalidUniqueId;
  }
  if (TCastToPtr<CScriptPlatform> platform = hit_actor) {
    return true;
  }
  if (TCastToPtr<CCollisionActor> coll_actor = hit_actor) {
    return true;
  }
  if (CPatterned::CastTo<MP1::CFlickerBat>(&hit_actor)) {
    return true;
  }
  CProjectileTouchResult res = CanCollideWithGameObject(hit_actor, mgr);
  return res.GetActorId() != kInvalidUniqueId;
}

void CNewFlameThrower::AddContactPoint(CCollisionInfo const& cinfo, u32 time) {
  int elem = 0;
  for (auto& cp_vec : x380_flameContactPoints) {
    cp_vec.emplace_back(cinfo.GetPoint()[elem], time);
    elem++;
  }
  x37c_27_newPointAdded = true;
}

int CNewFlameThrower::SortAndFindOverlappingPoints(Cube const& box) {
  if (x37c_27_newPointAdded) {
    for (auto& component_vec : x380_flameContactPoints) {
      std::sort(component_vec.begin(), component_vec.end());
    }
  }
  int min_overlap = std::numeric_limits<int>::max();

  for (int i = 0; i < x380_flameContactPoints.size(); i++) {
    auto const& component_vec = x380_flameContactPoints[i];
    float search_min = box.center[i] - box.bounds;
    float search_max = box.center[i] + box.bounds;
    auto min_result = rstl::binary_find(component_vec.begin(), component_vec.end(), Contact{search_min, 0});
    auto max_result = rstl::binary_find(component_vec.begin(), component_vec.end(), Contact{search_max, 0});
    if (min_result == component_vec.end()) {
      min_result = component_vec.begin();
    }

    int num_overlap = static_cast<int>(max_result - min_result);
    num_overlap = num_overlap < min_overlap ? num_overlap : min_overlap;
    if (num_overlap == 0) {
      return 0;
    }
    min_overlap = num_overlap;
  }
  return min_overlap;
}

bool CNewFlameThrower::FindCollisionInNearList(CStateManager& mgr, EntityList const& near_list,
                                               CCollisionPrimitive const& coll, TUniqueId& first_coll_out,
                                               CCollisionInfoList& collisions) {
  for (TUniqueId const& cur_uid : near_list) {
    if (TCastToPtr<CActor> near_actor = mgr.ObjectById(cur_uid)) {
      if (TCastToPtr<CPhysicsActor> pa = *near_actor) {
        CInternalCollisionStructure::CPrimDesc pa_desc(*pa->GetCollisionPrimitive(), pa->GetMaterialFilter(),
                                                       pa->GetPrimitiveTransform());
        CInternalCollisionStructure::CPrimDesc param_desc(coll, skExcludeProjectilePassthrough, zeus::CTransform());

        if (CCollisionPrimitive::Collide(pa_desc, param_desc, collisions)) {
          first_coll_out = cur_uid;
          return true;
        }
      } else {
        auto bounds = near_actor->GetTouchBounds();
        if (!bounds.has_value()) {
          continue;
        }
        CCollidableAABox coll_aabb(bounds.value(), CMaterialList(EMaterialTypes::Solid));
        CInternalCollisionStructure::CPrimDesc aabb_desc(coll_aabb, CMaterialFilter::skPassEverything,
                                                         zeus::CTransform());
        CInternalCollisionStructure::CPrimDesc param_desc(coll, skExcludeProjectilePassthrough, zeus::CTransform());

        if (CCollisionPrimitive::Collide(param_desc, aabb_desc, collisions)) {
          first_coll_out = cur_uid;
          return true;
        }
      }
    }
  }
  return false;
}

void CNewFlameThrower::DecrementContactPointTimers() {
  for (auto& vec : x380_flameContactPoints) {
    int end = static_cast<int>(vec.size() - 1);
    for (size_t i = 0; i < vec.size(); i++) {
      vec[i].remainingTime--;
      if (vec[i].remainingTime == 0) {
        vec[i] = vec[end];
        vec.pop_back();
        // forgot to decrement iterator ?
        // i--;
        end--;
      }
    }
  }
}
void CNewFlameThrower::Reset(CStateManager& mgr, bool deactivate) {
  if (deactivate) {
    SetLightsActive(mgr, false);
    SetActive(false);
    x374_flameState = EFlameState::Default;
    x2ec_particlesDoneTimer = 0.f;
    x2f0_flamesDoneTimer = 0.f;
  } else {
    x374_flameState = EFlameState::FireStopTimer;
  }
  x37c_25_firing = false;
  x358_mainFireGen->SetParticleEmission(false);
  x35c_mainSmokeGen->SetParticleEmission(false);
  x36c_swooshCenterGen->SetParticleEmission(false);
  x370_swooshFireGen->SetParticleEmission(false);
}

void CNewFlameThrower::SetLightsActive(CStateManager& mgr, bool active) {
  for (TUniqueId const& uid : x3b8_lightIds) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(uid)) {
      light->SetActive(active);
    }
  }
}

void CNewFlameThrower::UpdateFlameState(float dt, CStateManager& mgr) {
  bool flame_light_active = false;
  switch (x374_flameState) {
  case EFlameState::FireWaitForParticlesDone:
    x2ec_particlesDoneTimer += dt;
    if (x2ec_particlesDoneTimer > 0.1f && AreEffectsFinished()) {
      x374_flameState = EFlameState::Default;
      Reset(mgr, true);
    }
    break;
  case EFlameState::FireStopTimer:
    flame_light_active = true;
    x2f0_flamesDoneTimer = (dt * 4) + x2f0_flamesDoneTimer;
    if (x2f0_flamesDoneTimer > 1.f) {
      x2f0_flamesDoneTimer = 1.f;
      x374_flameState = EFlameState::FireWaitForParticlesDone;
      x37c_24_renderAuxEffects = false;
    }
    break;
  case EFlameState::FireStart:
    x374_flameState = EFlameState::FireActive;
    break;
  case EFlameState::FireActive:
    flame_light_active = true;
    break;
  default:
    break;
  }
  const float speed = flame_light_active ? 4.f : 1.f;
  const float target = flame_light_active ? 0.7f : 1.f;
  SetWorldLighting(mgr, mgr.GetPlayer().GetAreaIdAlways(), speed, target);
}

void CNewFlameThrower::SetWorldLighting(CStateManager& mgr, TAreaId area, float speed, float target) {
  if (x37c_28_activeLighting && x378_currentLitArea != kInvalidAreaId) {
    CGameArea* lit_area = mgr.GetWorld()->GetArea(x378_currentLitArea);
    // Restore previous area's lighting
    if (!lit_area->IsPostConstructed()) {
      lit_area->SetWeaponWorldLighting(1.f, 1.f);
    }
  }
  x378_currentLitArea = area;
  x37c_28_activeLighting = (target != 1.f);
  if (x378_currentLitArea != kInvalidAreaId) {
    CGameArea* lit_area = mgr.GetWorld()->GetArea(x378_currentLitArea);
    if (!lit_area->IsPostConstructed()) {
      lit_area->SetWeaponWorldLighting(speed, target);
    }
  }
}

} // namespace metaforce
