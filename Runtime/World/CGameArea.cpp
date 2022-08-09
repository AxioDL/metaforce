#include "Runtime/World/CGameArea.hpp"

#include <array>
#include <cstring>

#include "Runtime/CGameState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CCubeSurface.hpp"
#include "Runtime/World/CScriptAreaAttributes.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

static logvisor::Module Log("CGameArea");

CAreaRenderOctTree::CAreaRenderOctTree(const u8* buf) : x0_buf(buf) {
  CMemoryInStream r(x0_buf + 8, INT32_MAX);
  x8_bitmapCount = r.ReadLong();
  xc_meshCount = r.ReadLong();
  x10_nodeCount = r.ReadLong();
  x14_bitmapWordCount = (xc_meshCount + 31) / 32;
  x18_aabb = r.Get<zeus::CAABox>();

  x30_bitmaps = reinterpret_cast<const u32*>(x0_buf + 64);
  u32 wc = x14_bitmapWordCount * x8_bitmapCount;
  for (u32 i = 0; i < wc; ++i)
    const_cast<u32*>(x30_bitmaps)[i] = CBasics::SwapBytes(x30_bitmaps[i]);

  x34_indirectionTable = x30_bitmaps + wc;
  x38_entries = reinterpret_cast<const u8*>(x34_indirectionTable + x10_nodeCount);
  for (u32 i = 0; i < x10_nodeCount; ++i) {
    const_cast<u32*>(x34_indirectionTable)[i] = CBasics::SwapBytes(x34_indirectionTable[i]);
    Node* n = reinterpret_cast<Node*>(const_cast<u8*>(x38_entries) + x34_indirectionTable[i]);
    n->x0_bitmapIdx = CBasics::SwapBytes(n->x0_bitmapIdx);
    n->x2_flags = CBasics::SwapBytes(n->x2_flags);
    if (n->x2_flags) {
      u32 childCount = n->GetChildCount();
      for (u32 c = 0; c < childCount; ++c)
        n->x4_children[c] = CBasics::SwapBytes(n->x4_children[c]);
    }
  }
}

u32 CAreaRenderOctTree::Node::GetChildCount() const {
  static constexpr std::array<u32, 8> ChildCounts{0, 2, 2, 4, 2, 4, 4, 8};
  return ChildCounts[x2_flags];
}

zeus::CAABox CAreaRenderOctTree::Node::GetNodeBounds(const zeus::CAABox& curAABB, int idx) const {
  zeus::CVector3f center = curAABB.center();
  switch (x2_flags) {
  case 0:
  default:
    return curAABB;
  case 1:
    if (idx == 0)
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), center.x(), curAABB.max.y(), curAABB.max.z()};
    else
      return {center.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
  case 2:
    if (idx == 0)
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), center.y(), curAABB.max.z()};
    else
      return {curAABB.min.x(), center.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
  case 3: {
    switch (idx) {
    case 0:
    default:
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), center.x(), center.y(), curAABB.max.z()};
    case 1:
      return {center.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), center.y(), curAABB.max.z()};
    case 2:
      return {curAABB.min.x(), center.y(), curAABB.min.z(), center.x(), curAABB.max.y(), curAABB.max.z()};
    case 3:
      return {center.x(), center.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
    }
  }
  case 4:
    if (idx == 0)
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), center.z()};
    else
      return {curAABB.min.x(), curAABB.min.y(), center.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
  case 5: {
    switch (idx) {
    case 0:
    default:
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), center.x(), curAABB.max.y(), center.z()};
    case 1:
      return {center.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), center.z()};
    case 2:
      return {curAABB.min.x(), curAABB.min.y(), center.z(), center.x(), curAABB.max.y(), curAABB.max.z()};
    case 3:
      return {center.x(), curAABB.min.y(), center.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
    }
  }
  case 6: {
    switch (idx) {
    case 0:
    default:
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), center.y(), center.z()};
    case 1:
      return {curAABB.min.x(), center.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), center.z()};
    case 2:
      return {curAABB.min.x(), curAABB.min.y(), center.z(), curAABB.max.x(), center.y(), curAABB.max.z()};
    case 3:
      return {curAABB.min.x(), center.y(), center.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
    }
  }
  case 7: {
    switch (idx) {
    case 0:
    default:
      return {curAABB.min.x(), curAABB.min.y(), curAABB.min.z(), center.x(), center.y(), center.z()};
    case 1:
      return {center.x(), curAABB.min.y(), curAABB.min.z(), curAABB.max.x(), center.y(), center.z()};
    case 2:
      return {curAABB.min.x(), center.y(), curAABB.min.z(), center.x(), curAABB.max.y(), center.z()};
    case 3:
      return {center.x(), center.y(), curAABB.min.z(), curAABB.max.x(), curAABB.max.y(), center.z()};
    case 4:
      return {curAABB.min.x(), curAABB.min.y(), center.z(), center.x(), center.y(), curAABB.max.z()};
    case 5:
      return {center.x(), curAABB.min.y(), center.z(), curAABB.max.x(), center.y(), curAABB.max.z()};
    case 6:
      return {curAABB.min.x(), center.y(), center.z(), center.x(), curAABB.max.y(), curAABB.max.z()};
    case 7:
      return {center.x(), center.y(), center.z(), curAABB.max.x(), curAABB.max.y(), curAABB.max.z()};
    }
  }
  }
}

void CAreaRenderOctTree::Node::RecursiveBuildOverlaps(u32* bmpOut, const CAreaRenderOctTree& parent,
                                                      const zeus::CAABox& curAABB, const zeus::CAABox& testAABB) const {
  if (testAABB.intersects(curAABB)) {
    u32 childCount = GetChildCount(); // HACK: Always return the smallest set of intersections
    if (curAABB.inside(testAABB) || childCount == 0) {
      const u32* bmp = &parent.x30_bitmaps[x0_bitmapIdx * parent.x14_bitmapWordCount];
      for (u32 c = 0; c < parent.x14_bitmapWordCount; ++c)
        bmpOut[c] |= bmp[c];
    } else {
      for (u32 c = 0; c < childCount; ++c) {
        zeus::CAABox childAABB = GetNodeBounds(curAABB, c);
        reinterpret_cast<const Node*>(parent.x38_entries + parent.x34_indirectionTable[x4_children[c]])
            ->RecursiveBuildOverlaps(bmpOut, parent, childAABB, testAABB);
      }
    }
  }
}

