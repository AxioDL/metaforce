#pragma once

#include <array>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/Shaders/CEnvFxShaders.hpp"

#include <hecl/UniformBufferPool.hpp>
#include <hecl/VertexBufferPool.hpp>

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2i.hpp>

namespace urde {
class CActor;
class CStateManager;
class CTexture;

enum class EEnvFxType { None, Snow, Rain, UnderwaterFlake };

enum class EPhazonType { None, Blue, Orange };

class CVectorFixed8_8 {
public:
  union {
    struct {
      s16 x, y, z;
    };
    std::array<s16, 3> v{};
  };
  CVectorFixed8_8() = default;
  CVectorFixed8_8(s16 xi, s16 yi, s16 zi) : v{xi, yi, zi} {}
  CVectorFixed8_8(const zeus::CVector3f& vec) {
    zeus::simd_floats f(vec.mSimd);
    x = s16(f[0] * 256.f);
    y = s16(f[1] * 256.f);
    z = s16(f[2] * 256.f);
  }
  CVectorFixed8_8 operator+(const CVectorFixed8_8& other) const {
    return {s16(x + other.x), s16(y + other.y), s16(z + other.z)};
  }
  CVectorFixed8_8 operator-(const CVectorFixed8_8& other) const {
    return {s16(x - other.x), s16(y - other.y), s16(z - other.z)};
  }
  zeus::CVector3f toVec3f() const { return {x / 256.f, y / 256.f, z / 256.f}; }
};

class CEnvFxManagerGrid {
  friend class CEnvFxManager;
  friend class CEnvFxShaders;
  bool x0_24_blockDirty = true;
  zeus::CVector2i x4_position;                         /* 8.8 fixed point */
  zeus::CVector2i xc_extent;                           /* 8.8 fixed point */
  std::pair<bool, float> x14_block = {false, FLT_MAX}; /* Blocked-bool, Z-coordinate */
  std::vector<CVectorFixed8_8> x1c_particles;

  hecl::VertexBufferPool<CEnvFxShaders::Instance>::Token m_instBuf;
  hecl::UniformBufferPool<CEnvFxShaders::Uniform>::Token m_uniformBuf;
  CLineRenderer m_lineRenderer;

  boo::ObjToken<boo::IShaderDataBinding> m_snowBinding;
  boo::ObjToken<boo::IShaderDataBinding> m_underwaterBinding;

  float m_uvyOffset = 0.f;

  void RenderSnowParticles(const zeus::CTransform& camXf);
  void RenderRainParticles(const zeus::CTransform& camXf);
  void RenderUnderwaterParticles(const zeus::CTransform& camXf);

public:
  CEnvFxManagerGrid(const zeus::CVector2i& position, const zeus::CVector2i& extent,
                    std::vector<CVectorFixed8_8> initialParticles, int reserve, CEnvFxManager& parent,
                    boo::IGraphicsDataFactory::Context& ctx);
  void Render(const zeus::CTransform& xf, const zeus::CTransform& invXf, const zeus::CTransform& camXf, float fxDensity,
              EEnvFxType fxType, CEnvFxManager& parent);
};

class CEnvFxManager {
  friend class CEnvFxManagerGrid;
  friend class CEnvFxShaders;

  hecl::VertexBufferPool<CEnvFxShaders::Instance> m_instPool;
  hecl::UniformBufferPool<CEnvFxShaders::Uniform> m_uniformPool;
  CEnvFxShaders::Uniform m_uniformData;
  boo::ObjToken<boo::IGraphicsBufferD> m_fogUniformBuf;

  zeus::CAABox x0_particleBounds = zeus::CAABox(-63.5f, 63.5f);
  zeus::CVector3f x18_focusCellPosition = zeus::skZero3f;
  bool x24_enableSplash = false;
  float x28_firstSnowForce = 0.f;
  s32 x2c_lastBlockedGridIdx = -1;
  float x30_fxDensity = 0.f;
  float x34_targetFxDensity = 0.f;
  float x38_maxDensityDeltaSpeed = 0.f;
  // bool x3c_snowflakeTextureMipBlanked = false; /* Shader simulates this texture mod */
  TLockedToken<CTexture> x40_txtrEnvGradient;
  rstl::reserved_vector<CEnvFxManagerGrid, 64> x50_grids;
  float xb54_baseSplashRate = 0.f;
  TLockedToken<CGenDescription> xb58_envRainSplash;
  bool xb64_ = true;
  TUniqueId xb68_envRainSplashId = kInvalidUniqueId;
  bool xb6a_rainSoundActive = false;
  CSfxHandle xb6c_leftRainSound;
  CSfxHandle xb70_rightRainSound;
  TLockedToken<CTexture> xb74_txtrSnowFlake;
  bool xb80_ = true;
  rstl::reserved_vector<zeus::CVector3f, 16> xb84_snowZDeltas;
  TLockedToken<CTexture> xc48_underwaterFlake;
  bool xc54_ = true;

  void SetSplashEffectRate(float f, const CStateManager& mgr);
  void UpdateRainSounds(const CStateManager& mgr);
  zeus::CVector3f GetParticleBoundsToWorldScale() const;
  zeus::CTransform GetParticleBoundsToWorldTransform() const;
  void UpdateVisorSplash(CStateManager& mgr, float dt, const zeus::CTransform& camXf);
  void MoveWrapCells(s32, s32);
  void CalculateSnowForces(const CVectorFixed8_8& zVec, rstl::reserved_vector<CVectorFixed8_8, 256>& snowForces,
                           EEnvFxType type, const zeus::CVector3f& oopbtws, float dt);
  static void BuildBlockObjectList(rstl::reserved_vector<TUniqueId, 1024>& list, CStateManager& mgr);
  void UpdateBlockedGrids(CStateManager& mgr, EEnvFxType type, const zeus::CTransform& camXf,
                          const zeus::CTransform& xf, const zeus::CTransform& invXf);
  void CreateNewParticles(EEnvFxType type);
  void UpdateSnowParticles(const rstl::reserved_vector<CVectorFixed8_8, 256>& snowForces);
  void UpdateRainParticles(const CVectorFixed8_8& zVec, const zeus::CVector3f& oopbtws, float dt);
  void UpdateUnderwaterParticles(const CVectorFixed8_8& zVec);
  void SetupSnowTevs(const CStateManager& mgr);
  void SetupRainTevs() const;
  void SetupUnderwaterTevs(const zeus::CTransform& invXf, const CStateManager& mgr) const;

public:
  CEnvFxManager();
  void AsyncLoadResources(CStateManager& mgr);

  void Update(float, CStateManager& mgr);
  void Render(const CStateManager& mgr);
  void SetFxDensity(s32, float);
  void AreaLoaded();
  void SetSplashRate(float f) { xb54_baseSplashRate = f; }
  bool IsSplashActive() const { return x24_enableSplash; }
  float GetRainMagnitude() const { return x30_fxDensity; }
  void Cleanup();

  static void Initialize();
};

} // namespace urde
