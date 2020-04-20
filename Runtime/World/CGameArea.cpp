#include "Runtime/World/CGameArea.hpp"

#include <array>

#include "Runtime/CGameState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CScriptAreaAttributes.hpp"

#include "DataSpec/DNAMP1/MREA.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

static logvisor::Module Log("CGameArea");

CAreaRenderOctTree::CAreaRenderOctTree(const u8* buf) : x0_buf(buf) {
  CMemoryInStream r(x0_buf + 8, INT32_MAX);
  x8_bitmapCount = r.readUint32Big();
  xc_meshCount = r.readUint32Big();
  x10_nodeCount = r.readUint32Big();
  x14_bitmapWordCount = (xc_meshCount + 31) / 32;
  x18_aabb.readBoundingBoxBig(r);

  x30_bitmaps = reinterpret_cast<const u32*>(x0_buf + 64);
  u32 wc = x14_bitmapWordCount * x8_bitmapCount;
  for (u32 i = 0; i < wc; ++i)
    const_cast<u32*>(x30_bitmaps)[i] = hecl::SBig(x30_bitmaps[i]);

  x34_indirectionTable = x30_bitmaps + wc;
  x38_entries = reinterpret_cast<const u8*>(x34_indirectionTable + x10_nodeCount);
  for (u32 i = 0; i < x10_nodeCount; ++i) {
    const_cast<u32*>(x34_indirectionTable)[i] = hecl::SBig(x34_indirectionTable[i]);
    Node* n = reinterpret_cast<Node*>(const_cast<u8*>(x38_entries) + x34_indirectionTable[i]);
    n->x0_bitmapIdx = hecl::SBig(n->x0_bitmapIdx);
    n->x2_flags = hecl::SBig(n->x2_flags);
    if (n->x2_flags) {
      u32 childCount = n->GetChildCount();
      for (u32 c = 0; c < childCount; ++c)
        n->x4_children[c] = hecl::SBig(n->x4_children[c]);
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
  const u32 count = in.readUint32Big();
  ret.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    ret.emplace_back().readMLVL(in);
  }
  return ret;
}

std::pair<std::unique_ptr<u8[]>, s32> GetScriptingMemoryAlways(const IGameArea& area) {
  SObjectTag tag = {SBIG('MREA'), area.IGetAreaAssetId()};
  std::unique_ptr<u8[]> data = g_ResFactory->LoadNewResourcePartSync(tag, 0, 96);

  if (*reinterpret_cast<u32*>(data.get()) != SBIG(0xDEADBEEF))
    return {};

  SMREAHeader header;
  CMemoryInStream r(data.get() + 4, 96 - 4);
  u32 version = r.readUint32Big();
  if (!(version & 0x10000))
    Log.report(logvisor::Fatal, FMT_STRING("Attempted to load non-URDE MREA"));

  version &= ~0x10000;
  header.version = (version >= 12 && version <= 15) ? version : 0;
  if (!header.version)
    return {};

  header.xf.read34RowMajor(r);
  header.modelCount = r.readUint32Big();
  header.secCount = r.readUint32Big();
  header.geomSecIdx = r.readUint32Big();
  header.sclySecIdx = r.readUint32Big();
  header.collisionSecIdx = r.readUint32Big();
  header.unkSecIdx = r.readUint32Big();
  header.lightSecIdx = r.readUint32Big();
  header.visiSecIdx = r.readUint32Big();
  header.pathSecIdx = r.readUint32Big();
  header.arotSecIdx = r.readUint32Big();

  u32 dataLen = ROUND_UP_32(header.secCount * 4);

  data = g_ResFactory->LoadNewResourcePartSync(tag, 96, dataLen);

  r = CMemoryInStream(data.get(), dataLen);

  std::vector<u32> secSizes(header.secCount);
  u32 lastSize;
  for (u32 i = 0; i < header.secCount; ++i) {
    lastSize = r.readUint32Big();
    secSizes.push_back(lastSize);
  }

  // TODO: Finish
  return {};
}

CDummyGameArea::CDummyGameArea(CInputStream& in, int idx, int mlvlVersion) {
  x8_nameSTRG = in.readUint32Big();
  x14_transform.read34RowMajor(in);
  zeus::CAABox aabb;
  aabb.readBoundingBoxBig(in);
  xc_mrea = in.readUint32Big();
  if (mlvlVersion > 15)
    x10_areaId = in.readUint32Big();

  u32 attachAreaCount = in.readUint32Big();
  x44_attachedAreaIndices.reserve(attachAreaCount);
  for (u32 i = 0; i < attachAreaCount; ++i)
    x44_attachedAreaIndices.push_back(in.readUint16Big());

  ::urde::ReadDependencyList(in);
  ::urde::ReadDependencyList(in);

  if (mlvlVersion > 13) {
    u32 depCount = in.readUint32Big();
    for (u32 i = 0; i < depCount; ++i)
      in.readUint32Big();
  }

  u32 dockCount = in.readUint32Big();
  x54_docks.reserve(dockCount);
  for (u32 i = 0; i < dockCount; ++i)
    x54_docks.emplace_back(in, x14_transform);
}

std::pair<std::unique_ptr<u8[]>, s32> CDummyGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

TAreaId CDummyGameArea::IGetAreaId() const { return x10_areaId; }

CAssetId CDummyGameArea::IGetAreaAssetId() const { return xc_mrea; }

bool CDummyGameArea::IIsActive() const { return true; }

TAreaId CDummyGameArea::IGetAttachedAreaId(int idx) const { return x44_attachedAreaIndices[idx]; }

u32 CDummyGameArea::IGetNumAttachedAreas() const { return x44_attachedAreaIndices.size(); }

CAssetId CDummyGameArea::IGetStringTableAssetId() const { return x8_nameSTRG; }

const zeus::CTransform& CDummyGameArea::IGetTM() const { return x14_transform; }

CGameArea::CGameArea(CInputStream& in, int idx, int mlvlVersion) : x4_selfIdx(idx) {
  x8_nameSTRG = in.readUint32Big();
  xc_transform.read34RowMajor(in);
  x3c_invTransform = xc_transform.inverse();
  x6c_aabb.readBoundingBoxBig(in);

  x84_mrea = in.readUint32Big();
  if (mlvlVersion > 15)
    x88_areaId = in.readInt32Big();
  else
    x88_areaId = -1;

  const u32 attachedCount = in.readUint32Big();
  x8c_attachedAreaIndices.reserve(attachedCount);
  for (u32 i = 0; i < attachedCount; ++i) {
    x8c_attachedAreaIndices.emplace_back(in.readUint16Big());
  }

  x9c_deps1 = ::urde::ReadDependencyList(in);
  xac_deps2 = ::urde::ReadDependencyList(in);

  const zeus::CAABox aabb = x6c_aabb.getTransformedAABox(xc_transform);
  x6c_aabb = aabb;

  if (mlvlVersion > 13) {
    const u32 depCount = in.readUint32Big();
    xbc_layerDepOffsets.reserve(depCount);
    for (u32 i = 0; i < depCount; ++i) {
      xbc_layerDepOffsets.emplace_back(in.readUint32Big());
    }
  }

  const u32 dockCount = in.readUint32Big();
  xcc_docks.reserve(dockCount);
  for (u32 i = 0; i < dockCount; ++i) {
    xcc_docks.emplace_back(in, xc_transform);
  }

  ClearTokenList();

  for (CToken& tok : xdc_tokens)
    xec_totalResourcesSize += g_ResFactory->ResourceSize(*tok.GetObjectTag());

  xec_totalResourcesSize += g_ResFactory->ResourceSize(SObjectTag{FOURCC('MREA'), x84_mrea});
}

CGameArea::CGameArea(CAssetId mreaId) : x84_mrea(mreaId), xf0_25_active{false} {
  while (StartStreamingMainArea())
    for (auto& req : xf8_loadTransactions)
      req->WaitUntilComplete();

  SMREAHeader header = VerifyHeader();
  x12c_postConstructed->x4c_insts.resize(header.modelCount);

  FillInStaticGeometry(false);

  CBooModel::SetDummyTextures(true);
  CBooModel::EnableShadowMaps(g_Renderer->x220_sphereRamp.get(), zeus::CTransform());
  CGraphics::CProjectionState backupProj = CGraphics::GetProjectionState();
  zeus::CTransform backupViewPoint = CGraphics::g_ViewMatrix;
  zeus::CTransform backupModel = CGraphics::g_GXModelMatrix;
  CGraphics::SetViewPointMatrix(zeus::CTransform::Translate(0.f, -2048.f, 0.f));
  CGraphics::SetOrtho(-2048.f, 2048.f, 2048.f, -2048.f, 0.f, 4096.f);
  CModelFlags defaultFlags;
  for (CMetroidModelInstance& inst : x12c_postConstructed->x4c_insts) {
    CGraphics::SetModelMatrix(zeus::CTransform::Translate(-inst.x34_aabb.center()));
    inst.m_instance->UpdateUniformData(defaultFlags, nullptr, nullptr);
    inst.m_instance->WarmupDrawSurfaces();
  }
  CGraphics::SetProjectionState(backupProj);
  CGraphics::SetViewPointMatrix(backupViewPoint);
  CGraphics::SetModelMatrix(backupModel);
  CBooModel::DisableShadowMaps();
  CBooModel::SetDummyTextures(false);
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
        unloaded = UnloadAllloadedTextures();
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
                                  x4_selfIdx, &x12c_postConstructed->m_materialSet);
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
      totalSz += hecl::SBig(reinterpret_cast<u32*>(x110_mreaSecBufs[1].first.get())[i]);

    AllocNewAreaData(x128_mreaDataOffset, totalSz);

    m_resolvedBufs.reserve(secCount);
    m_resolvedBufs.emplace_back(x110_mreaSecBufs[0].first.get(), x110_mreaSecBufs[0].second);
    m_resolvedBufs.emplace_back(x110_mreaSecBufs[1].first.get(), x110_mreaSecBufs[1].second);

    u32 curOff = 0;
    for (u32 i = 0; i < secCount; ++i) {
      u32 size = hecl::SBig(reinterpret_cast<u32*>(x110_mreaSecBufs[1].first.get())[i]);
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
  return hecl::SBig(*reinterpret_cast<u32*>(x110_mreaSecBufs[0].first.get() + 60));
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

  CPVSAreaSet* pvs = x12c_postConstructed->xa0_pvs.get();
  if (pvs && x12c_postConstructed->x1108_29_pvsHasActors) {
    for (int i = 0; i < pvs->GetNumActors(); ++i) {
      TEditorId entId = pvs->GetEntityIdByIndex(i) | (x4_selfIdx << 16);
      TUniqueId id = mgr.GetIdForScript(entId);
      if (id != kInvalidUniqueId) {
        CPostConstructed::MapEntry& ent = x12c_postConstructed->xa8_pvsEntityMap[id.Value()];
        ent.x0_id = i + (pvs->GetNumFeatures() - pvs->GetNumActors());
        ent.x4_uid = id;
      }
    }
  }

  xf0_28_validated = true;
  mgr.AreaLoaded(x4_selfIdx);
}

void CGameArea::LoadScriptObjects(CStateManager& mgr) {
  CWorldLayerState& layerState = *mgr.WorldLayerState();
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

  auto secIt = m_resolvedBufs.begin() + 2;

  /* Materials */
  ++secIt;

  u32 sec = 3;

  /* Models */
  x12c_postConstructed->x4c_insts.resize(header.modelCount);
  for (u32 i = 0; i < header.modelCount; ++i) {
    u32 surfCount = hecl::SBig(*reinterpret_cast<const u32*>((secIt + 4)->first));
    secIt += 5 + surfCount;
    sec += 5 + surfCount;
  }

  /* Render octree */
  if (header.version == 15 && header.arotSecIdx != -1) {
    x12c_postConstructed->xc_octTree.emplace(secIt->first);
    ++secIt;
  }

  /* Scriptable layer section */
  x12c_postConstructed->x10c8_sclyBuf = secIt->first;
  x12c_postConstructed->x10d0_sclySize = secIt->second;
  ++secIt;

  /* Collision section */
  std::unique_ptr<CAreaOctTree> collision = CAreaOctTree::MakeFromMemory(secIt->first, secIt->second);
  if (collision) {
    x12c_postConstructed->x0_collision = std::move(collision);
    x12c_postConstructed->x8_collisionSize = secIt->second;
  }
  ++secIt;

  /* Unknown section */
  ++secIt;

  /* Lights section */
  if (header.version > 6) {
    athena::io::MemoryReader r(secIt->first, secIt->second);
    u32 magic = r.readUint32Big();
    if (magic == 0xBABEDEAD) {
      u32 aCount = r.readUint32Big();
      x12c_postConstructed->x60_lightsA.reserve(aCount);
      x12c_postConstructed->x70_gfxLightsA.reserve(aCount);
      for (u32 i = 0; i < aCount; ++i) {
        x12c_postConstructed->x60_lightsA.emplace_back(r);
        x12c_postConstructed->x70_gfxLightsA.push_back(x12c_postConstructed->x60_lightsA.back().GetAsCGraphicsLight());
      }

      u32 bCount = r.readUint32Big();
      x12c_postConstructed->x80_lightsB.reserve(bCount);
      x12c_postConstructed->x90_gfxLightsB.reserve(bCount);
      for (u32 i = 0; i < bCount; ++i) {
        x12c_postConstructed->x80_lightsB.emplace_back(r);
        x12c_postConstructed->x90_gfxLightsB.push_back(x12c_postConstructed->x80_lightsB.back().GetAsCGraphicsLight());
      }
    }

    ++secIt;
  }

  /* PVS section */
  if (header.version > 7) {
    athena::io::MemoryReader r(secIt->first, secIt->second);
    u32 magic = r.readUint32Big();
    if (magic == 'VISI') {
      x12c_postConstructed->x10a8_pvsVersion = r.readUint32Big();
      if (x12c_postConstructed->x10a8_pvsVersion == 2) {
        x12c_postConstructed->x1108_29_pvsHasActors = r.readBool();
        x12c_postConstructed->x1108_30_ = r.readBool();
        x12c_postConstructed->xa0_pvs =
            std::make_unique<CPVSAreaSet>(secIt->first + r.position(), secIt->second - r.position());
      }
    }

    ++secIt;
  }

  /* Pathfinding section */
  if (header.version > 9) {
    athena::io::MemoryReader r(secIt->first, secIt->second);
    CAssetId pathId = r.readUint32Big();
    x12c_postConstructed->x10ac_pathToken = g_SimplePool->GetObj(SObjectTag{FOURCC('PATH'), pathId});
    x12c_postConstructed->x10bc_pathArea = x12c_postConstructed->x10ac_pathToken.GetObj();
    x12c_postConstructed->x10bc_pathArea->SetTransform(xc_transform);
    ++secIt;
  }

  x12c_postConstructed->x10c0_areaObjs = std::make_unique<CAreaObjectList>(x4_selfIdx);
  x12c_postConstructed->x10c4_areaFog = std::make_unique<CAreaFog>();

  /* URDE addition: preemptively fill in area models so shaders may be polled for completion */
  if (!x12c_postConstructed->x1108_25_modelsConstructed)
    FillInStaticGeometry();

  xf0_24_postConstructed = true;

  /* Resolve layer pointers */
  if (x12c_postConstructed->x10c8_sclyBuf) {
    athena::io::MemoryReader r(x12c_postConstructed->x10c8_sclyBuf, x12c_postConstructed->x10d0_sclySize);
    hecl::DNAFourCC magic;
    magic.read(r);
    if (magic == FOURCC('SCLY')) {
      r.readUint32Big();
      u32 layerCount = r.readUint32Big();
      x12c_postConstructed->x110c_layerPtrs.resize(layerCount);
      for (u32 l = 0; l < layerCount; ++l)
        x12c_postConstructed->x110c_layerPtrs[l].second = r.readUint32Big();
      const u8* ptr = x12c_postConstructed->x10c8_sclyBuf + r.position();
      for (u32 l = 0; l < layerCount; ++l) {
        x12c_postConstructed->x110c_layerPtrs[l].first = ptr;
        ptr += x12c_postConstructed->x110c_layerPtrs[l].second;
      }
    }
  }
}

void CGameArea::FillInStaticGeometry(bool textures) {
  if (!x12c_postConstructed->x4c_insts.empty())
    for (CMetroidModelInstance& inst : x12c_postConstructed->x4c_insts)
      inst.Clear();

  /* Materials */
  SShader& matSet = x12c_postConstructed->m_materialSet;
  auto secIt = m_resolvedBufs.begin() + 2;
  {
    athena::io::MemoryReader r(secIt->first, secIt->second);
    matSet.m_matSet.read(r);
    if (textures)
      CBooModel::MakeTexturesFromMats(matSet.m_matSet, matSet.x0_textures, *g_SimplePool);
    matSet.InitializeLayout(nullptr);
    ++secIt;
  }
  
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    /* Reserve extra buffers for 16 cubemaps and shadow rendering */
    matSet.m_geomLayout->ReserveSharedBuffers(ctx, 96 + int(EWorldShadowMode::MAX));

    /* Models */
    for (CMetroidModelInstance& inst : x12c_postConstructed->x4c_insts) {
      {
        DataSpec::DNAMP1::MREA::MeshHeader header;
        athena::io::MemoryReader r(secIt->first, secIt->second);
        header.read(r);
        inst.x0_visorFlags = header.visorFlags.flags;
        inst.x4_xf = header.xfMtx;
        inst.x34_aabb = zeus::CAABox(header.aabb[0], header.aabb[1]);
        ++secIt;
      }

      {
        athena::io::MemoryReader r(secIt->first, secIt->second);
        inst.m_hmdlMeta.read(r);
      }
      ++secIt;

      boo::ObjToken<boo::IGraphicsBufferS> vbo;
      boo::ObjToken<boo::IGraphicsBufferS> ibo;
      vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, secIt->first, inst.m_hmdlMeta.vertStride,
                                inst.m_hmdlMeta.vertCount);
      ++secIt;
      ibo = ctx.newStaticBuffer(boo::BufferUse::Index, secIt->first, 4, inst.m_hmdlMeta.indexCount);
      ++secIt;

      const u32 surfCount = hecl::SBig(*reinterpret_cast<const u32*>(secIt->first));
      inst.m_surfaces.reserve(surfCount);
      inst.m_shaders.reserve(surfCount);
      ++secIt;
      for (u32 j = 0; j < surfCount; ++j) {
        CBooSurface& surf = inst.m_surfaces.emplace_back();
        surf.selfIdx = j;
        athena::io::MemoryReader r(secIt->first, secIt->second);
        surf.m_data.read(r);
        ++secIt;
      }

      TToken<CModel> nullModel;
      inst.m_instance = std::make_unique<CBooModel>(nullModel, nullptr, &inst.m_surfaces, matSet, vbo, ibo,
                                                    inst.x34_aabb, inst.x0_visorFlags, 0);
    }

    return true;
  } BooTrace);

  for (CMetroidModelInstance& inst : x12c_postConstructed->x4c_insts) {
    for (CBooSurface& surf : inst.m_surfaces) {
      auto& shad = inst.m_shaders[surf.m_data.matIdx];
      if (!shad)
        shad = matSet.BuildShader(inst.m_hmdlMeta, matSet.m_matSet.materials[surf.m_data.matIdx]);
    }
    inst.m_instance->RemapMaterialData(matSet, inst.m_shaders);
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
  if (x110_mreaSecBufs.empty())
    return {};
  if (*reinterpret_cast<u32*>(x110_mreaSecBufs[0].first.get()) != SBIG(0xDEADBEEF))
    return {};

  SMREAHeader header;
  CMemoryInStream r(x110_mreaSecBufs[0].first.get() + 4, x110_mreaSecBufs[0].second - 4);
  u32 version = r.readUint32Big();
  if (!(version & 0x10000))
    Log.report(logvisor::Fatal, FMT_STRING("Attempted to load non-URDE MREA"));
  version &= ~0x10000;
  header.version = (version >= 12 && version <= 15) ? version : 0;
  if (!header.version)
    return {};

  header.xf.read34RowMajor(r);
  header.modelCount = r.readUint32Big();
  header.secCount = r.readUint32Big();
  header.geomSecIdx = r.readUint32Big();
  header.sclySecIdx = r.readUint32Big();
  header.collisionSecIdx = r.readUint32Big();
  header.unkSecIdx = r.readUint32Big();
  header.lightSecIdx = r.readUint32Big();
  header.visiSecIdx = r.readUint32Big();
  header.pathSecIdx = r.readUint32Big();
  header.arotSecIdx = r.readUint32Big();

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

bool CGameArea::CAreaObjectList::IsQualified(const CEntity& ent) const { return (ent.GetAreaIdAlways() == x200c_areaIdx); }
void CGameArea::WarmupShaders(const SObjectTag& mreaTag) {
  // Calling this version of the constructor performs warmup implicitly
  CGameArea area(mreaTag.id);
}

} // namespace urde