void CAreaRenderOctTree::FindOverlappingModels(std::vector<u32>& out, const zeus::CAABox& testAABB) const {
  out.resize(x14_bitmapWordCount);
  reinterpret_cast<const Node*>(x38_entries + x34_indirectionTable[0])
      ->RecursiveBuildOverlaps(out.data(), *this, x18_aabb, testAABB);
}

void CAreaRenderOctTree::FindOverlappingModels(u32* out, const zeus::CAABox& testAABB) const {
  reinterpret_cast<const Node*>(x38_entries + x34_indirectionTable[0])
      ->RecursiveBuildOverlaps(out, *this, x18_aabb, testAABB);
}

void CGameArea::CAreaFog::SetCurrent() const {
  g_Renderer->SetWorldFog(x0_fogMode, x4_rangeCur[0], x4_rangeCur[1], x1c_colorCur);
}

void CGameArea::CAreaFog::Update(float dt) {
  if (x0_fogMode == ERglFogMode::None)
    return;
  if (x1c_colorCur == x28_colorTarget && x4_rangeCur == xc_rangeTarget)
    return;

  float colorDelta = x34_colorDelta * dt;
  zeus::CVector2f rangeDelta = x14_rangeDelta * dt;

  for (u32 i = 0; i < 3; ++i) {
    float delta = x28_colorTarget[i] - x1c_colorCur[i];
    if (std::fabs(delta) <= colorDelta) {
      x1c_colorCur[i] = float(x28_colorTarget[i]);
    } else {
      if (delta < 0.f)
        x1c_colorCur[i] -= colorDelta;
      else
        x1c_colorCur[i] += colorDelta;
    }
  }

  for (u32 i = 0; i < 2; ++i) {
    float delta = xc_rangeTarget[i] - x4_rangeCur[i];
    if (std::fabs(delta) <= rangeDelta[i]) {
      x4_rangeCur[i] = float(xc_rangeTarget[i]);
    } else {
      if (delta < 0.f)
        x4_rangeCur[i] -= rangeDelta[i];
      else
        x4_rangeCur[i] += rangeDelta[i];
    }
  }
}

void CGameArea::CAreaFog::RollFogOut(float rangeDelta, float colorDelta, const zeus::CColor& color) {
  x14_rangeDelta = {rangeDelta, rangeDelta * 2.f};
  xc_rangeTarget = {4096.f, 4096.f};
  x34_colorDelta = colorDelta;
  x28_colorTarget = color;
}

void CGameArea::CAreaFog::FadeFog(ERglFogMode mode, const zeus::CColor& color, const zeus::CVector2f& vec1, float speed,
                                  const zeus::CVector2f& vec2) {
  if (x0_fogMode == ERglFogMode::None) {
    x1c_colorCur = color;
    x28_colorTarget = color;
    x4_rangeCur = {vec1[1], vec1[1]};
    xc_rangeTarget = vec1;
  } else {
    x28_colorTarget = color;
    xc_rangeTarget = vec1;
  }
  x0_fogMode = mode;
  x34_colorDelta = speed;
  x14_rangeDelta = vec2;
}

void CGameArea::CAreaFog::SetFogExplicit(ERglFogMode mode, const zeus::CColor& color, const zeus::CVector2f& range) {
  x0_fogMode = mode;
  x1c_colorCur = color;
  x28_colorTarget = color;
  x4_rangeCur = range;
  xc_rangeTarget = range;
}

bool CGameArea::CAreaFog::IsFogDisabled() const { return x0_fogMode == ERglFogMode::None; }

void CGameArea::CAreaFog::DisableFog() { x0_fogMode = ERglFogMode::None; }

static std::vector<SObjectTag> ReadDependencyList(CInputStream& in) {
  std::vector<SObjectTag> ret;
  const u32 count = in.ReadLong();
  ret.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    ret.emplace_back().ReadMLVL(in);
  }
  return ret;
}

std::pair<std::unique_ptr<u8[]>, s32> GetScriptingMemoryAlways(const IGameArea& area) {
  const SObjectTag tag = {SBIG('MREA'), area.IGetAreaAssetId()};
  std::unique_ptr<u8[]> data = g_ResFactory->LoadNewResourcePartSync(tag, 0, 96);

  u32 magic{};
  std::memcpy(&magic, data.get(), sizeof(u32));
  if (magic != SBIG(0xDEADBEEF)) {
    return {};
  }

  CMemoryInStream r(data.get() + 4, 96 - 4);
  u32 version = r.ReadLong();
  if ((version & 0x10000) == 0) {
    Log.report(logvisor::Fatal, FMT_STRING("Attempted to load non-URDE MREA"));
  }
  version &= ~0x10000;

  SMREAHeader header;
  header.version = (version >= 12 && version <= 15) ? version : 0;
  if (!header.version) {
    return {};
  }

  header.xf = r.Get<zeus::CTransform>();
  header.modelCount = r.ReadLong();
  header.secCount = r.ReadLong();
  header.geomSecIdx = r.ReadLong();
  header.sclySecIdx = r.ReadLong();
  header.collisionSecIdx = r.ReadLong();
  header.unkSecIdx = r.ReadLong();
  header.lightSecIdx = r.ReadLong();
  header.visiSecIdx = r.ReadLong();
  header.pathSecIdx = r.ReadLong();
  header.arotSecIdx = r.ReadLong();

  u32 dataLen = ROUND_UP_32(header.secCount * 4);

  data = g_ResFactory->LoadNewResourcePartSync(tag, 96, dataLen);

  r = CMemoryInStream(data.get(), dataLen);

  std::vector<u32> secSizes(header.secCount);
  u32 lastSize;
  for (u32 i = 0; i < header.secCount; ++i) {
    lastSize = r.ReadLong();
    secSizes.push_back(lastSize);
  }

  // TODO: Finish
  return {};
}

