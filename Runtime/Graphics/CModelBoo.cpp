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

#include <boo/graphicsdev/Metal.hpp>
#include <hecl/CVarManager.hpp>
#include <hecl/HMDLMeta.hpp>
#include <hecl/Runtime.hpp>
#include <logvisor/logvisor.hpp>

namespace urde {
namespace {
logvisor::Module Log("urde::CBooModel");
CBooModel* g_FirstModel = nullptr;

constexpr zeus::CMatrix4f ReflectBaseMtx{
    0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f,
};

constexpr zeus::CMatrix4f ReflectPostGL{
    1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f,
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

bool CBooModel::g_DrawingOccluders = false;

void CBooModel::Shutdown() {
  g_shadowMap.reset();
  g_disintegrateTexture.reset();
  g_reflectionCube.reset();
  assert(g_FirstModel == nullptr && "Dangling CBooModels detected");
}

void CBooModel::ClearModelUniformCounters() {
  for (CBooModel* model = g_FirstModel; model; model = model->m_next)
    model->ClearUniformCounter();
}

zeus::CVector3f CBooModel::g_PlayerPosition = {};
float CBooModel::g_ModSeconds = 0.f;
float CBooModel::g_TransformedTime = 0.f;
float CBooModel::g_TransformedTime2 = 0.f;
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

CBooModel* CBooModel::g_LastModelCached = nullptr;
void CBooModel::KillCachedViewDepState() { g_LastModelCached = nullptr; }

bool CBooModel::g_DummyTextures = false;
bool CBooModel::g_RenderModelBlack = false;

zeus::CVector3f CBooModel::g_ReflectViewPos = {};

void CBooModel::EnsureViewDepStateCached(const CBooModel& model, const CBooSurface* surf, zeus::CMatrix4f* mtxsOut,
                                         float& alphaOut) {
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
  if (distance >= 5.f) {
    alphaOut = 0.f;
  } else {
    alphaOut = (5.f - distance) / 5.f;

    /* Indirect map matrix */
    mtxsOut[0] = (CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix).toMatrix4f();

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
    mtxsOut[1] = ReflectBaseMtx;
    mtxsOut[1][0][0] = f1 * v2.x();
    mtxsOut[1][1][0] = f1 * v2.y();
    mtxsOut[1][3][0] = -surfPos.dot(v2) * f1 + 0.5f;
    mtxsOut[1][2][1] = f2;
    mtxsOut[1][3][1] = -modelToPlayerLocal.z() * f2;
    switch (CGraphics::g_BooPlatform) {
    case boo::IGraphicsDataFactory::Platform::OpenGL:
      mtxsOut[1] = ReflectPostGL * mtxsOut[1];
      break;
    default:
      break;
    }
  }
}

boo::ObjToken<boo::ITexture> CBooModel::g_shadowMap;
zeus::CTransform CBooModel::g_shadowTexXf;
boo::ObjToken<boo::ITexture> CBooModel::g_disintegrateTexture;
boo::ObjToken<boo::ITextureCubeR> CBooModel::g_reflectionCube;

void CBooModel::EnableShadowMaps(const boo::ObjToken<boo::ITexture>& map, const zeus::CTransform& texXf) {
  g_shadowMap = map;
  g_shadowTexXf = texXf;
}
void CBooModel::DisableShadowMaps() { g_shadowMap = nullptr; }

CBooModel::~CBooModel() {
  if (m_prev)
    m_prev->m_next = m_next;
  if (m_next)
    m_next->m_prev = m_prev;
  if (this == g_FirstModel)
    g_FirstModel = m_next;
}

CBooModel::CBooModel(TToken<CModel>& token, CModel* parent, std::vector<CBooSurface>* surfaces, SShader& shader,
                     const boo::ObjToken<boo::IGraphicsBufferS>& vbo, const boo::ObjToken<boo::IGraphicsBufferS>& ibo,
                     const zeus::CAABox& aabb, u8 renderMask, int numInsts)
: m_modelTok(token)
, m_model(parent)
, x0_surfaces(surfaces)
, x4_matSet(&shader.m_matSet)
, m_geomLayout(&*shader.m_geomLayout)
, m_matSetIdx(shader.m_matSetIdx)
, m_pipelines(&shader.m_shaders)
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

boo::ObjToken<boo::IGraphicsBuffer> CBooModel::ModelInstance::GetBooVBO(const CBooModel& model,
                                                                        boo::IGraphicsDataFactory::Context& ctx) {
  if (model.m_staticVbo)
    return model.m_staticVbo.get();
  if (!m_dynamicVbo && model.m_model) {
    const CModel& parent = *model.m_model;
    m_dynamicVbo =
        ctx.newDynamicBuffer(boo::BufferUse::Vertex, parent.m_hmdlMeta.vertStride, parent.m_hmdlMeta.vertCount);
    m_dynamicVbo->load(parent.m_dynamicVertexData.get(), parent.m_hmdlMeta.vertStride * parent.m_hmdlMeta.vertCount);
  }
  return m_dynamicVbo.get();
}

GeometryUniformLayout::GeometryUniformLayout(const CModel* model, const MaterialSet* matSet) {
  if (model) {
    m_skinBankCount = model->m_hmdlMeta.bankCount;
    m_weightVecCount = model->m_hmdlMeta.weightCount;
  }

  m_skinOffs.reserve(std::max(size_t(1), m_skinBankCount));
  m_skinSizes.reserve(std::max(size_t(1), m_skinBankCount));

  m_uvOffs.reserve(matSet->materials.size());
  m_uvSizes.reserve(matSet->materials.size());

  if (m_skinBankCount) {
    /* Skinned */
    for (size_t i = 0; i < m_skinBankCount; ++i) {
      size_t thisSz = ROUND_UP_256(sizeof(zeus::CMatrix4f) * (2 * m_weightVecCount * 4 + 3));
      m_skinOffs.push_back(m_geomBufferSize);
      m_skinSizes.push_back(thisSz);
      m_geomBufferSize += thisSz;
    }
  } else {
    /* Non-Skinned */
    size_t thisSz = ROUND_UP_256(sizeof(zeus::CMatrix4f) * 3);
    m_skinOffs.push_back(m_geomBufferSize);
    m_skinSizes.push_back(thisSz);
    m_geomBufferSize += thisSz;
  }

  /* Animated UV transform matrices */
  for (const MaterialSet::Material& mat : matSet->materials) {
    (void)mat;
    size_t thisSz = ROUND_UP_256(/*mat.uvAnims.size()*/ 8 * (sizeof(zeus::CMatrix4f) * 2));
    m_uvOffs.push_back(m_geomBufferSize);
    m_uvSizes.push_back(thisSz);
    m_geomBufferSize += thisSz;
  }
}

CBooModel::ModelInstance* CBooModel::PushNewModelInstance(int sharedLayoutBuf) {
  if (!x40_24_texturesLoaded && !g_DummyTextures) {
    return nullptr;
  }

  if (m_instances.size() >= 512) {
    Log.report(logvisor::Fatal, FMT_STRING("Model buffer overflow"));
  }

  ModelInstance& newInst = m_instances.emplace_back();

  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    /* Build geometry uniform buffer if shared not available */
    boo::ObjToken<boo::IGraphicsBufferD> geomUniformBuf;
    if (sharedLayoutBuf >= 0) {
      geomUniformBuf = m_geomLayout->GetSharedBuffer(sharedLayoutBuf);
    } else {
      geomUniformBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, m_geomLayout->m_geomBufferSize, 1);
      newInst.m_geomUniformBuffer = geomUniformBuf;
    }

    /* Lighting and reflection uniforms */
    size_t uniBufSize = 0;

    /* Lighting uniform */
    size_t lightOff = 0;
    size_t lightSz = 0;
    {
      size_t thisSz = ROUND_UP_256(sizeof(CModelShaders::LightingUniform));
      lightOff = uniBufSize;
      lightSz = thisSz;
      uniBufSize += thisSz;
    }

    /* Surface reflection texmatrix uniform with first identity slot */
    size_t reflectOff = uniBufSize;
    uniBufSize += 256;
    for (const CBooSurface& surf : *x0_surfaces) {
      const MaterialSet::Material& mat = x4_matSet->materials.at(surf.m_data.matIdx);
      if (mat.flags.samusReflection() || mat.flags.samusReflectionSurfaceEye())
        uniBufSize += 256;
    }

    /* Allocate resident buffer */
    m_uniformDataSize = uniBufSize;
    newInst.m_uniformBuffer = ctx.newDynamicBuffer(boo::BufferUse::Uniform, uniBufSize, 1);

    const std::array<boo::ObjToken<boo::IGraphicsBuffer>, 4> bufs{
        geomUniformBuf.get(),
        geomUniformBuf.get(),
        newInst.m_uniformBuffer.get(),
        newInst.m_uniformBuffer.get(),
    };

    /* Binding for each surface */
    newInst.m_shaderDataBindings.reserve(x0_surfaces->size());

    std::array<size_t, 4> thisOffs;
    std::array<size_t, 4> thisSizes;

    static constexpr std::array stages{
        boo::PipelineStage::Vertex,
        boo::PipelineStage::Vertex,
        boo::PipelineStage::Fragment,
        boo::PipelineStage::Vertex,
    };

    /* Enumerate surfaces and build data bindings */
    size_t curReflect = reflectOff + 256;
    for (const CBooSurface& surf : *x0_surfaces) {
      const MaterialSet::Material& mat = x4_matSet->materials.at(surf.m_data.matIdx);

      std::array<boo::ObjToken<boo::ITexture>, 12> texs{
          g_Renderer->m_clearTexture.get(),  g_Renderer->m_clearTexture.get(),  g_Renderer->m_clearTexture.get(),
          g_Renderer->m_clearTexture.get(),  g_Renderer->m_clearTexture.get(),  g_Renderer->m_clearTexture.get(),
          g_Renderer->m_whiteTexture.get(),  g_Renderer->m_clearTexture.get(),  g_Renderer->x220_sphereRamp.get(),
          g_Renderer->x220_sphereRamp.get(), g_Renderer->x220_sphereRamp.get(), g_Renderer->x220_sphereRamp.get(),
      };
      if (!g_DummyTextures) {
        for (const auto& ch : mat.chunks) {
          if (auto pass = ch.get_if<MaterialSet::Material::PASS>()) {
            auto search = x1c_textures.find(pass->texId.toUint32());
            boo::ObjToken<boo::ITexture> btex;
            if (search != x1c_textures.cend() && (btex = search->second.GetObj()->GetBooTexture()))
              texs[MaterialSet::Material::TexMapIdx(pass->type)] = btex;
          } else if (auto pass = ch.get_if<MaterialSet::Material::CLR>()) {
            boo::ObjToken<boo::ITexture> btex = g_Renderer->GetColorTexture(zeus::CColor(pass->color));
            texs[MaterialSet::Material::TexMapIdx(pass->type)] = btex;
          }
        }
      }

      if (m_geomLayout->m_skinBankCount) {
        thisOffs[0] = m_geomLayout->m_skinOffs[surf.m_data.skinMtxBankIdx];
        thisSizes[0] = m_geomLayout->m_skinSizes[surf.m_data.skinMtxBankIdx];
      } else {
        thisOffs[0] = 0;
        thisSizes[0] = 256;
      }

      thisOffs[1] = m_geomLayout->m_uvOffs[surf.m_data.matIdx];
      thisSizes[1] = m_geomLayout->m_uvSizes[surf.m_data.matIdx];

      thisOffs[2] = lightOff;
      thisSizes[2] = lightSz;

      bool useReflection = mat.flags.samusReflection() || mat.flags.samusReflectionSurfaceEye();
      if (useReflection) {
        if (g_Renderer->x14c_reflectionTex)
          texs[11] = g_Renderer->x14c_reflectionTex.get();
        thisOffs[3] = curReflect;
        curReflect += 256;
      } else {
        thisOffs[3] = reflectOff;
      }
      thisSizes[3] = 256;

      const CModelShaders::ShaderPipelines& pipelines = m_pipelines->at(surf.m_data.matIdx);

      std::vector<boo::ObjToken<boo::IShaderDataBinding>>& extendeds = newInst.m_shaderDataBindings.emplace_back();
      extendeds.reserve(pipelines->size());

      EExtendedShader idx{};
      for (const auto& pipeline : *pipelines) {
        if (idx == EExtendedShader::Thermal) {
          texs[8] = g_Renderer->x220_sphereRamp.get();
        } else if (idx == EExtendedShader::MorphBallShadow) {
          texs[8] = g_Renderer->m_ballShadowId.get();
          texs[9] = g_Renderer->x220_sphereRamp.get();
          texs[10] = g_Renderer->m_ballFade.get();
        } else if (idx == EExtendedShader::WorldShadow || idx == EExtendedShader::LightingCubeReflectionWorldShadow) {
          if (g_shadowMap)
            texs[8] = g_shadowMap;
          else
            texs[8] = g_Renderer->x220_sphereRamp.get();
        } else if (idx == EExtendedShader::Disintegrate) {
          if (g_disintegrateTexture)
            texs[8] = g_disintegrateTexture;
          else
            texs[8] = g_Renderer->x220_sphereRamp.get();
        } else if (hecl::com_cubemaps->toBoolean() && (idx == EExtendedShader::LightingCubeReflection ||
                   idx == EExtendedShader::LightingCubeReflectionWorldShadow)) {
          if (m_lastDrawnReflectionCube)
            texs[11] = m_lastDrawnReflectionCube.get();
          else
            texs[11] = g_Renderer->x220_sphereRamp.get();
        }
        extendeds.push_back(ctx.newShaderDataBinding(
            pipeline, newInst.GetBooVBO(*this, ctx), nullptr, m_staticIbo.get(), bufs.size(), bufs.data(),
            stages.data(), thisOffs.data(), thisSizes.data(), texs.size(), texs.data(), nullptr, nullptr));
        idx = EExtendedShader(size_t(idx) + 1);
      }
    }
    return true;
  } BooTrace);

