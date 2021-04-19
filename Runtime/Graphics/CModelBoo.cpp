#include "Runtime/Graphics/CModel.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"

#include <array>

#include <hecl/CVarManager.hpp>
#include <hecl/HMDLMeta.hpp>
#include <hecl/Runtime.hpp>
#include <logvisor/logvisor.hpp>

namespace metaforce {
namespace {
logvisor::Module Log("metaforce::CBooModel");
CBooModel* g_FirstModel = nullptr;

constexpr zeus::CMatrix4f ReflectBaseMtx{
    0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f,
};

constexpr zeus::CMatrix4f MBShadowPost0{
    1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f,
};

constexpr zeus::CMatrix4f MBShadowPost1{
    0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, -0.0625f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f,
};

constexpr zeus::CMatrix4f DisintegratePost{
    1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f,
};
} // Anonymous namespace

void CBooModel::Shutdown() {
  g_shadowMap.reset();
  g_disintegrateTexture.reset();
  g_reflectionCube.reset();
  assert(g_FirstModel == nullptr && "Dangling CBooModels detected");
}

void CBooModel::ClearModelUniformCounters() {
  OPTICK_EVENT();
  for (CBooModel* model = g_FirstModel; model; model = model->m_next)
    model->ClearUniformCounter();
}

void CBooModel::SetNewPlayerPositionAndTime(const zeus::CVector3f& pos) {
  g_PlayerPosition = pos;
  KillCachedViewDepState();
  u32 modMillis =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
          .count() %
      u64(100000.f * 4.f * M_PIF / 3.f);
  g_ModSeconds = modMillis / 1000.f;
  g_TransformedTime = 1.f / -(0.05f * std::sin(g_ModSeconds * 1.5f) - 1.f);
  g_TransformedTime2 = 1.f / -(0.015f * std::sin(g_ModSeconds * 1.5f + 1.f) - 1.f);
}

void CBooModel::KillCachedViewDepState() { g_LastModelCached = nullptr; }

void CBooModel::EnsureViewDepStateCached(const CBooModel& model, const CBooSurface* surf,
                                         hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::ReflectMtx>>& buf) {
#if HSH_PROFILE_MODE
  return;
#endif

  zeus::CVector3f modelToPlayer = g_PlayerPosition - CGraphics::g_GXModelMatrix.origin;
  zeus::CVector3f modelToPlayerLocal = CGraphics::g_GXModelMatrix.transposeRotate(modelToPlayer);

  zeus::CVector3f surfPos;
  float surfSize = 0.f;
  if (surf) {
    zeus::CVector3f surfCenter(surf->m_data.centroid);
    zeus::CVector3f surfNormal(surf->m_data.reflectionNormal);
    float dotDelta = surfNormal.dot(modelToPlayerLocal) - surfCenter.dot(surfNormal);
    surfPos = modelToPlayerLocal - surfNormal * dotDelta;
  } else {
    surfPos = model.x20_aabb.center();
    surfSize =
        (model.x20_aabb.max.x() - model.x20_aabb.min.x()) + (model.x20_aabb.max.y() - model.x20_aabb.min.y()) * 0.5f;
  }

  if (g_Renderer->x318_24_refectionDirty) {
    zeus::CVector3f playerToPos = g_ReflectViewPos - g_PlayerPosition;
    zeus::CVector3f vecToPos = surfPos - g_PlayerPosition;
    if (playerToPos.dot(playerToPos) < vecToPos.dot(vecToPos))
      g_ReflectViewPos = surfPos;
  } else {
    g_ReflectViewPos = surfPos;
    g_Renderer->x318_24_refectionDirty = true;
  }

  zeus::CVector3f playerToSurf = surfPos - modelToPlayerLocal;
  float distance = std::max(-(0.5f * surfSize - playerToSurf.magnitude()), FLT_EPSILON);

  auto* out = buf.map();
  if (distance >= 5.f) {
    out->reflectAlpha = 0.f;
  } else {
    out->reflectAlpha = (5.f - distance) / 5.f;

    /* Indirect map matrix */
    out->indMtx = (CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix).toMatrix4f();

    /* Reflection map matrix */
    zeus::CVector3f v1 = playerToSurf * (1.f / surfSize);
    zeus::CVector3f v2 = v1.cross(zeus::skUp);
    if (v2.canBeNormalized())
      v2.normalize();
    else
      v2 = zeus::skRight;

    float timeScale = 0.32258067f * (0.02f * distance + 1.f);
    float f1 = timeScale * g_TransformedTime;
    float f2 = timeScale * g_TransformedTime2;
    out->reflectMtx = ReflectBaseMtx;
    out->reflectMtx[0][0] = f1 * v2.x();
    out->reflectMtx[1][0] = f1 * v2.y();
    out->reflectMtx[3][0] = -surfPos.dot(v2) * f1 + 0.5f;
    out->reflectMtx[2][1] = f2;
    out->reflectMtx[3][1] = -modelToPlayerLocal.z() * f2;
  }
  buf.unmap();
}

hsh::texture2d CBooModel::g_shadowMap;
zeus::CTransform CBooModel::g_shadowTexXf;
hsh::texture2d CBooModel::g_disintegrateTexture;
hsh::texturecube CBooModel::g_reflectionCube;

void CBooModel::EnableShadowMaps(hsh::texture2d map, const zeus::CTransform& texXf) {
  g_shadowMap = map;
  g_shadowTexXf = texXf;
}
void CBooModel::DisableShadowMaps() { g_shadowMap.reset(); }

CBooModel::~CBooModel() {
  if (m_prev)
    m_prev->m_next = m_next;
  if (m_next)
    m_next->m_prev = m_prev;
  if (this == g_FirstModel)
    g_FirstModel = m_next;
}

CBooModel::CBooModel(TToken<CModel>& token, CModel* parent, std::vector<CBooSurface>* surfaces, SShader& shader,
                     hsh::vertex_buffer_typeless vbo, hsh::index_buffer<u32> ibo, const zeus::CAABox& aabb,
                     u8 renderMask, int numInsts, VertexFormat vtxFmt)
: m_modelTok(token)
, m_model(parent)
, x0_surfaces(surfaces)
, x4_matSet(&shader.m_matSet)
, m_geomLayout(&*shader.m_geomLayout)
, m_vtxFmt(vtxFmt)
, m_matSetIdx(shader.m_matSetIdx)
, x1c_textures(shader.x0_textures)
, x20_aabb(aabb)
, x41_mask(renderMask)
, m_staticVbo(vbo)
, m_staticIbo(ibo) {
  if (!g_FirstModel)
    g_FirstModel = this;
  else {
    g_FirstModel->m_prev = this;
    m_next = g_FirstModel;
    g_FirstModel = this;
  }

  for (CBooSurface& surf : *x0_surfaces)
    surf.m_parent = this;

  for (auto it = x0_surfaces->rbegin(); it != x0_surfaces->rend(); ++it) {
    u32 matId = it->m_data.matIdx;
    const MaterialSet::Material& matData = GetMaterialByIndex(matId);
    if (matData.flags.depthSorting()) {
      it->m_next = x3c_firstSortedSurface;
      x3c_firstSortedSurface = &*it;
    } else {
      it->m_next = x38_firstUnsortedSurface;
      x38_firstUnsortedSurface = &*it;
    }
  }

  m_instances.reserve(numInsts);
  for (int i = 0; i < numInsts; ++i)
    PushNewModelInstance();
}

hsh::vertex_buffer_typeless ModelInstance::GetBooVBO(const CBooModel& model) const {
  if (model.m_staticVbo)
    return model.m_staticVbo;
  if (!m_dynamicVbo && model.m_model) {
    const CModel& parent = *model.m_model;
    const hecl::HMDLMeta& meta = parent.GetHMDLMeta();
    MapVertData(meta, [this, meta]<typename VertData>() {
      m_dynamicVbo = hsh::create_dynamic_vertex_buffer<VertData>(meta.vertCount);
    });
#if !HSH_PROFILE_MODE
    m_dynamicVbo.load<uint8_t>({parent.GetDynamicVertexData(), meta.vertStride * meta.vertCount});
#endif
  }
  return m_dynamicVbo.get();
}

GeometryUniformLayout::GeometryUniformLayout(const CModel* model, const MaterialSet* matSet) {
  if (model) {
    m_skinBankCount = model->m_hmdlMeta.bankCount;
    m_weightVecCount = model->m_hmdlMeta.weightCount;
  }
}

ModelInstance* CBooModel::PushNewModelInstance(int sharedLayoutBuf, boo::IGraphicsDataFactory::Context* ctx) {
  OPTICK_EVENT();
  if (!x40_24_texturesLoaded && !g_DummyTextures) {
    return nullptr;
  }

  if (m_instances.size() >= 512) {
    Log.report(logvisor::Fatal, FMT_STRING("Model buffer overflow"));
  }

  ModelInstance& newInst = m_instances.emplace_back();

  newInst.m_geometryUniforms.resize(std::max(m_geomLayout->m_skinBankCount, size_t(1)));
#if !HSH_PROFILE_MODE
  for (auto& uniform : newInst.m_geometryUniforms)
    uniform = m_geomLayout->AllocateVertUniformBuffer();
#endif

  newInst.m_tcgUniforms.resize(x4_matSet->materialCount);
#if !HSH_PROFILE_MODE
  for (auto& uniform : newInst.m_tcgUniforms) {
    uniform = hsh::create_dynamic_uniform_buffer<CModelShaders::TCGMatrixUniform>();
    uniform.map()->fill({zeus::CMatrix4f{}, zeus::CMatrix4f{}});
    uniform.unmap();
  }
#endif

  newInst.m_reflectUniforms.resize(x0_surfaces->size());
#if !HSH_PROFILE_MODE
  for (auto& uniform : newInst.m_reflectUniforms) {
    uniform = hsh::create_dynamic_uniform_buffer<CModelShaders::ReflectMtx>();
    uniform.load({zeus::CMatrix4f{}, zeus::CMatrix4f{}, 0.f});
  }
#endif

  /* Allocate resident buffer */
#if !HSH_PROFILE_MODE
  newInst.m_fragmentUniform = hsh::create_dynamic_uniform_buffer<CModelShaders::FragmentUniform>();
  newInst.m_fragmentUniform.load({});
#endif

  /* Binding for each surface */
  newInst.m_shaderDataBindings.resize(x0_surfaces->size());

  return &newInst;
}

void CBooModel::MakeTexturesFromMats(const MaterialSet& matSet,
                                     std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut,
                                     IObjectStore& store) {
  for (const auto& mat : matSet.materials) {
    for (const auto& chunk : mat.chunks) {
      if (const auto* const pass = chunk.get_if<MaterialSet::Material::PASS>()) {
        toksOut.emplace(std::make_pair(pass->texId.toUint32(), store.GetObj({SBIG('TXTR'), pass->texId.toUint32()})));
      }
    }
  }
}

void CBooModel::MakeTexturesFromMats(std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut,
                                     IObjectStore& store) {
  MakeTexturesFromMats(*x4_matSet, toksOut, store);
}

void CBooModel::ActivateLights(const std::vector<CLight>& lights) {
  m_lightingData.ActivateLights(lights);
  m_lightsActive = true;
}

void CBooModel::DisableAllLights() {
  m_lightingData.ambient = zeus::skBlack;
  m_lightsActive = false;

  for (size_t curLight = 0; curLight < m_lightingData.lights.size(); ++curLight) {
    CModelShaders::Light& lightOut = m_lightingData.lights[curLight];
    lightOut.color = zeus::skClear;
    lightOut.linAtt[0] = 1.f;
    lightOut.angAtt[0] = 1.f;
  }
}

void CBooModel::RemapMaterialData(SShader& shader) {
  if (!shader.m_geomLayout)
    return;
  x4_matSet = &shader.m_matSet;
  m_geomLayout = &*shader.m_geomLayout;
  m_matSetIdx = shader.m_matSetIdx;
  x1c_textures = shader.x0_textures;
  x40_24_texturesLoaded = false;
  m_instances.clear();
}

bool CBooModel::TryLockTextures() {
  if (!x40_24_texturesLoaded) {
    bool allLoad = true;
    for (auto& tex : x1c_textures) {
      tex.second.Lock();
      if (!tex.second.IsLoaded()) {
        allLoad = false;
      }
    }

    x40_24_texturesLoaded = allLoad;
  }

  return x40_24_texturesLoaded;
}

void CBooModel::UnlockTextures() {
  m_instances.clear();

  for (auto& tex : x1c_textures) {
    tex.second.Unlock();
  }

  x40_24_texturesLoaded = false;
}

void CBooModel::SyncLoadTextures() {
  if (x40_24_texturesLoaded) {
    return;
  }

  for (auto& tex : x1c_textures) {
    tex.second.GetObj();
  }

  x40_24_texturesLoaded = true;
}

void CBooModel::DrawFlat(ESurfaceSelection sel, EExtendedShader extendedIdx) const {
  const CBooSurface* surf;
  CModelFlags flags = {};
  // TODO
  // flags.m_extendedShader = extendedIdx;

  if (sel != ESurfaceSelection::SortedOnly) {
    surf = x38_firstUnsortedSurface;
    while (surf) {
      DrawSurface(*surf, flags);
      surf = surf->m_next;
    }
  }

  if (sel != ESurfaceSelection::UnsortedOnly) {
    surf = x3c_firstSortedSurface;
    while (surf) {
      DrawSurface(*surf, flags);
      surf = surf->m_next;
    }
  }
}

void CBooModel::DrawAlphaSurfaces(const CModelFlags& flags) const {
  const CBooSurface* surf = x3c_firstSortedSurface;
  while (surf) {
    DrawSurface(*surf, flags);
    surf = surf->m_next;
  }
}

void CBooModel::DrawNormalSurfaces(const CModelFlags& flags) const {
  const CBooSurface* surf = x38_firstUnsortedSurface;
  while (surf) {
    DrawSurface(*surf, flags);
    surf = surf->m_next;
  }
}

void CBooModel::DrawSurfaces(const CModelFlags& flags) const {
  const CBooSurface* surf = x38_firstUnsortedSurface;
  while (surf) {
    DrawSurface(*surf, flags);
    surf = surf->m_next;
  }

  surf = x3c_firstSortedSurface;
  while (surf) {
    DrawSurface(*surf, flags);
    surf = surf->m_next;
  }
}

void CBooModel::DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const {
  // if (m_uniUpdateCount == 0)
  //    Log.report(logvisor::Fatal, FMT_STRING("UpdateUniformData() not called"));
  if (m_uniUpdateCount == 0 || m_uniUpdateCount > m_instances.size())
    return;
  ModelInstance& inst = const_cast<ModelInstance&>(m_instances[m_uniUpdateCount - 1]);

  const MaterialSet::Material& data = GetMaterialByIndex(surf.m_data.matIdx);
  if (data.flags.shadowOccluderMesh() && !g_DrawingOccluders)
    return;

  auto& binding = const_cast<hsh::binding&>(inst.m_shaderDataBindings[surf.selfIdx]);
  CModelShaders::SetCurrent(binding, flags, *this, inst, surf);
  if (binding) {
    binding.draw_indexed(surf.m_data.idxStart, surf.m_data.idxCount);
  } else {
    hsh::profile_context::instance.write_headers();
  }
}

void CBooModel::WarmupDrawSurfaces(const CModelFlags& unsortedFlags, const CModelFlags& sortedFlags) const {
  const CBooSurface* surf = x38_firstUnsortedSurface;
  while (surf) {
    WarmupDrawSurface(*surf, unsortedFlags);
    surf = surf->m_next;
  }

  surf = x3c_firstSortedSurface;
  while (surf) {
    WarmupDrawSurface(*surf, sortedFlags);
    surf = surf->m_next;
  }
}

void CBooModel::WarmupDrawSurface(const CBooSurface& surf, const CModelFlags& flags) const {
  if (m_uniUpdateCount > m_instances.size())
    return;
  const ModelInstance& inst = m_instances[m_uniUpdateCount - 1];

  auto& binding = const_cast<hsh::binding&>(inst.m_shaderDataBindings[surf.selfIdx]);
  CModelShaders::SetCurrent(binding, flags, *this, inst, surf);
  if (binding)
    binding.draw_indexed(surf.m_data.idxStart, surf.m_data.idxCount);
}

void CBooModel::UVAnimationBuffer::ProcessAnimation(CModelShaders::TCGMatrix& tcg,
                                                    const MaterialSet::Material::PASS& anim) {
  using UVAnimType = MaterialSet::Material::BlendMaterial::UVAnimType;
  if (anim.uvAnimType == UVAnimType::Invalid)
    return;
  tcg.mtx = zeus::CMatrix4f();
  tcg.postMtx = zeus::CMatrix4f();
  switch (anim.uvAnimType) {
  case UVAnimType::MvInvNoTranslation: {
    tcg.mtx = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    tcg.mtx[3].w = 1.f;
    tcg.postMtx[0].x = 0.5f;
    tcg.postMtx[1].y = 0.5f;
    tcg.postMtx[3].x = 0.5f;
    tcg.postMtx[3].y = 0.5f;
    break;
  }
  case UVAnimType::MvInv: {
    tcg.mtx = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    tcg.mtx[3] = hsh::float4{CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix.origin, 1.f};
    tcg.postMtx[0].x = 0.5f;
    tcg.postMtx[1].y = 0.5f;
    tcg.postMtx[3].x = 0.5f;
    tcg.postMtx[3].y = 0.5f;
    break;
  }
  case UVAnimType::Scroll: {
    tcg.mtx[3].x = CGraphics::GetSecondsMod900() * anim.uvAnimParms[2] + anim.uvAnimParms[0];
    tcg.mtx[3].y = CGraphics::GetSecondsMod900() * anim.uvAnimParms[3] + anim.uvAnimParms[1];
    break;
  }
  case UVAnimType::Rotation: {
    float angle = CGraphics::GetSecondsMod900() * anim.uvAnimParms[1] + anim.uvAnimParms[0];
    float acos = std::cos(angle);
    float asin = std::sin(angle);
    tcg.mtx[0].x = acos;
    tcg.mtx[0].y = asin;
    tcg.mtx[1].x = -asin;
    tcg.mtx[1].y = acos;
    tcg.mtx[3].x = (1.0f - (acos - asin)) * 0.5f;
    tcg.mtx[3].y = (1.0f - (asin + acos)) * 0.5f;
    break;
  }
  case UVAnimType::HStrip: {
    float value = anim.uvAnimParms[0] * anim.uvAnimParms[2] * (anim.uvAnimParms[3] + CGraphics::GetSecondsMod900());
    tcg.mtx[3].x = std::trunc(anim.uvAnimParms[1] * fmod(value, 1.0f)) * anim.uvAnimParms[2];
    break;
  }
  case UVAnimType::VStrip: {
    float value = anim.uvAnimParms[0] * anim.uvAnimParms[2] * (anim.uvAnimParms[3] + CGraphics::GetSecondsMod900());
    tcg.mtx[3].y = std::trunc(anim.uvAnimParms[1] * fmod(value, 1.0f)) * anim.uvAnimParms[2];
    break;
  }
  case UVAnimType::Model: {
    tcg.mtx = CGraphics::g_GXModelMatrix.toMatrix4f();
    tcg.mtx[3] = hsh::float4{0.f, 0.f, 0.f, 1.f};
    tcg.postMtx[0].x = 0.5f;
    tcg.postMtx[1].y = 0.f;
    tcg.postMtx[2].y = 0.5f;
    tcg.postMtx[3].x = CGraphics::g_GXModelMatrix.origin.x() * 0.05f;
    tcg.postMtx[3].y = CGraphics::g_GXModelMatrix.origin.y() * 0.05f;
    break;
  }
  case UVAnimType::CylinderEnvironment: {
    tcg.mtx = CGraphics::g_GXModelViewInvXpose.toMatrix4f();

    const zeus::CVector3f& viewOrigin = CGraphics::g_ViewMatrix.origin;
    float xy = (viewOrigin.x() + viewOrigin.y()) * 0.025f * anim.uvAnimParms[1];
    xy = (xy - std::trunc(xy));
    float z = (viewOrigin.z()) * 0.05f * anim.uvAnimParms[1];
    z = (z - std::trunc(z));

    float halfA = anim.uvAnimParms[0] * 0.5f;

    tcg.postMtx =
        zeus::CTransform(zeus::CMatrix3f(halfA, 0.0, 0.0, 0.0, 0.0, halfA, 0.0, 0.0, 0.0), zeus::CVector3f(xy, z, 1.0))
            .toMatrix4f();
    break;
  }
  default:
    break;
  }
}

void CBooModel::UVAnimationBuffer::Update(
    std::vector<hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::TCGMatrixUniform>>>& uniforms,
    const MaterialSet* matSet, const CModelFlags& flags, const CBooModel* parent) {
#if HSH_PROFILE_MODE
  return;
#endif

  if (flags.m_postType == EPostType::MBShadow) {
    /* Special matrices for MorphBall shadow rendering */
    zeus::CMatrix4f texMtx = (zeus::CTransform::Scale(1.f / (flags.m_mbShadowBox.max - flags.m_mbShadowBox.min)) *
                              zeus::CTransform::Translate(-flags.m_mbShadowBox.min) * CGraphics::g_GXModelMatrix)
                                 .toMatrix4f();
    for (size_t i = 0; i < matSet->materialCount; ++i) {
      auto& buf = uniforms[i];
      auto* out = buf.map();
      out->at(0) = {texMtx, MBShadowPost0};
      out->at(1) = {texMtx, MBShadowPost1};
      buf.unmap();
    }
    return;
  } else if (flags.m_postType == EPostType::Disintegrate) {
    assert(parent != nullptr && "Parent CBooModel not set");
    zeus::CTransform xf = zeus::CTransform::RotateX(-zeus::degToRad(45.f));
    zeus::CAABox aabb = parent->GetAABB().getTransformedAABox(xf);
    xf = zeus::CTransform::Scale(5.f / (aabb.max - aabb.min)) * zeus::CTransform::Translate(-aabb.min) * xf;
    zeus::CMatrix4f texMtx = xf.toMatrix4f();
    zeus::CMatrix4f post0 = DisintegratePost;
    post0[3].x() = flags.m_addColor.a();
    post0[3].y() = 6.f * -(1.f - flags.m_addColor.a()) + 1.f;
    zeus::CMatrix4f post1 = DisintegratePost;
    post1[3].x() = -0.85f * flags.m_addColor.a() - 0.15f;
    post1[3].y() = float(post0[3].y());
    /* Special matrices for disintegration rendering */
    for (size_t i = 0; i < matSet->materialCount; ++i) {
      auto& buf = uniforms[i];
      auto* out = buf.map();
      out->at(0) = {texMtx, post0};
      out->at(1) = {texMtx, post1};
      buf.unmap();
    }
    return;
  }

  std::optional<CModelShaders::TCGMatrix> specialMtxOut;
  if (flags.m_postType == EPostType::ThermalHot) {
    /* Special Mode0 matrix for exclusive Thermal Visor use */
    specialMtxOut.emplace();

    auto& texMtxOut = specialMtxOut->mtx;
    texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    texMtxOut[3] = {0.f, 0.f, 0.f, 1.f};

    auto& postMtxOut = specialMtxOut->postMtx;
    postMtxOut[0].x = 0.5f;
    postMtxOut[1].y = 0.5f;
    postMtxOut[3].x = 0.5f;
    postMtxOut[3].y = 0.5f;
  }
  // TODO
  // else if (flags.m_extendedShader == EExtendedShader::WorldShadow ||
  //             flags.m_extendedShader == EExtendedShader::LightingCubeReflectionWorldShadow) {
  //    /* Special matrix for mapping world shadow */
  //    specialMtxOut.emplace();
  //
  //    zeus::CMatrix4f mat = g_shadowTexXf.toMatrix4f();
  //    zeus::CMatrix4f& texMtxOut = (*specialMtxOut)[0];
  //    texMtxOut[0][0] = float(mat[0][0]);
  //    texMtxOut[1][0] = float(mat[1][0]);
  //    texMtxOut[2][0] = float(mat[2][0]);
  //    texMtxOut[3][0] = float(mat[3][0]);
  //    texMtxOut[0][1] = float(mat[0][2]);
  //    texMtxOut[1][1] = float(mat[1][2]);
  //    texMtxOut[2][1] = float(mat[2][2]);
  //    texMtxOut[3][1] = float(mat[3][2]);
  //  }

  for (size_t i = 0; i < matSet->materialCount; ++i) {
    const MaterialSet::Material& mat = matSet->materials[i];
    if (!specialMtxOut && !std::any_of(mat.chunks.begin(), mat.chunks.end(), [](auto& chunk) {
          if (const auto* const pass = chunk.template get_if<MaterialSet::Material::PASS>()) {
            return pass->uvAnimType != MaterialSet::Material::BlendMaterial::UVAnimType::Invalid;
          }
          return false;
        })) {
      continue;
    }
    auto& buf = uniforms[i];
    auto* out = buf.map();
    if (specialMtxOut) {
      out->at(7) = *specialMtxOut;
    }
    size_t passIdx = 0;
    for (const auto& chunk : mat.chunks) {
      if (const auto* const pass = chunk.get_if<MaterialSet::Material::PASS>()) {
        ProcessAnimation(out->at(passIdx++), *pass);
      }
    }
    buf.unmap();
  }
}

void GeometryUniformLayout::Update(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose,
                                   const MaterialSet* matSet,
                                   std::vector<hsh::dynamic_owner<hsh::uniform_buffer_typeless>>& skinBankUniforms,
                                   const CBooModel* parent) const {
#if HSH_PROFILE_MODE
  return;
#endif

  if (m_skinBankCount) {
    /* Skinned */
    std::vector<const zeus::CTransform*> bankTransforms;
    size_t weightCount = m_weightVecCount * 4;
    bankTransforms.reserve(weightCount);
    auto WriteUniform = [&]<uint32_t NSkinSlots>(int skinBankIdx) constexpr {
      auto& buf = skinBankUniforms[skinBankIdx];
      buf.GetTypeInfo().Assert<hsh::uniform_buffer<CModelShaders::VertUniform<NSkinSlots>>>();
      auto* out = reinterpret_cast<CModelShaders::VertUniform<NSkinSlots>*>(buf.map());
      if (cskr && pose) {
        cskr->GetBankTransforms(bankTransforms, *pose, skinBankIdx);
        for (size_t slot = 0; slot < NSkinSlots; ++slot) {
          if (slot >= bankTransforms.size()) {
            out->objs[slot] = zeus::CMatrix4f();
            out->objsInv[slot] = zeus::CMatrix4f();
          } else {
            out->objs[slot] = bankTransforms[slot]->toMatrix4f();
            out->objsInv[slot] = bankTransforms[slot]->toMatrix4f();
          }
        }
        bankTransforms.clear();
      } else {
        for (size_t slot = 0; slot < NSkinSlots; ++slot) {
          out->objs[slot] = zeus::CMatrix4f();
          out->objsInv[slot] = zeus::CMatrix4f();
        }
      }
      out->mv = CGraphics::g_GXModelView.toMatrix4f();
      out->mvInv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
      out->proj = CGraphics::GetPerspectiveProjectionMatrix(true);
      buf.unmap();
    };
    for (size_t i = 0; i < m_skinBankCount; ++i) {
      switch (weightCount) {
      case 1:
        WriteUniform.operator()<1>(i);
        break;
      case 4:
        WriteUniform.operator()<4>(i);
        break;
      case 8:
        WriteUniform.operator()<8>(i);
        break;
      case 12:
        WriteUniform.operator()<12>(i);
        break;
      case 16:
        WriteUniform.operator()<16>(i);
        break;
      case 20:
        WriteUniform.operator()<20>(i);
        break;
      default:
        assert(false && "Unhandled weightCount");
      }
    }
  } else {
    /* Non-Skinned */
    auto& buf = skinBankUniforms[0];
    buf.GetTypeInfo().Assert<hsh::uniform_buffer<CModelShaders::VertUniform<0>>>();
    auto* out = reinterpret_cast<CModelShaders::VertUniform<0>*>(buf.map());
    out->mv = CGraphics::g_GXModelView.toMatrix4f();
    out->mvInv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    out->proj = CGraphics::GetPerspectiveProjectionMatrix(true);
    buf.unmap();
  }
}

hsh::dynamic_owner<hsh::uniform_buffer_typeless> GeometryUniformLayout::AllocateVertUniformBuffer() const {
  switch (m_weightVecCount * 4) {
#define VERT_UNIFORM(SkinSlotCount)                                                                                    \
  case SkinSlotCount:                                                                                                  \
    return hsh::create_dynamic_uniform_buffer<CModelShaders::VertUniform<SkinSlotCount>>();
    VERT_UNIFORM(0)
    VERT_UNIFORM(4)
    VERT_UNIFORM(8)
    VERT_UNIFORM(12)
    VERT_UNIFORM(16)
    VERT_UNIFORM(20)
#undef VERT_UNIFORM
  default:
    assert(false && "Unhandled vertex uniform size");
    return {};
  }
}

hsh::dynamic_owner<hsh::vertex_buffer_typeless>* CBooModel::UpdateUniformData(const CModelFlags& flags,
                                                                              const CSkinRules* cskr,
                                                                              const CPoseAsTransforms* pose,
                                                                              int sharedLayoutBuf) {
  if (!g_DummyTextures && !TryLockTextures()) {
    return nullptr;
  }

  ModelInstance* inst = nullptr;
  if (sharedLayoutBuf >= 0) {
    if (m_instances.size() <= sharedLayoutBuf) {
      do {
        inst = PushNewModelInstance(m_instances.size(), ctx);
        if (inst == nullptr) {
          return nullptr;
        }
      } while (m_instances.size() <= sharedLayoutBuf);
    } else {
      inst = &m_instances[sharedLayoutBuf];
    }
    m_uniUpdateCount = sharedLayoutBuf + 1;
  } else {
    if (m_instances.size() <= m_uniUpdateCount) {
      inst = PushNewModelInstance(sharedLayoutBuf, ctx);
      if (inst == nullptr) {
        return nullptr;
      }
    } else {
      inst = &m_instances[m_uniUpdateCount];
    }
    ++m_uniUpdateCount;
  }

#if !HSH_PROFILE_MODE
  m_geomLayout->Update(flags, cskr, pose, x4_matSet, inst->m_geometryUniforms, this);
  CBooModel::UVAnimationBuffer::Update(inst->m_tcgUniforms, x4_matSet, flags, this);

  CModelShaders::FragmentUniform lightingOut;
  lightingOut = m_lightingData;
  if (flags.m_postType == EPostType::ThermalHot) {
    lightingOut.ambient = flags.m_addColor;
  } else if (!m_lightsActive) {
    lightingOut.ambient = gx_AmbientColors[0];
  }
  lightingOut.lightmapMul = CGraphics::g_ColorRegs[1];
  lightingOut.flagsColor = flags.x4_color;
  lightingOut.fog = CGraphics::g_Fog;
  inst->m_fragmentUniform.load(lightingOut);
#endif

  //  u8* dataOut = reinterpret_cast<u8*>(inst->m_uniformBuffer.map());
  //  u8* dataCur = dataOut;
  //
  //  if (flags.m_extendedShader == EExtendedShader::Thermal) /* Thermal Model (same as UV Mode 0) */
  //  {
  //    CModelShaders::ThermalUniform& thermalOut = *reinterpret_cast<CModelShaders::ThermalUniform*>(dataCur);
  //    thermalOut.mulColor = flags.x4_color;
  //    thermalOut.addColor = flags.addColor;
  //  } else if (flags.m_extendedShader >= EExtendedShader::SolidColor &&
  //             flags.m_extendedShader <= EExtendedShader::SolidColorBackfaceCullGreaterAlphaOnly) /* Solid color
  //             render */
  //  {
  //    CModelShaders::SolidUniform& solidOut = *reinterpret_cast<CModelShaders::SolidUniform*>(dataCur);
  //    solidOut.solidColor = flags.x4_color;
  //  } else if (flags.m_extendedShader == EExtendedShader::MorphBallShadow) /* MorphBall shadow render */
  //  {
  //    CModelShaders::MBShadowUniform& shadowOut = *reinterpret_cast<CModelShaders::MBShadowUniform*>(dataCur);
  //    shadowOut.shadowUp = CGraphics::g_GXModelView.rotate(zeus::skUp);
  //    shadowOut.shadowUp.w() = flags.x4_color.a();
  //    shadowOut.shadowId = flags.x4_color.r();
  //  } else if (flags.m_extendedShader == EExtendedShader::Disintegrate) {
  //    CModelShaders::OneTextureUniform& oneTexOut = *reinterpret_cast<CModelShaders::OneTextureUniform*>(dataCur);
  //    oneTexOut.addColor = flags.addColor;
  //    oneTexOut.fog = CGraphics::g_Fog;
  //  } else {
  //    CModelShaders::LightingUniform& lightingOut = *reinterpret_cast<CModelShaders::LightingUniform*>(dataCur);
  //    lightingOut = m_lightingData;
  //    lightingOut.colorRegs = CGraphics::g_ColorRegs;
  //    lightingOut.mulColor = flags.x4_color;
  //    lightingOut.addColor = flags.addColor;
  //    lightingOut.fog = CGraphics::g_Fog;
  //  }
  //
  //  dataCur += sizeof(CModelShaders::LightingUniform);
  //  dataCur = dataOut + ROUND_UP_256(dataCur - dataOut);

  /* Reflection texmtx uniform */
  for (size_t i = 0; i < x0_surfaces->size(); ++i) {
    const auto& surf = x0_surfaces->at(i);
    const MaterialSet::Material& mat = x4_matSet->materials.at(surf.m_data.matIdx);
    if (mat.flags.samusReflection() || mat.flags.samusReflectionSurfaceEye()) {
      EnsureViewDepStateCached(*this, mat.flags.samusReflectionSurfaceEye() ? &surf : nullptr,
                               inst->m_reflectUniforms[i]);
    }
  }

  inst->GetBooVBO(*this);
  return &inst->m_dynamicVbo;
}

void CBooModel::DrawAlpha(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose) {
  CModelFlags rFlags = flags;
  /* Check if we're overriding with RenderModelBlack */
  if (g_RenderModelBlack) {
    rFlags.m_postType = EPostType::Solid;
    rFlags.x4_color = zeus::skBlack;
  }

  if (TryLockTextures()) {
    UpdateUniformData(rFlags, cskr, pose);
    DrawAlphaSurfaces(rFlags);
  }
}

void CBooModel::DrawNormal(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose) {
  CModelFlags rFlags = flags;
  /* Check if we're overriding with RenderModelBlack */
  if (g_RenderModelBlack) {
    rFlags.m_postType = EPostType::Solid;
    rFlags.x4_color = zeus::skBlack;
  }
  if (TryLockTextures()) {
    UpdateUniformData(rFlags, cskr, pose);
    DrawNormalSurfaces(rFlags);
  }
}

void CBooModel::Draw(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose) {
  CModelFlags rFlags = flags;
  /* Check if we're overriding with RenderModelBlack */
  if (g_RenderModelBlack) {
    rFlags.m_postType = EPostType::Solid;
    rFlags.x4_color = zeus::skBlack;
  }

  if (TryLockTextures()) {
    UpdateUniformData(rFlags, cskr, pose);
    DrawSurfaces(rFlags);
  }
}

static const u8* MemoryFromPartData(const u8*& dataCur, const u32*& secSizeCur) {
  const u8* ret;
  if (*secSizeCur != 0)
    ret = dataCur;
  else
    ret = nullptr;

  dataCur += hecl::SBig(*secSizeCur);
  ++secSizeCur;
  return ret;
}

std::unique_ptr<CBooModel> CModel::MakeNewInstance(int shaderIdx, int subInsts, bool lockParent) {
  if (shaderIdx >= x18_matSets.size())
    shaderIdx = 0;
  auto ret = std::make_unique<CBooModel>(m_selfToken, this, &x8_surfaces, x18_matSets[shaderIdx], m_staticVbo.get(),
                                         m_ibo.get(), m_aabb, (m_flags & 0x2) != 0, subInsts, VertexFormat{m_hmdlMeta});
  if (lockParent)
    ret->LockParent();
  return ret;
}

CModel::CModel(std::unique_ptr<u8[]>&& in, u32 /* dataLen */, IObjectStore* store, CObjectReference* selfRef)
: m_selfToken(selfRef) {
  x38_lastFrame = CGraphics::GetFrameCounter() - 2;
  std::unique_ptr<u8[]> data = std::move(in);

  u32 version = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x4));
  m_flags = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x8));
  if (version != 0x10002)
    Log.report(logvisor::Fatal, FMT_STRING("invalid CMDL for loading with boo"));

  u32 secCount = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x24));
  u32 matSetCount = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x28));
  x18_matSets.reserve(matSetCount);
  const u8* dataCur = data.get() + ROUND_UP_32(0x2c + secCount * 4);
  const u32* secSizeCur = reinterpret_cast<const u32*>(data.get() + 0x2c);
  for (u32 i = 0; i < matSetCount; ++i) {
    const u32 matSetSz = hecl::SBig(*secSizeCur);
    const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
    SShader& shader = x18_matSets.emplace_back(i);
    athena::io::MemoryReader r(sec, matSetSz);
    shader.m_matSet.read(r);
    CBooModel::MakeTexturesFromMats(shader.m_matSet, shader.x0_textures, *store);
  }

  {
    u32 hmdlSz = hecl::SBig(*secSizeCur);
    const u8* hmdlMetadata = MemoryFromPartData(dataCur, secSizeCur);
    athena::io::MemoryReader r(hmdlMetadata, hmdlSz);
    m_hmdlMeta.read(r);
  }

  const u8* vboData = MemoryFromPartData(dataCur, secSizeCur);
  const u8* iboData = MemoryFromPartData(dataCur, secSizeCur);
  const u8* surfInfo = MemoryFromPartData(dataCur, secSizeCur);

  for (SShader& matSet : x18_matSets) {
    matSet.InitializeLayout(this);
  }

  /* Index buffer is always static */
  if (m_hmdlMeta.indexCount)
    m_ibo =
        hsh::create_index_buffer(hsh::detail::ArrayProxy{reinterpret_cast<const u32*>(iboData), m_hmdlMeta.indexCount});

  if (!m_hmdlMeta.bankCount) {
    /* Non-skinned models use static vertex buffers shared with CBooModel instances */
    MapVertData(m_hmdlMeta, [this, vboData]<typename VertData>() {
      const auto* data = reinterpret_cast<const VertData*>(vboData);
      m_staticVbo = hsh::create_vertex_buffer(hsh::detail::ArrayProxy{data, m_hmdlMeta.vertCount});
    });
  } else {
    /* Skinned models use per-instance dynamic buffers for vertex manipulation effects */
    size_t vboSz = m_hmdlMeta.vertStride * m_hmdlMeta.vertCount;
    if (vboSz) {
      m_dynamicVertexData.reset(new uint8_t[vboSz]);
      memmove(m_dynamicVertexData.get(), vboData, vboSz);
    }
  }

  const u32 surfCount = hecl::SBig(*reinterpret_cast<const u32*>(surfInfo));
  x8_surfaces.reserve(surfCount);
  for (u32 i = 0; i < surfCount; ++i) {
    const u32 surfSz = hecl::SBig(*secSizeCur);
    const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
    CBooSurface& surf = x8_surfaces.emplace_back();
    surf.selfIdx = i;
    athena::io::MemoryReader r(sec, surfSz);
    surf.m_data.read(r);
  }

  const float* aabbPtr = reinterpret_cast<const float*>(data.get() + 0xc);
  m_aabb = zeus::CAABox(hecl::SBig(aabbPtr[0]), hecl::SBig(aabbPtr[1]), hecl::SBig(aabbPtr[2]), hecl::SBig(aabbPtr[3]),
                        hecl::SBig(aabbPtr[4]), hecl::SBig(aabbPtr[5]));
  x28_modelInst = MakeNewInstance(0, 1, false);
}