CDummyGameArea::CDummyGameArea(CInputStream& in, int idx, int mlvlVersion) {
  x8_nameSTRG = in.Get<CAssetId>();
  x14_transform = in.Get<zeus::CTransform>();
  zeus::CAABox aabb = in.Get<zeus::CAABox>();
  xc_mrea = in.Get<CAssetId>();
  if (mlvlVersion > 15) {
    x10_areaId = in.ReadLong();
  } else {
    x10_areaId = -1;
  }

  u32 attachAreaCount = in.ReadLong();
  x44_attachedAreaIndices.reserve(attachAreaCount);
  for (u32 i = 0; i < attachAreaCount; ++i)
    x44_attachedAreaIndices.push_back(in.ReadShort());

  ::metaforce::ReadDependencyList(in);
  ::metaforce::ReadDependencyList(in);

  if (mlvlVersion > 13) {
    u32 depCount = in.ReadLong();
    for (u32 i = 0; i < depCount; ++i)
      in.ReadLong();
  }

  u32 dockCount = in.ReadLong();
  x54_docks.reserve(dockCount);
  for (u32 i = 0; i < dockCount; ++i)
    x54_docks.emplace_back(in, x14_transform);
}

std::pair<std::unique_ptr<u8[]>, s32> CDummyGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

s32 CDummyGameArea::IGetAreaSaveId() const { return x10_areaId; }

CAssetId CDummyGameArea::IGetAreaAssetId() const { return xc_mrea; }

bool CDummyGameArea::IIsActive() const { return true; }

TAreaId CDummyGameArea::IGetAttachedAreaId(int idx) const { return x44_attachedAreaIndices[idx]; }

u32 CDummyGameArea::IGetNumAttachedAreas() const { return x44_attachedAreaIndices.size(); }

CAssetId CDummyGameArea::IGetStringTableAssetId() const { return x8_nameSTRG; }

const zeus::CTransform& CDummyGameArea::IGetTM() const { return x14_transform; }

CGameArea::CGameArea(CInputStream& in, int idx, int mlvlVersion) : x4_selfIdx(idx) {
  x8_nameSTRG = in.Get<CAssetId>();
  xc_transform = in.Get<zeus::CTransform>();
  x3c_invTransform = xc_transform.inverse();
  x6c_aabb = in.Get<zeus::CAABox>();

  x84_mrea = in.Get<CAssetId>();
  if (mlvlVersion > 15)
    x88_areaId = in.ReadLong();
  else
    x88_areaId = INT_MAX;

  const u32 attachedCount = in.ReadLong();
  x8c_attachedAreaIndices.reserve(attachedCount);
  for (u32 i = 0; i < attachedCount; ++i) {
    x8c_attachedAreaIndices.emplace_back(in.ReadShort());
  }

  x9c_deps1 = metaforce::ReadDependencyList(in);
  xac_deps2 = metaforce::ReadDependencyList(in);

  const zeus::CAABox aabb = x6c_aabb.getTransformedAABox(xc_transform);
  x6c_aabb = aabb;

  if (mlvlVersion > 13) {
    const u32 depCount = in.ReadLong();
    xbc_layerDepOffsets.reserve(depCount);
    for (u32 i = 0; i < depCount; ++i) {
      xbc_layerDepOffsets.emplace_back(in.ReadLong());
    }
  }

  const u32 dockCount = in.ReadLong();
  xcc_docks.reserve(dockCount);
  for (u32 i = 0; i < dockCount; ++i) {
    xcc_docks.emplace_back(in, xc_transform);
  }

  ClearTokenList();

  for (CToken& tok : xdc_tokens)
    xec_totalResourcesSize += g_ResFactory->ResourceSize(*tok.GetObjectTag());

  xec_totalResourcesSize += g_ResFactory->ResourceSize(SObjectTag{FOURCC('MREA'), x84_mrea});
}

CGameArea::~CGameArea() {
  for (auto& lt : xf8_loadTransactions)
    lt->PostCancelRequest();

  if (xf0_24_postConstructed)
    RemoveStaticGeometry();
  else
    while (!Invalidate(nullptr)) {}
}

bool CGameArea::IsFinishedOccluding() const {
  if (x12c_postConstructed->x10dc_occlusionState != EOcclusionState::Occluded)
    return true;

  return x12c_postConstructed->x1108_27_;
}

std::pair<std::unique_ptr<u8[]>, s32> CGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

bool CGameArea::IIsActive() const { return xf0_25_active; }

TAreaId CGameArea::IGetAttachedAreaId(int idx) const { return x8c_attachedAreaIndices[idx]; }

u32 CGameArea::IGetNumAttachedAreas() const { return x8c_attachedAreaIndices.size(); }

CAssetId CGameArea::IGetStringTableAssetId() const { return x8_nameSTRG; }

const zeus::CTransform& CGameArea::IGetTM() const { return xc_transform; }

void CGameArea::SetLoadPauseState(bool paused) {
  if (xf0_26_tokensReady)
    return;
  xf0_27_loadPaused = paused;
  if (!paused)
    return;

  for (CToken& tok : xdc_tokens)
    if (!tok.IsLoaded())
      tok.Unlock();
}

void CGameArea::SetXRaySpeedAndTarget(float f1, float f2) {
  x12c_postConstructed->x112c_xraySpeed = f1;
  x12c_postConstructed->x1130_xrayTarget = f2;
}

void CGameArea::SetThermalSpeedAndTarget(float speed, float target) {
  x12c_postConstructed->x1120_thermalSpeed = speed;
  x12c_postConstructed->x1124_thermalTarget = target;
}