  return &newInst;
}

void CBooModel::MakeTexturesFromMats(const MaterialSet& matSet,
                                     std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut,
                                     IObjectStore& store) {
  for (const auto& mat : matSet.materials) {
    for (const auto& chunk : mat.chunks) {
      if (auto pass = chunk.get_if<MaterialSet::Material::PASS>()) {
        toksOut.emplace(std::make_pair(pass->texId.toUint32(), store.GetObj({SBIG('TXTR'), pass->texId.toUint32()})));
      }
    }
  }
}

void CBooModel::MakeTexturesFromMats(std::unordered_map<CAssetId, TCachedToken<CTexture>>& toksOut,
                                     IObjectStore& store) {
  MakeTexturesFromMats(*x4_matSet, toksOut, store);
}

void CBooModel::ActivateLights(const std::vector<CLight>& lights) { m_lightingData.ActivateLights(lights); }

void CBooModel::DisableAllLights() {
  m_lightingData.ambient = zeus::skBlack;

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
  m_pipelines = &shader.m_shaders;
  x40_24_texturesLoaded = false;
  m_instances.clear();
}

void CBooModel::RemapMaterialData(SShader& shader,
                                  const std::unordered_map<int, CModelShaders::ShaderPipelines>& pipelines) {
  if (!shader.m_geomLayout)
    return;
  x4_matSet = &shader.m_matSet;
  m_geomLayout = &*shader.m_geomLayout;
  m_matSetIdx = shader.m_matSetIdx;
  x1c_textures = shader.x0_textures;
  m_pipelines = &pipelines;
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

    if (allLoad) {
      for (auto& pipeline : *m_pipelines) {
        for (auto& subpipeline : *pipeline.second) {
          if (!subpipeline->isReady()) {
            allLoad = false;
            break;
          }
        }
        if (!allLoad) {
          break;
        }
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
  flags.m_extendedShader = extendedIdx;

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

static EExtendedShader ResolveExtendedShader(const MaterialSet::Material& data, const CModelFlags& flags) {
  bool noZWrite = flags.m_noZWrite || !data.flags.depthWrite();

  /* Ensure cubemap extension shaders fall back to non-cubemap equivalents if necessary */
  EExtendedShader intermediateExtended = flags.m_extendedShader;
  if (!hecl::com_cubemaps->toBoolean() || g_Renderer->IsThermalVisorHotPass() || g_Renderer->IsThermalVisorActive()) {
    if (intermediateExtended == EExtendedShader::LightingCubeReflection)
      intermediateExtended = EExtendedShader::Lighting;
    else if (intermediateExtended == EExtendedShader::LightingCubeReflectionWorldShadow)
      intermediateExtended = EExtendedShader::WorldShadow;
  }

  EExtendedShader extended = EExtendedShader::Flat;
  if (intermediateExtended == EExtendedShader::Lighting) {
    /* Transform lighting into thermal cold if the thermal visor is active */
    if (g_Renderer->IsThermalVisorHotPass())
      return EExtendedShader::LightingAlphaWrite;
    else if (g_Renderer->IsThermalVisorActive())
      return EExtendedShader::ThermalCold;
    if (data.blendMode == MaterialSet::Material::BlendMaterial::BlendMode::Opaque) {
      /* Override shader if originally opaque (typical for FRME models) */
      if (flags.x0_blendMode > 6) {
        if (flags.m_depthGreater)
          extended = EExtendedShader::ForcedAdditiveNoZWriteDepthGreater;
        else
          extended =
            flags.m_noCull
            ? (noZWrite ? EExtendedShader::ForcedAdditiveNoCullNoZWrite : EExtendedShader::ForcedAdditiveNoCull)
            : (noZWrite ? EExtendedShader::ForcedAdditiveNoZWrite : EExtendedShader::ForcedAdditive);
      } else if (flags.x0_blendMode > 4) {
        extended = flags.m_noCull
                   ? (noZWrite ? EExtendedShader::ForcedAlphaNoCullNoZWrite : EExtendedShader::ForcedAlphaNoCull)
                   : (noZWrite ? EExtendedShader::ForcedAlphaNoZWrite : EExtendedShader::ForcedAlpha);
      } else {
        extended = flags.m_noCull
                   ? (noZWrite ? EExtendedShader::ForcedAlphaNoCullNoZWrite : EExtendedShader::ForcedAlphaNoCull)
                   : (noZWrite ? EExtendedShader::ForcedAlphaNoZWrite : EExtendedShader::Lighting);
      }
    } else if (flags.m_noCull && noZWrite) {
      /* Substitute no-cull,no-zwrite pipeline if available */
      if (data.blendMode == MaterialSet::Material::BlendMaterial::BlendMode::Additive)
        extended = EExtendedShader::ForcedAdditiveNoCullNoZWrite;
      else
        extended = EExtendedShader::ForcedAlphaNoCullNoZWrite;
    } else if (flags.m_noCull) {
      /* Substitute no-cull pipeline if available */
      if (data.blendMode == MaterialSet::Material::BlendMaterial::BlendMode::Additive)
        extended = EExtendedShader::ForcedAdditiveNoCull;
      else
        extended = EExtendedShader::ForcedAlphaNoCull;
    } else if (noZWrite) {
      /* Substitute no-zwrite pipeline if available */
      if (data.blendMode == MaterialSet::Material::BlendMaterial::BlendMode::Additive)
        extended = EExtendedShader::ForcedAdditiveNoZWrite;
      else
        extended = EExtendedShader::ForcedAlphaNoZWrite;
    } else {
      extended = EExtendedShader::Lighting;
    }
  } else if (intermediateExtended < EExtendedShader::MAX) {
    extended = intermediateExtended;
  }

  return extended;
}

void CBooModel::DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const {
  // if (m_uniUpdateCount == 0)
  //    Log.report(logvisor::Fatal, FMT_STRING("UpdateUniformData() not called"));
  if (m_uniUpdateCount == 0 || m_uniUpdateCount > m_instances.size())
    return;
  const ModelInstance& inst = m_instances[m_uniUpdateCount - 1];

  const MaterialSet::Material& data = GetMaterialByIndex(surf.m_data.matIdx);
  if (data.flags.shadowOccluderMesh() && !g_DrawingOccluders)
    return;

  const std::vector<boo::ObjToken<boo::IShaderDataBinding>>& extendeds = inst.m_shaderDataBindings[surf.selfIdx];
  EExtendedShader extended = ResolveExtendedShader(data, flags);

  boo::ObjToken<boo::IShaderDataBinding> binding = extendeds[size_t(extended)];
  CGraphics::SetShaderDataBinding(binding);
  CGraphics::DrawArrayIndexed(surf.m_data.idxStart, surf.m_data.idxCount);
}

void CBooModel::WarmupDrawSurfaces() const {
  const CBooSurface* surf = x38_firstUnsortedSurface;
  while (surf) {
    WarmupDrawSurface(*surf);
    surf = surf->m_next;
  }

  surf = x3c_firstSortedSurface;
  while (surf) {
    WarmupDrawSurface(*surf);
    surf = surf->m_next;
  }
}

void CBooModel::WarmupDrawSurface(const CBooSurface& surf) const {
  if (m_uniUpdateCount > m_instances.size())
    return;
  const ModelInstance& inst = m_instances[m_uniUpdateCount - 1];

  for (auto& binding : inst.m_shaderDataBindings[surf.selfIdx]) {
    CGraphics::SetShaderDataBinding(binding);
    CGraphics::DrawArrayIndexed(surf.m_data.idxStart, std::min(u32(3), surf.m_data.idxCount));
  }
}

void CBooModel::UVAnimationBuffer::ProcessAnimation(u8*& bufOut, const MaterialSet::Material::PASS& anim) {
  using UVAnimType = MaterialSet::Material::BlendMaterial::UVAnimType;
  if (anim.uvAnimType == UVAnimType::Invalid)
    return;
  zeus::CMatrix4f& texMtxOut = reinterpret_cast<zeus::CMatrix4f&>(*bufOut);
  zeus::CMatrix4f& postMtxOut = reinterpret_cast<zeus::CMatrix4f&>(*(bufOut + sizeof(zeus::CMatrix4f)));
  texMtxOut = zeus::CMatrix4f();
  postMtxOut = zeus::CMatrix4f();
  switch (anim.uvAnimType) {
  case UVAnimType::MvInvNoTranslation: {
    texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    texMtxOut[3].w() = 1.f;
    postMtxOut[0].x() = 0.5f;
    postMtxOut[1].y() = 0.5f;
    postMtxOut[3].x() = 0.5f;
    postMtxOut[3].y() = 0.5f;
    break;
  }
  case UVAnimType::MvInv: {
    texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    texMtxOut[3] = CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix.origin;
    texMtxOut[3].w() = 1.f;
    postMtxOut[0].x() = 0.5f;
    postMtxOut[1].y() = 0.5f;
    postMtxOut[3].x() = 0.5f;
    postMtxOut[3].y() = 0.5f;
    break;
  }
  case UVAnimType::Scroll: {
    texMtxOut[3].x() = CGraphics::GetSecondsMod900() * anim.uvAnimParms[2] + anim.uvAnimParms[0];
    texMtxOut[3].y() = CGraphics::GetSecondsMod900() * anim.uvAnimParms[3] + anim.uvAnimParms[1];
    break;
  }
  case UVAnimType::Rotation: {
    float angle = CGraphics::GetSecondsMod900() * anim.uvAnimParms[1] + anim.uvAnimParms[0];
    float acos = std::cos(angle);
    float asin = std::sin(angle);
    texMtxOut[0].x() = acos;
    texMtxOut[0].y() = asin;
    texMtxOut[1].x() = -asin;
    texMtxOut[1].y() = acos;
    texMtxOut[3].x() = (1.0f - (acos - asin)) * 0.5f;
    texMtxOut[3].y() = (1.0f - (asin + acos)) * 0.5f;
    break;
  }
  case UVAnimType::HStrip: {
    float value = anim.uvAnimParms[0] * anim.uvAnimParms[2] * (anim.uvAnimParms[3] + CGraphics::GetSecondsMod900());
    texMtxOut[3].x() = std::trunc(anim.uvAnimParms[1] * fmod(value, 1.0f)) * anim.uvAnimParms[2];
    break;
  }
  case UVAnimType::VStrip: {
    float value = anim.uvAnimParms[0] * anim.uvAnimParms[2] * (anim.uvAnimParms[3] + CGraphics::GetSecondsMod900());
    texMtxOut[3].y() = std::trunc(anim.uvAnimParms[1] * fmod(value, 1.0f)) * anim.uvAnimParms[2];
    break;
  }
  case UVAnimType::Model: {
    texMtxOut = CGraphics::g_GXModelMatrix.toMatrix4f();
    texMtxOut[3] = zeus::CVector4f(0.f, 0.f, 0.f, 1.f);
    postMtxOut[0].x() = 0.5f;
    postMtxOut[1].y() = 0.f;
    postMtxOut[2].y() = 0.5f;
    postMtxOut[3].x() = CGraphics::g_GXModelMatrix.origin.x() * 0.05f;
    postMtxOut[3].y() = CGraphics::g_GXModelMatrix.origin.y() * 0.05f;
    break;
  }
  case UVAnimType::CylinderEnvironment: {
    texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();

    const zeus::CVector3f& viewOrigin = CGraphics::g_ViewMatrix.origin;
    float xy = (viewOrigin.x() + viewOrigin.y()) * 0.025f * anim.uvAnimParms[1];
    xy = (xy - std::trunc(xy));
    float z = (viewOrigin.z()) * 0.05f * anim.uvAnimParms[1];
    z = (z - std::trunc(z));

    float halfA = anim.uvAnimParms[0] * 0.5f;

    postMtxOut =
        zeus::CTransform(zeus::CMatrix3f(halfA, 0.0, 0.0, 0.0, 0.0, halfA, 0.0, 0.0, 0.0), zeus::CVector3f(xy, z, 1.0))
            .toMatrix4f();
    break;
  }
  default:
    break;
  }
  bufOut += sizeof(zeus::CMatrix4f) * 2;
}

void CBooModel::UVAnimationBuffer::PadOutBuffer(u8*& bufStart, u8*& bufOut) {
  bufOut = bufStart + ROUND_UP_256(bufOut - bufStart);
}

void CBooModel::UVAnimationBuffer::Update(u8*& bufOut, const MaterialSet* matSet, const CModelFlags& flags,
                                          const CBooModel* parent) {
  u8* start = bufOut;

  if (flags.m_extendedShader == EExtendedShader::MorphBallShadow) {
    /* Special matrices for MorphBall shadow rendering */
    zeus::CMatrix4f texMtx = (zeus::CTransform::Scale(1.f / (flags.mbShadowBox.max - flags.mbShadowBox.min)) *
                              zeus::CTransform::Translate(-flags.mbShadowBox.min) * CGraphics::g_GXModelMatrix)
                                 .toMatrix4f();
    for (const MaterialSet::Material& mat : matSet->materials) {
      (void)mat;
      std::array<zeus::CMatrix4f, 2>* mtxs = reinterpret_cast<std::array<zeus::CMatrix4f, 2>*>(bufOut);
      mtxs[0][0] = texMtx;
      mtxs[0][1] = MBShadowPost0;
      mtxs[1][0] = texMtx;
      mtxs[1][1] = MBShadowPost1;
      bufOut += sizeof(zeus::CMatrix4f) * 2 * 8;
      PadOutBuffer(start, bufOut);
    }
    return;
  } else if (flags.m_extendedShader == EExtendedShader::Disintegrate) {
    assert(parent != nullptr && "Parent CBooModel not set");
    zeus::CTransform xf = zeus::CTransform::RotateX(-zeus::degToRad(45.f));
    zeus::CAABox aabb = parent->GetAABB().getTransformedAABox(xf);
    xf = zeus::CTransform::Scale(5.f / (aabb.max - aabb.min)) * zeus::CTransform::Translate(-aabb.min) * xf;
    zeus::CMatrix4f texMtx = xf.toMatrix4f();
    zeus::CMatrix4f post0 = DisintegratePost;
    post0[3].x() = flags.addColor.a();
    post0[3].y() = 6.f * -(1.f - flags.addColor.a()) + 1.f;
    zeus::CMatrix4f post1 = DisintegratePost;
    post1[3].x() = -0.85f * flags.addColor.a() - 0.15f;
    post1[3].y() = float(post0[3].y());
    /* Special matrices for disintegration rendering */
    for (const MaterialSet::Material& mat : matSet->materials) {
      (void)mat;
      std::array<zeus::CMatrix4f, 2>* mtxs = reinterpret_cast<std::array<zeus::CMatrix4f, 2>*>(bufOut);
      mtxs[0][0] = texMtx;
      mtxs[0][1] = post0;
      mtxs[1][0] = texMtx;
      mtxs[1][1] = post1;
      bufOut += sizeof(zeus::CMatrix4f) * 2 * 8;
      PadOutBuffer(start, bufOut);
    }
    return;
  }

  std::optional<std::array<zeus::CMatrix4f, 2>> specialMtxOut;
  if (flags.m_extendedShader == EExtendedShader::Thermal) {
    /* Special Mode0 matrix for exclusive Thermal Visor use */
    specialMtxOut.emplace();

    zeus::CMatrix4f& texMtxOut = (*specialMtxOut)[0];
    texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    texMtxOut[3].zeroOut();
    texMtxOut[3].w() = 1.f;

    zeus::CMatrix4f& postMtxOut = (*specialMtxOut)[1];
    postMtxOut[0].x() = 0.5f;
    postMtxOut[1].y() = 0.5f;
    postMtxOut[3].x() = 0.5f;
    postMtxOut[3].y() = 0.5f;
  } else if (flags.m_extendedShader == EExtendedShader::WorldShadow ||
             flags.m_extendedShader == EExtendedShader::LightingCubeReflectionWorldShadow) {
    /* Special matrix for mapping world shadow */
    specialMtxOut.emplace();

    zeus::CMatrix4f mat = g_shadowTexXf.toMatrix4f();
    zeus::CMatrix4f& texMtxOut = (*specialMtxOut)[0];
    texMtxOut[0][0] = float(mat[0][0]);
    texMtxOut[1][0] = float(mat[1][0]);
    texMtxOut[2][0] = float(mat[2][0]);
    texMtxOut[3][0] = float(mat[3][0]);
    texMtxOut[0][1] = float(mat[0][2]);
    texMtxOut[1][1] = float(mat[1][2]);
    texMtxOut[2][1] = float(mat[2][2]);
    texMtxOut[3][1] = float(mat[3][2]);
  }

  for (const MaterialSet::Material& mat : matSet->materials) {
    if (specialMtxOut) {
      std::array<zeus::CMatrix4f, 2>* mtxs = reinterpret_cast<std::array<zeus::CMatrix4f, 2>*>(bufOut);
      mtxs[7][0] = (*specialMtxOut)[0];
      mtxs[7][1] = (*specialMtxOut)[1];
    }
    u8* bufOrig = bufOut;
    for (const auto& chunk : mat.chunks) {
      if (auto pass = chunk.get_if<MaterialSet::Material::PASS>()) {
        ProcessAnimation(bufOut, *pass);
      }
    }
    bufOut = bufOrig + sizeof(zeus::CMatrix4f) * 2 * 8;
    PadOutBuffer(start, bufOut);
  }
}

void GeometryUniformLayout::Update(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose,
                                   const MaterialSet* matSet, const boo::ObjToken<boo::IGraphicsBufferD>& buf,
                                   const CBooModel* parent) const {
  u8* dataOut = reinterpret_cast<u8*>(buf->map(m_geomBufferSize));
  u8* dataCur = dataOut;

  if (m_skinBankCount) {
    /* Skinned */
    std::vector<const zeus::CTransform*> bankTransforms;
    size_t weightCount = m_weightVecCount * 4;
    bankTransforms.reserve(weightCount);
    for (size_t i = 0; i < m_skinBankCount; ++i) {
      if (cskr && pose) {
        cskr->GetBankTransforms(bankTransforms, *pose, i);

        for (size_t w = 0; w < weightCount; ++w) {
          zeus::CMatrix4f& obj = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
          if (w >= bankTransforms.size())
            obj = zeus::CMatrix4f();
          else
            obj = bankTransforms[w]->toMatrix4f();
          dataCur += sizeof(zeus::CMatrix4f);
        }
        for (size_t w = 0; w < weightCount; ++w) {
          zeus::CMatrix4f& objInv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
          if (w >= bankTransforms.size())
            objInv = zeus::CMatrix4f();
          else
            objInv = bankTransforms[w]->basis;
          dataCur += sizeof(zeus::CMatrix4f);
        }

        bankTransforms.clear();
      } else {
        for (size_t w = 0; w < weightCount; ++w) {
          zeus::CMatrix4f& mv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
          mv = zeus::CMatrix4f();
          dataCur += sizeof(zeus::CMatrix4f);
        }
        for (size_t w = 0; w < weightCount; ++w) {
          zeus::CMatrix4f& mvinv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
          mvinv = zeus::CMatrix4f();
          dataCur += sizeof(zeus::CMatrix4f);
        }
      }
      zeus::CMatrix4f& mv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
      mv = CGraphics::g_GXModelView.toMatrix4f();
      dataCur += sizeof(zeus::CMatrix4f);

      zeus::CMatrix4f& mvinv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
      mvinv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
      dataCur += sizeof(zeus::CMatrix4f);

      zeus::CMatrix4f& proj = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
      proj = CGraphics::GetPerspectiveProjectionMatrix(true);
      dataCur += sizeof(zeus::CMatrix4f);

      dataCur = dataOut + ROUND_UP_256(dataCur - dataOut);
    }
  } else {
    /* Non-Skinned */
    zeus::CMatrix4f& mv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
    mv = CGraphics::g_GXModelView.toMatrix4f();
    dataCur += sizeof(zeus::CMatrix4f);

    zeus::CMatrix4f& mvinv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
    mvinv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    dataCur += sizeof(zeus::CMatrix4f);

    zeus::CMatrix4f& proj = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
    proj = CGraphics::GetPerspectiveProjectionMatrix(true);
    dataCur += sizeof(zeus::CMatrix4f);

    dataCur = dataOut + ROUND_UP_256(dataCur - dataOut);
  }

  CBooModel::UVAnimationBuffer::Update(dataCur, matSet, flags, parent);
  buf->unmap();
}

void GeometryUniformLayout::ReserveSharedBuffers(boo::IGraphicsDataFactory::Context& ctx, int size) {
  if (m_sharedBuffer.size() < size)
    m_sharedBuffer.resize(size);
  for (int i = 0; i < size; ++i) {
    auto& buf = m_sharedBuffer[i];
    if (!buf)
      buf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, m_geomBufferSize, 1);
  }
}

boo::ObjToken<boo::IGraphicsBufferD> GeometryUniformLayout::GetSharedBuffer(int idx) const {
  if (idx >= m_sharedBuffer.size())
    m_sharedBuffer.resize(idx + 1);

  auto& buf = m_sharedBuffer[idx];
  if (!buf) {
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
      buf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, m_geomBufferSize, 1);
      return true;
    } BooTrace);
  }

  return buf;
}