void SShader::UnlockTextures() {
  for (auto& tex : x0_textures)
    tex.second.Unlock();
}

void CBooModel::VerifyCurrentShader(int shaderIdx) {
  if (shaderIdx != m_matSetIdx && m_model)
    RemapMaterialData(m_model->x18_matSets[shaderIdx]);
}

void CBooModel::Touch(int shaderIdx) {
  VerifyCurrentShader(shaderIdx);
  TryLockTextures();
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
  x28_modelInst->VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->DrawAlpha(flags, nullptr, nullptr);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
  x28_modelInst->VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->DrawNormal(flags, nullptr, nullptr);
}

void CModel::Draw(const CModelFlags& flags) const {
  x28_modelInst->VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->Draw(flags, nullptr, nullptr);
}

bool CModel::IsLoaded(int shaderIdx) const {
  x28_modelInst->VerifyCurrentShader(shaderIdx);
  return x28_modelInst->TryLockTextures();
}

zeus::CVector3f CModel::GetPoolVertex(size_t idx) const {
  const auto* const offset = m_dynamicVertexData.get() + idx * m_hmdlMeta.vertStride;
  return reinterpret_cast<const CModelShaders::VertData<0, 0, 0>*>(offset)->posIn;
}

zeus::CVector3f CModel::GetPoolNormal(size_t idx) const {
  const auto* const offset = m_dynamicVertexData.get() + idx * m_hmdlMeta.vertStride;
  return reinterpret_cast<const CModelShaders::VertData<0, 0, 0>*>(offset)->normIn;
}