void CGameArea::SetWeaponWorldLighting(float speed, float target) {
  x12c_postConstructed->x1134_weaponWorldLightingSpeed = speed;
  x12c_postConstructed->x1138_weaponWorldLightingTarget = target;
}

float CGameArea::GetXRayFogDistance() const {
  const CScriptAreaAttributes* attrs = x12c_postConstructed->x10d8_areaAttributes;
  if (attrs)
    return attrs->GetXRayFogDistance();
  return 1.f;
}

EEnvFxType CGameArea::DoesAreaNeedEnvFx() const {
  const CPostConstructed* postConstructed = GetPostConstructed();
  if (!postConstructed)
    return EEnvFxType::None;

  const CScriptAreaAttributes* attrs = postConstructed->x10d8_areaAttributes;
  if (!attrs)
    return EEnvFxType::None;
  if (postConstructed->x10dc_occlusionState == EOcclusionState::Occluded)
    return EEnvFxType::None;
  return attrs->GetEnvFxType();
}

bool CGameArea::DoesAreaNeedSkyNow() const {
  const CPostConstructed* postConstructed = GetPostConstructed();
  if (!postConstructed)
    return false;

  const CScriptAreaAttributes* attrs = postConstructed->x10d8_areaAttributes;
  if (!attrs)
    return false;

  return attrs->GetNeedsSky();
}

void CGameArea::UpdateFog(float dt) {
  CAreaFog* fog = GetPostConstructed()->x10c4_areaFog.get();
  if (fog)
    fog->Update(dt);
}

void CGameArea::OtherAreaOcclusionChanged() {
  if (GetPostConstructed()->x10e0_ == 3 && GetPostConstructed()->x10dc_occlusionState == EOcclusionState::Visible) {
    x12c_postConstructed->x1108_27_ = false;
  } else if (GetPostConstructed()->x10dc_occlusionState == EOcclusionState::Visible) {
    ReloadAllUnloadedTextures();
  }
}

void CGameArea::PingOcclusionState() {
  if (GetOcclusionState() == EOcclusionState::Occluded && GetPostConstructed()->x10e0_ < 2) {
    x12c_postConstructed->x10e0_ += 1;
    return;
  }

  x12c_postConstructed->x10e0_ = 3;
  if (!x12c_postConstructed->x1108_27_) {
    bool unloaded = true;
    bool transferred = true;
#if 0
        unloaded = UnloadAllLoadedTextures();
        transferred = TransferTokens();
#endif
    if (unloaded && transferred)
      x12c_postConstructed->x1108_27_ = true;
  }
  x12c_postConstructed->x1108_26_ = true;
}

void CGameArea::PreRender() {
  if (!xf0_24_postConstructed)
    return;

  if (x12c_postConstructed->x1108_28_occlusionPinged)
    x12c_postConstructed->x1108_28_occlusionPinged = false;
  else
    PingOcclusionState();
}

void CGameArea::UpdateThermalVisor(float dt) {
  if (x12c_postConstructed->x1120_thermalSpeed == 0.f)
    return;

  float influence = x12c_postConstructed->x111c_thermalCurrent;

  float delta = x12c_postConstructed->x1120_thermalSpeed * dt;
  if (std::fabs(x12c_postConstructed->x1124_thermalTarget - x12c_postConstructed->x111c_thermalCurrent) < delta) {
    influence = x12c_postConstructed->x1124_thermalTarget;
    x12c_postConstructed->x1120_thermalSpeed = 0.f;
  } else if (x12c_postConstructed->x1124_thermalTarget < influence)
    influence -= delta;
  else
    influence += delta;

  x12c_postConstructed->x111c_thermalCurrent = influence;
}

void CGameArea::UpdateWeaponWorldLighting(float dt) {
  float newLightingLevel = x12c_postConstructed->x1128_worldLightingLevel;
  if (x12c_postConstructed->x112c_xraySpeed != 0.f) {
    float speed = dt * x12c_postConstructed->x112c_xraySpeed;
    if (std::fabs(x12c_postConstructed->x1130_xrayTarget - newLightingLevel) < speed) {
      newLightingLevel = x12c_postConstructed->x1130_xrayTarget;
      x12c_postConstructed->x1134_weaponWorldLightingSpeed = 0.f;
    } else if (x12c_postConstructed->x1130_xrayTarget < newLightingLevel) {
      newLightingLevel -= speed;
    } else {
      newLightingLevel += speed;
    }
  }

  if (x12c_postConstructed->x1134_weaponWorldLightingSpeed != 0.f) {
    float newWeaponWorldLightingLevel = x12c_postConstructed->x1128_worldLightingLevel;
    float speed = dt * x12c_postConstructed->x1134_weaponWorldLightingSpeed;
    if (std::fabs(x12c_postConstructed->x1138_weaponWorldLightingTarget - newLightingLevel) < speed) {
      newWeaponWorldLightingLevel = x12c_postConstructed->x1138_weaponWorldLightingTarget;
      x12c_postConstructed->x1134_weaponWorldLightingSpeed = 0.f;
    } else if (x12c_postConstructed->x1138_weaponWorldLightingTarget < newWeaponWorldLightingLevel) {
      newWeaponWorldLightingLevel -= speed;
    } else {
      newWeaponWorldLightingLevel += speed;
    }
    if (x12c_postConstructed->x112c_xraySpeed != 0.f) {
      newLightingLevel = std::min(newLightingLevel, newWeaponWorldLightingLevel);
    } else {
      newLightingLevel = newWeaponWorldLightingLevel;
    }
  }

  if (std::fabs(x12c_postConstructed->x1128_worldLightingLevel - newLightingLevel) >= 0.00001f) {
    x12c_postConstructed->x1128_worldLightingLevel = newLightingLevel;
    for (CEntity* ent : *x12c_postConstructed->x10c0_areaObjs)
      if (TCastToPtr<CActor> act = ent)
        act->SetWorldLightingDirty(true);
  }
}