boo::ObjToken<boo::IGraphicsBufferD> CBooModel::UpdateUniformData(const CModelFlags& flags, const CSkinRules* cskr,
                                                                  const CPoseAsTransforms* pose, int sharedLayoutBuf) {
  if (!g_DummyTextures && !TryLockTextures())
    return {};

  /* Invalidate instances if new shadow being drawn */
  if ((flags.m_extendedShader == EExtendedShader::WorldShadow ||
       flags.m_extendedShader == EExtendedShader::LightingCubeReflectionWorldShadow) &&
      m_lastDrawnShadowMap != g_shadowMap) {
    m_lastDrawnShadowMap = g_shadowMap;
    m_instances.clear();
  }

  /* Invalidate instances if new one-texture being drawn */
  if (flags.m_extendedShader == EExtendedShader::Disintegrate && m_lastDrawnOneTexture != g_disintegrateTexture) {
    m_lastDrawnOneTexture = g_disintegrateTexture;
    m_instances.clear();
  }

  /* Invalidate instances if new reflection cube being drawn */
  if (hecl::com_cubemaps->toBoolean() && (flags.m_extendedShader == EExtendedShader::LightingCubeReflection ||
       flags.m_extendedShader == EExtendedShader::LightingCubeReflectionWorldShadow) &&
      m_lastDrawnReflectionCube != g_reflectionCube) {
    m_lastDrawnReflectionCube = g_reflectionCube;
    m_instances.clear();
  }

  const ModelInstance* inst;
  if (sharedLayoutBuf >= 0) {
    if (m_instances.size() <= sharedLayoutBuf) {
      do {
        inst = PushNewModelInstance(m_instances.size());
        if (!inst) {
          return {};
        }
      } while (m_instances.size() <= sharedLayoutBuf);
    } else {
      inst = &m_instances[sharedLayoutBuf];
    }
    m_uniUpdateCount = sharedLayoutBuf + 1;
  } else {
    if (m_instances.size() <= m_uniUpdateCount) {
      inst = PushNewModelInstance(sharedLayoutBuf);
      if (!inst) {
        return {};
      }
    } else {
      inst = &m_instances[m_uniUpdateCount];
    }
    ++m_uniUpdateCount;
  }

  if (inst->m_geomUniformBuffer) {
    m_geomLayout->Update(flags, cskr, pose, x4_matSet, inst->m_geomUniformBuffer, this);
  }

  u8* dataOut = reinterpret_cast<u8*>(inst->m_uniformBuffer->map(m_uniformDataSize));
  u8* dataCur = dataOut;

  if (flags.m_extendedShader == EExtendedShader::Thermal) /* Thermal Model (same as UV Mode 0) */
  {
    CModelShaders::ThermalUniform& thermalOut = *reinterpret_cast<CModelShaders::ThermalUniform*>(dataCur);
    thermalOut.mulColor = flags.x4_color;
    thermalOut.addColor = flags.addColor;
  } else if (flags.m_extendedShader >= EExtendedShader::SolidColor &&
             flags.m_extendedShader <= EExtendedShader::SolidColorBackfaceCullGreaterAlphaOnly) /* Solid color render */
  {
    CModelShaders::SolidUniform& solidOut = *reinterpret_cast<CModelShaders::SolidUniform*>(dataCur);
    solidOut.solidColor = flags.x4_color;
  } else if (flags.m_extendedShader == EExtendedShader::MorphBallShadow) /* MorphBall shadow render */
  {
    CModelShaders::MBShadowUniform& shadowOut = *reinterpret_cast<CModelShaders::MBShadowUniform*>(dataCur);
    shadowOut.shadowUp = CGraphics::g_GXModelView.rotate(zeus::skUp);
    shadowOut.shadowUp.w() = flags.x4_color.a();
    shadowOut.shadowId = flags.x4_color.r();
  } else if (flags.m_extendedShader == EExtendedShader::Disintegrate) {
    CModelShaders::OneTextureUniform& oneTexOut = *reinterpret_cast<CModelShaders::OneTextureUniform*>(dataCur);
    oneTexOut.addColor = flags.addColor;
    oneTexOut.fog = CGraphics::g_Fog;
  } else {
    CModelShaders::LightingUniform& lightingOut = *reinterpret_cast<CModelShaders::LightingUniform*>(dataCur);
    lightingOut = m_lightingData;
    lightingOut.colorRegs = CGraphics::g_ColorRegs;
    lightingOut.mulColor = flags.x4_color;
    lightingOut.addColor = flags.addColor;
    lightingOut.fog = CGraphics::g_Fog;
  }

  dataCur += sizeof(CModelShaders::LightingUniform);
  dataCur = dataOut + ROUND_UP_256(dataCur - dataOut);

  /* Reflection texmtx uniform */
  zeus::CMatrix4f* identMtxs = reinterpret_cast<zeus::CMatrix4f*>(dataCur);
  identMtxs[0] = zeus::CMatrix4f();
  identMtxs[1] = zeus::CMatrix4f();
  u8* curReflect = dataCur + 256;
  for (const CBooSurface& surf : *x0_surfaces) {
    const MaterialSet::Material& mat = x4_matSet->materials.at(surf.m_data.matIdx);
    if (mat.flags.samusReflection() || mat.flags.samusReflectionSurfaceEye()) {
      zeus::CMatrix4f* mtxs = reinterpret_cast<zeus::CMatrix4f*>(curReflect);
      float& alpha = reinterpret_cast<float&>(mtxs[2]);
      curReflect += 256;
      EnsureViewDepStateCached(*this, mat.flags.samusReflectionSurfaceEye() ? &surf : nullptr, mtxs, alpha);
    }
  }

  inst->m_uniformBuffer->unmap();
  return inst->m_dynamicVbo;
}