void CModel::ApplyVerticesCPU(hsh::dynamic_owner<hsh::vertex_buffer_typeless>& vertBuf,
                              const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) const {
#if HSH_PROFILE_MODE
  return;
#endif
  u8* data = reinterpret_cast<u8*>(vertBuf.map());
  for (u32 i = 0; i < std::min(u32(vn.size()), m_hmdlMeta.vertCount); ++i) {
    auto* const offset = data + i * m_hmdlMeta.vertStride;
    auto* const vert = reinterpret_cast<CModelShaders::VertData<0, 0, 0>*>(offset);
    vert->posIn = vn[i].first;
    vert->normIn = vn[i].second;
  }
  vertBuf.unmap();
}

void CModel::RestoreVerticesCPU(hsh::dynamic_owner<hsh::vertex_buffer_typeless>& vertBuf) const {
#if HSH_PROFILE_MODE
  return;
#endif
  size_t size = m_hmdlMeta.vertStride * m_hmdlMeta.vertCount;
  memcpy(vertBuf.map(), m_dynamicVertexData.get(), size);
  vertBuf.unmap();
}

void CModel::_WarmupShaders() {
  CBooModel::SetDummyTextures(true);
  CBooModel::EnableShadowMaps(g_Renderer->x220_sphereRamp.get(), zeus::CTransform());
  CGraphics::CProjectionState backupProj = CGraphics::GetProjectionState();
  zeus::CTransform backupViewPoint = CGraphics::g_ViewMatrix;
  zeus::CTransform backupModel = CGraphics::g_GXModelMatrix;
  CGraphics::SetModelMatrix(zeus::CTransform::Translate(-m_aabb.center()));
  CGraphics::SetViewPointMatrix(zeus::CTransform::Translate(0.f, -2048.f, 0.f));
  CGraphics::SetOrtho(-2048.f, 2048.f, 2048.f, -2048.f, 0.f, 4096.f);
  CModelFlags defaultFlags;
  for (SShader& shader : x18_matSets) {
    GetInstance().RemapMaterialData(shader);
    GetInstance().UpdateUniformData(defaultFlags, nullptr, nullptr);
    GetInstance().WarmupDrawSurfaces(defaultFlags, defaultFlags);
  }
  CGraphics::SetProjectionState(backupProj);
  CGraphics::SetViewPointMatrix(backupViewPoint);
  CGraphics::SetModelMatrix(backupModel);
  CBooModel::DisableShadowMaps();
  CBooModel::SetDummyTextures(false);
}

void CModel::WarmupShaders(const SObjectTag& cmdlTag) {
  TToken<CModel> model = g_SimplePool->GetObj(cmdlTag);
  CModel* modelObj = model.GetObj();
  modelObj->_WarmupShaders();
}

CFactoryFnReturn FModelFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  CFactoryFnReturn ret = TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, sp, selfRef));
  return ret;
}

} // namespace metaforce