void CGameArea::AliveUpdate(float dt) {
  if (x12c_postConstructed->x10dc_occlusionState == EOcclusionState::Occluded)
    x12c_postConstructed->x10e4_occludedTime += dt;
  else
    x12c_postConstructed->x10e4_occludedTime = 0.f;
  UpdateFog(dt);
  UpdateThermalVisor(dt);
  UpdateWeaponWorldLighting(dt);
}

void CGameArea::SetOcclusionState(EOcclusionState state) {
  if (!xf0_24_postConstructed || x12c_postConstructed->x10dc_occlusionState == state)
    return;

  if (state != EOcclusionState::Occluded) {
    ReloadAllUnloadedTextures();
    AddStaticGeometry();
  } else {
    x12c_postConstructed->x1108_26_ = true;
    x12c_postConstructed->x1108_27_ = false;
    RemoveStaticGeometry();
  }
}

void CGameArea::RemoveStaticGeometry() {
  if (!xf0_24_postConstructed || !x12c_postConstructed ||
      x12c_postConstructed->x10dc_occlusionState == EOcclusionState::Occluded)
    return;
  x12c_postConstructed->x10e0_ = 0;
  x12c_postConstructed->x10dc_occlusionState = EOcclusionState::Occluded;
  g_Renderer->RemoveStaticGeometry(&x12c_postConstructed->x4c_insts);
}

void CGameArea::AddStaticGeometry() {
  if (x12c_postConstructed->x10dc_occlusionState != EOcclusionState::Visible) {
    x12c_postConstructed->x10e0_ = 0;
    x12c_postConstructed->x10dc_occlusionState = EOcclusionState::Visible;
    if (!x12c_postConstructed->x1108_25_modelsConstructed)
      FillInStaticGeometry();
    g_Renderer->AddStaticGeometry(&x12c_postConstructed->x4c_insts,
                                  x12c_postConstructed->xc_octTree ? &*x12c_postConstructed->xc_octTree : nullptr,
                                  x4_selfIdx);
  }
}

EChain CGameArea::SetChain(CGameArea* next, EChain setChain) {
  if (x138_curChain == setChain)
    return x138_curChain;

  if (x134_prev)
    x134_prev->x130_next = x130_next;
  if (x130_next)
    x130_next->x134_prev = x134_prev;

  x134_prev = nullptr;
  x130_next = next;
  if (next)
    next->x134_prev = this;

  EChain ret = x138_curChain;
  x138_curChain = setChain;
  return ret;
}

bool CGameArea::StartStreamingMainArea() {
  if (xf0_24_postConstructed)
    return false;

  switch (xf4_phase) {
  case EPhase::LoadHeader: {
    x110_mreaSecBufs.reserve(3);
    AllocNewAreaData(0, 96);
    x12c_postConstructed = std::make_unique<CPostConstructed>();
    xf4_phase = EPhase::LoadSecSizes;
    break;
  }
  case EPhase::LoadSecSizes: {
    CullDeadAreaRequests();
    if (xf8_loadTransactions.size())
      break;
    SMREAHeader header = VerifyHeader();
    AllocNewAreaData(x110_mreaSecBufs[0].second, ROUND_UP_32(header.secCount * 4));
    xf4_phase = EPhase::ReserveSections;
    break;
  }
  case EPhase::ReserveSections: {
    CullDeadAreaRequests();
    if (xf8_loadTransactions.size())
      break;
    // x110_mreaSecBufs.reserve(GetNumPartSizes() + 2);
    x124_secCount = 0;
    x128_mreaDataOffset = x110_mreaSecBufs[0].second + x110_mreaSecBufs[1].second;
    xf4_phase = EPhase::LoadDataSections;
    break;
  }
  case EPhase::LoadDataSections: {
    CullDeadAreaRequests();

    u32 totalSz = 0;
    u32 secCount = GetNumPartSizes();
    for (u32 i = 0; i < secCount; ++i)
      totalSz += CBasics::SwapBytes(reinterpret_cast<u32*>(x110_mreaSecBufs[1].first.get())[i]);

    AllocNewAreaData(x128_mreaDataOffset, totalSz);

    m_resolvedBufs.reserve(secCount);
    m_resolvedBufs.emplace_back(x110_mreaSecBufs[0].first.get(), x110_mreaSecBufs[0].second);
    m_resolvedBufs.emplace_back(x110_mreaSecBufs[1].first.get(), x110_mreaSecBufs[1].second);

    u32 curOff = 0;
    for (u32 i = 0; i < secCount; ++i) {
      u32 size = CBasics::SwapBytes(reinterpret_cast<u32*>(x110_mreaSecBufs[1].first.get())[i]);
      m_resolvedBufs.emplace_back(x110_mreaSecBufs[2].first.get() + curOff, size);
      curOff += size;
    }

    xf4_phase = EPhase::WaitForFinish;
    break;
  }
  case EPhase::WaitForFinish: {
    CullDeadAreaRequests();
    if (xf8_loadTransactions.size())
      break;
    return false;
  }
  default:
    break;
  }

  return true;
}

void CGameArea::ReloadAllUnloadedTextures() {}

u32 CGameArea::GetNumPartSizes() const {
  u32 value{};
  std::memcpy(&value, x110_mreaSecBufs[0].first.get() + 60, sizeof(u32));
  return CBasics::SwapBytes(value);
}

void CGameArea::AllocNewAreaData(int offset, int size) {
  x110_mreaSecBufs.emplace_back(std::unique_ptr<u8[]>(new u8[size]), size);
  xf8_loadTransactions.push_back(g_ResFactory->LoadResourcePartAsync(SObjectTag{FOURCC('MREA'), x84_mrea}, offset, size,
                                                                     x110_mreaSecBufs.back().first.get()));
}