void CBooModel::DrawAlpha(const CModelFlags& flags, const CSkinRules* cskr, const CPoseAsTransforms* pose) {
  CModelFlags rFlags = flags;
  /* Check if we're overriding with RenderModelBlack */
  if (g_RenderModelBlack) {
    rFlags.m_extendedShader = EExtendedShader::SolidColor;
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
    rFlags.m_extendedShader = EExtendedShader::SolidColor;
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
    rFlags.m_extendedShader = EExtendedShader::SolidColor;
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
  auto ret = std::make_unique<CBooModel>(m_selfToken, this, &x8_surfaces, x18_matSets[shaderIdx], m_staticVbo, m_ibo,
                                         m_aabb, (m_flags & 0x2) != 0, subInsts);
  if (lockParent)
    ret->LockParent();
  return ret;
}

CModelShaders::ShaderPipelines SShader::BuildShader(const hecl::HMDLMeta& meta, const MaterialSet::Material& mat) {
  hecl::Backend::ReflectionType reflectionType;
  if (mat.flags.samusReflectionIndirectTexture())
    reflectionType = hecl::Backend::ReflectionType::Indirect;
  else if (mat.flags.samusReflection() || mat.flags.samusReflectionSurfaceEye())
    reflectionType = hecl::Backend::ReflectionType::Simple;
  else
    reflectionType = hecl::Backend::ReflectionType::None;
  hecl::Backend::ShaderTag tag(mat.hash, meta.colorCount, meta.uvCount, meta.weightCount, meta.weightCount * 4,
                               boo::Primitive(meta.topology), reflectionType, true, true, true, mat.flags.alphaTest());
  return CModelShaders::BuildExtendedShader(tag, mat);
}

void SShader::BuildShaders(const hecl::HMDLMeta& meta,
                           std::unordered_map<int, CModelShaders::ShaderPipelines>& shaders) {
  shaders.reserve(m_matSet.materials.size());
  int idx = 0;
  for (const MaterialSet::Material& mat : m_matSet.materials)
    shaders[idx++] = BuildShader(meta, mat);
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
    matSet.BuildShaders(m_hmdlMeta);
  }

  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    /* Index buffer is always static */
    if (m_hmdlMeta.indexCount)
      m_ibo = ctx.newStaticBuffer(boo::BufferUse::Index, iboData, 4, m_hmdlMeta.indexCount);

    if (!m_hmdlMeta.bankCount) {
      /* Non-skinned models use static vertex buffers shared with CBooModel instances */
      if (m_hmdlMeta.vertCount)
        m_staticVbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, vboData, m_hmdlMeta.vertStride, m_hmdlMeta.vertCount);
    } else {
      /* Skinned models use per-instance dynamic buffers for vertex manipulation effects */
      size_t vboSz = m_hmdlMeta.vertStride * m_hmdlMeta.vertCount;
      if (vboSz) {
        m_dynamicVertexData.reset(new uint8_t[vboSz]);
        memmove(m_dynamicVertexData.get(), vboData, vboSz);
      }
    }

    return true;
  } BooTrace);

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