bool CGameArea::Invalidate(CStateManager* mgr) {
  if (!xf0_24_postConstructed) {
    ClearTokenList();

    for (auto it = xf8_loadTransactions.begin(); it != xf8_loadTransactions.end();) {
      if (!(*it)->IsComplete()) {
        (*it)->PostCancelRequest();
        ++it;
        continue;
      }
      it = xf8_loadTransactions.erase(it);
    }
    if (xf8_loadTransactions.size() != 0)
      return false;

    x12c_postConstructed.reset();
    KillmAreaData();

    return true;
  }

  if (mgr)
    mgr->PrepareAreaUnload(GetAreaId());

#if 0
    dword_805a8eb0 -= GetPostConstructedSize();
#endif
  RemoveStaticGeometry();
  x12c_postConstructed.reset();
  xf0_24_postConstructed = false;
  xf0_28_validated = false;
  xf4_phase = EPhase::LoadHeader;
  xf8_loadTransactions.clear();
  CullDeadAreaRequests();
  KillmAreaData();
  ClearTokenList();
  if (mgr)
    mgr->AreaUnloaded(GetAreaId());

  return true;
}

void CGameArea::KillmAreaData() {
  m_resolvedBufs.clear();
  x110_mreaSecBufs.clear();
}

void CGameArea::CullDeadAreaRequests() {
  for (auto it = xf8_loadTransactions.begin(); it != xf8_loadTransactions.end();) {
    if ((*it)->IsComplete()) {
      it = xf8_loadTransactions.erase(it);
      continue;
    }
    ++it;
  }
}

void CGameArea::StartStreamIn(CStateManager& mgr) {
  if (xf0_24_postConstructed || xf0_27_loadPaused)
    return;

  OPTICK_EVENT();
  VerifyTokenList(mgr);

  if (!xf0_26_tokensReady) {
    u32 notLoaded = 0;
    for (CToken& tok : xdc_tokens) {
      tok.Lock();
      if (!tok.IsLoaded())
        ++notLoaded;
    }
    if (notLoaded)
      return;
    xf0_26_tokensReady = true;
  }

  StartStreamingMainArea();
  if (xf4_phase != EPhase::WaitForFinish)
    return;
  CullDeadAreaRequests();
  if (xf8_loadTransactions.size())
    return;
  Validate(mgr);
}

void CGameArea::Validate(CStateManager& mgr) {
  if (xf0_24_postConstructed)
    return;

  while (StartStreamingMainArea()) {}

  for (auto& req : xf8_loadTransactions)
    req->WaitUntilComplete();

  if (xdc_tokens.empty()) {
    VerifyTokenList(mgr);
    for (CToken& tok : xdc_tokens)
      tok.Lock();
    for (CToken& tok : xdc_tokens)
      tok.GetObj();
    xf0_26_tokensReady = true;
  }

  xf8_loadTransactions.clear();

  PostConstructArea();
  if (x4_selfIdx != kInvalidAreaId)
    mgr.GetWorld()->MoveAreaToAliveChain(x4_selfIdx);

  LoadScriptObjects(mgr);

  const CPVSAreaSet* pvs = x12c_postConstructed->xa0_pvs.get();
  if (pvs && x12c_postConstructed->x1108_29_pvsHasActors) {
    for (size_t i = 0; i < pvs->GetNumActors(); ++i) {
      const TEditorId entId = pvs->GetEntityIdByIndex(i) | (x4_selfIdx << 16);
      const TUniqueId id = mgr.GetIdForScript(entId);

      if (id == kInvalidUniqueId) {
        continue;
      }

      CPostConstructed::MapEntry& ent = x12c_postConstructed->xa8_pvsEntityMap[id.Value()];
      ent.x0_id = static_cast<s16>(i + (pvs->GetNumFeatures() - pvs->GetNumActors()));
      ent.x4_uid = id;
    }
  }

  xf0_28_validated = true;
  mgr.AreaLoaded(x4_selfIdx);
}

void CGameArea::LoadScriptObjects(CStateManager& mgr) {
  CScriptLayerManager& layerState = *mgr.WorldLayerState();
  u32 layerCount = layerState.GetAreaLayerCount(x4_selfIdx);
  std::vector<TEditorId> objIds;
  for (u32 i = 0; i < layerCount; ++i) {
    if (layerState.IsLayerActive(x4_selfIdx, i)) {
      auto layerBuf = GetLayerScriptBuffer(i);
      CMemoryInStream r(layerBuf.first, layerBuf.second);
      mgr.LoadScriptObjects(x4_selfIdx, r, objIds);
    }
  }
  mgr.InitScriptObjects(objIds);
}

std::pair<const u8*, u32> CGameArea::GetLayerScriptBuffer(int layer) const {
  if (!xf0_24_postConstructed)
    return {};
  return x12c_postConstructed->x110c_layerPtrs[layer];
}

void CGameArea::PostConstructArea() {
  SMREAHeader header = VerifyHeader();

  /* Materials */
  x12c_postConstructed->x10ec_firstMatSection = 2;

  /* Models */
  auto secIt = m_resolvedBufs.begin() + 3;
  x12c_postConstructed->x4c_insts.resize(header.modelCount);
  for (u32 i = 0; i < header.modelCount; ++i) {
    u32 surfCount = CBasics::SwapBytes(*reinterpret_cast<const u32*>((secIt + 6)->first));
    secIt += 7 + surfCount;
  }

  /* Render octree */
  if (header.version > 14 && header.arotSecIdx != -1) {
    x12c_postConstructed->xc_octTree.emplace(secIt->first);
    ++secIt;
  }

  /* Scriptable layer section */
  x12c_postConstructed->x10c8_sclyBuf = secIt->first;
  x12c_postConstructed->x10d0_sclySize = secIt->second;
  ++secIt;

  /* Collision section */
  x12c_postConstructed->x0_collision = CAreaOctTree::MakeFromMemory(secIt->first, secIt->second);
  ++secIt;

  /* Unknown section */
  ++secIt;

  /* Lights section */
  if (header.version > 6) {
    CMemoryInStream r(secIt->first, secIt->second, CMemoryInStream::EOwnerShip::NotOwned);
    u32 magic = r.ReadLong();
    u32 aCount = magic;
    if (magic == 0xBABEDEAD) {
      aCount = r.ReadLong();
    }
    x12c_postConstructed->x60_lightsA.reserve(aCount);
    x12c_postConstructed->x70_gfxLightsA.reserve(aCount);
    for (u32 i = 0; i < aCount; ++i) {
      x12c_postConstructed->x60_lightsA.emplace_back(r);
      x12c_postConstructed->x70_gfxLightsA.push_back(x12c_postConstructed->x60_lightsA.back().GetAsCGraphicsLight());
    }

    if (magic == 0xBABEDEAD) {
      u32 bCount = r.ReadLong();
      x12c_postConstructed->x80_lightsB.reserve(bCount);
      x12c_postConstructed->x90_gfxLightsB.reserve(bCount);
      for (u32 i = 0; i < bCount; ++i) {
        x12c_postConstructed->x80_lightsB.emplace_back(r);
        x12c_postConstructed->x90_gfxLightsB.push_back(x12c_postConstructed->x80_lightsB.back().GetAsCGraphicsLight());
      }
    }

    if (x12c_postConstructed->x80_lightsB.empty()) {
      x12c_postConstructed->x80_lightsB = x12c_postConstructed->x60_lightsA;
      x12c_postConstructed->x90_gfxLightsB = x12c_postConstructed->x70_gfxLightsA;
    }

    ++secIt;
  }

  /* PVS section */
  if (header.version > 7) {
    CMemoryInStream r(secIt->first, secIt->second, CMemoryInStream::EOwnerShip::NotOwned);
    if (secIt->second > 0) { // TODO this works around CMemoryInStream inf loop on 0 len
      u32 magic = r.ReadLong();
      if (magic == 'VISI') {
        x12c_postConstructed->x10a8_pvsVersion = r.ReadLong();
        if (x12c_postConstructed->x10a8_pvsVersion == 2) {
          x12c_postConstructed->x1108_29_pvsHasActors = r.ReadBool();
          x12c_postConstructed->x1108_30_ = r.ReadBool();
          x12c_postConstructed->xa0_pvs =
              std::make_unique<CPVSAreaSet>(secIt->first + r.GetReadPosition(), secIt->second - r.GetReadPosition());
        }
      }
    }

    ++secIt;
  }

  /* Pathfinding section */
  if (header.version > 9) {
    CMemoryInStream r(secIt->first, secIt->second, CMemoryInStream::EOwnerShip::NotOwned);
    CAssetId pathId = r.Get<CAssetId>();
    x12c_postConstructed->x10ac_pathToken = g_SimplePool->GetObj(SObjectTag{FOURCC('PATH'), pathId});
    x12c_postConstructed->x10bc_pathArea = x12c_postConstructed->x10ac_pathToken.GetObj();
    x12c_postConstructed->x10bc_pathArea->SetTransform(xc_transform);
    ++secIt;
  }

  x12c_postConstructed->x10c0_areaObjs = std::make_unique<CAreaObjectList>(x4_selfIdx);
  x12c_postConstructed->x10c4_areaFog = std::make_unique<CAreaFog>();

  /* URDE addition: preemptively fill in area models so shaders may be polled for completion */
//  if (!x12c_postConstructed->x1108_25_modelsConstructed)
//    FillInStaticGeometry();

  xf0_24_postConstructed = true;

  /* Resolve layer pointers */
  if (x12c_postConstructed->x10c8_sclyBuf != nullptr) {
    CMemoryInStream r(x12c_postConstructed->x10c8_sclyBuf, x12c_postConstructed->x10d0_sclySize,
                      CMemoryInStream::EOwnerShip::NotOwned);
    FourCC magic;
    r.Get(reinterpret_cast<u8*>(&magic), 4);
    if (magic == FOURCC('SCLY')) {
      r.ReadLong();
      u32 layerCount = r.ReadLong();
      x12c_postConstructed->x110c_layerPtrs.resize(layerCount);
      for (u32 l = 0; l < layerCount; ++l)
        x12c_postConstructed->x110c_layerPtrs[l].second = r.ReadLong();
      const u8* ptr = x12c_postConstructed->x10c8_sclyBuf + r.GetReadPosition();
      for (u32 l = 0; l < layerCount; ++l) {
        x12c_postConstructed->x110c_layerPtrs[l].first = ptr;
        ptr += x12c_postConstructed->x110c_layerPtrs[l].second;
      }
    }
  }
}

void CGameArea::FillInStaticGeometry() {
  u32 start = x12c_postConstructed->x10ec_firstMatSection;
  x12c_postConstructed->x10d4_firstMatPtr = x110_mreaSecBufs[start].first.get();

  // Clear the instances without resizing
  if (!x12c_postConstructed->x4c_insts.empty()) {
    for (CMetroidModelInstance& inst : x12c_postConstructed->x4c_insts) {
      inst = {};
    }
  }

  auto iter = m_resolvedBufs.begin() + start + 1;
  for (auto& inst : x12c_postConstructed->x4c_insts) {
    auto modelHeader = *iter++;
    auto positions = *iter++;
    auto normals = *iter++;
    auto colors = *iter++;
    auto texCoords = *iter++;
    auto packedTexCoords = *iter++;
    u32 surfaceCount = CBasics::SwapBytes(*reinterpret_cast<const u32*>(iter++->first));
    if (surfaceCount != 0) {
      std::vector<CCubeSurface> surfaces;
      surfaces.reserve(surfaceCount);
      for (int idx = 0; idx < surfaceCount; ++idx) {
        auto [ptr, len] = *iter++;
        surfaces.emplace_back(ptr, len);
      }
      inst = CMetroidModelInstance{
          modelHeader,     x12c_postConstructed->x10d4_firstMatPtr,
          positions,       normals,
          colors,          texCoords,
          packedTexCoords, std::move(surfaces),
      };
    }
  }

  x12c_postConstructed->x1108_25_modelsConstructed = true;
}