size_t CModel::GetPoolVertexOffset(size_t idx) const { return m_hmdlMeta.vertStride * idx; }

zeus::CVector3f CModel::GetPoolVertex(size_t idx) const {
  auto* floats = reinterpret_cast<const float*>(m_dynamicVertexData.get() + GetPoolVertexOffset(idx));
  return {floats};
}

size_t CModel::GetPoolNormalOffset(size_t idx) const { return m_hmdlMeta.vertStride * idx + 12; }

zeus::CVector3f CModel::GetPoolNormal(size_t idx) const {
  auto* floats = reinterpret_cast<const float*>(m_dynamicVertexData.get() + GetPoolNormalOffset(idx));
  return {floats};
}

void CModel::ApplyVerticesCPU(const boo::ObjToken<boo::IGraphicsBufferD>& vertBuf,
                              const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) const {
  u8* data = reinterpret_cast<u8*>(vertBuf->map(m_hmdlMeta.vertStride * m_hmdlMeta.vertCount));
  for (u32 i = 0; i < std::min(u32(vn.size()), m_hmdlMeta.vertCount); ++i) {
    const std::pair<zeus::CVector3f, zeus::CVector3f>& avn = vn[i];
    float* floats = reinterpret_cast<float*>(data + GetPoolVertexOffset(i));
    floats[0] = avn.first.x();
    floats[1] = avn.first.y();
    floats[2] = avn.first.z();
    floats[3] = avn.second.x();
    floats[4] = avn.second.y();
    floats[5] = avn.second.z();
  }
  vertBuf->unmap();
}

void CModel::RestoreVerticesCPU(const boo::ObjToken<boo::IGraphicsBufferD>& vertBuf) const {
  size_t size = m_hmdlMeta.vertStride * m_hmdlMeta.vertCount;
  u8* data = reinterpret_cast<u8*>(vertBuf->map(size));
  memcpy(data, m_dynamicVertexData.get(), size);
  vertBuf->unmap();
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
    GetInstance().WarmupDrawSurfaces();
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

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms, CObjectReference* selfRef) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  CFactoryFnReturn ret = TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, sp, selfRef));
  return ret;
}

} // namespace urde