void CGameArea::VerifyTokenList(CStateManager& stateMgr) {
  if (xdc_tokens.size())
    return;
  ClearTokenList();

  if (xac_deps2.empty())
    return;

  auto end = xac_deps2.end();
  for (int lidx = int(xbc_layerDepOffsets.size() - 1); lidx >= 0; --lidx) {
    auto begin = xac_deps2.begin() + xbc_layerDepOffsets[lidx];
    if (stateMgr.WorldLayerState()->IsLayerActive(x4_selfIdx, lidx)) {
      for (auto it = begin; it != end; ++it) {
        xdc_tokens.push_back(g_SimplePool->GetObj(*it));
        xdc_tokens.back().Lock();
      }
    }
    end = begin;
  }
}

void CGameArea::ClearTokenList() {
  if (xdc_tokens.empty())
    xdc_tokens.reserve(xac_deps2.size());
  else
    xdc_tokens.clear();

  xf0_26_tokensReady = false;
}

u32 CGameArea::GetPreConstructedSize() const { return 0; }

SMREAHeader CGameArea::VerifyHeader() const {
  if (x110_mreaSecBufs.empty()) {
    return {};
  }

  u32 magic{};
  std::memcpy(&magic, x110_mreaSecBufs[0].first.get(), sizeof(u32));
  if (magic != SBIG(0xDEADBEEF)) {
    return {};
  }

  CMemoryInStream r(x110_mreaSecBufs[0].first.get() + 4, x110_mreaSecBufs[0].second - 4);
  u32 version = r.ReadLong();
  if ((version & 0x10000) != 0) {
    Log.report(logvisor::Fatal, FMT_STRING("Attempted to load non-retail MREA"));
  }

  SMREAHeader header;
  header.version = (version >= 12 && version <= 15) ? version : 0;
  if (!header.version) {
    return {};
  }

  header.xf = r.Get<zeus::CTransform>();
  header.modelCount = r.ReadLong();
  header.secCount = r.ReadLong();
  header.geomSecIdx = r.ReadLong();
  header.sclySecIdx = r.ReadLong();
  header.collisionSecIdx = r.ReadLong();
  header.unkSecIdx = r.ReadLong();
  header.lightSecIdx = r.ReadLong();
  header.visiSecIdx = r.ReadLong();
  header.pathSecIdx = r.ReadLong();
  header.arotSecIdx = r.ReadLong();

  return header;
}

TUniqueId CGameArea::LookupPVSUniqueID(TUniqueId id) const {
  return x12c_postConstructed->xa8_pvsEntityMap[id.Value()].x4_uid;
}

s16 CGameArea::LookupPVSID(TUniqueId id) const { return x12c_postConstructed->xa8_pvsEntityMap[id.Value()].x0_id; }

void CGameArea::SetAreaAttributes(const CScriptAreaAttributes* areaAttributes) {
  x12c_postConstructed->x10d8_areaAttributes = areaAttributes;
  if (areaAttributes == nullptr)
    return;

  x12c_postConstructed->x111c_thermalCurrent = areaAttributes->GetThermalHeat();
  x12c_postConstructed->x1128_worldLightingLevel = areaAttributes->GetWorldLightingLevel();
}

bool CGameArea::CAreaObjectList::IsQualified(const CEntity& ent) const {
  return (ent.GetAreaIdAlways() == x200c_areaIdx);
}

void CGameArea::DebugDraw() {
  if (!m_debugSphereRes) {
    const auto* tok = g_ResFactory->GetResourceIdByName("CMDL_DebugSphere");
    if (tok != nullptr && tok->type == FOURCC('CMDL')) {
      m_debugSphereRes = CStaticRes(tok->id, zeus::skOne3f);
    }
  }

  if (m_debugSphereRes && !m_debugSphereModel) {
    m_debugSphereModel = std::make_unique<CModelData>(*m_debugSphereRes);
  }

  if (!m_debugConeRes) {
    const auto* tok = g_ResFactory->GetResourceIdByName("CMDL_DebugLightCone");
    if (tok != nullptr && tok->type == FOURCC('CMDL')) {
      m_debugConeRes = CStaticRes(tok->id, zeus::skOne3f);
    }
  }

  if (m_debugConeRes && !m_debugConeModel) {
    m_debugConeModel = std::make_unique<CModelData>(*m_debugConeRes);
  }

  if (IsPostConstructed()) {
    for (const auto& light : x12c_postConstructed->x70_gfxLightsA) {
      DebugDrawLight(light);
    }
    for (const auto& light : x12c_postConstructed->x90_gfxLightsB) {
      DebugDrawLight(light);
    }
  }
}

void CGameArea::DebugDrawLight(const CLight& light) {
  if (light.GetType() == ELightType::LocalAmbient) {
    return;
  }
  g_Renderer->SetGXRegister1Color(light.GetColor());
  CModelFlags modelFlags;
  modelFlags.x0_blendMode = 5;
  modelFlags.x4_color = zeus::skWhite;
  modelFlags.x4_color.a() = 0.5f;
  if ((light.GetType() == ELightType::Spot || light.GetType() == ELightType::Directional) && m_debugConeModel) {
    m_debugConeModel->Render(CModelData::EWhichModel::Normal,
                             zeus::lookAt(light.GetPosition(), light.GetPosition() + light.GetDirection()) *
                                 zeus::CTransform::Scale(zeus::clamp(-90.f, light.GetRadius(), 90.f)),
                             nullptr, modelFlags);
  } else if (m_debugSphereModel) {
    m_debugSphereModel->Render(CModelData::EWhichModel::Normal, zeus::CTransform::Translate(light.GetPosition()),
                               nullptr, modelFlags);
    m_debugSphereModel->Render(CModelData::EWhichModel::Normal,
                               zeus::CTransform::Translate(light.GetPosition()) *
                                   zeus::CTransform::Scale(light.GetRadius()),
                               nullptr, modelFlags);
  }
}

} // namespace metaforce
