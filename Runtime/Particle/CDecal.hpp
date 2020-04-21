#pragma once

#include <array>

#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CDecalDescription.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
struct SQuadDescr;
struct CQuadDecal {
  bool x0_24_invalid : 1 = true;
  s32 x4_lifetime = 0;
  float x8_rotation = 0.f;
  const SQuadDescr* m_desc = nullptr;
  CQuadDecal() = default;
  CQuadDecal(s32 i, float f) : x4_lifetime(i), x8_rotation(f) {}

  boo::ObjToken<boo::IGraphicsBufferD> m_instBuf;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniformBuf;
  boo::ObjToken<boo::IShaderDataBinding> m_normalDataBind;
  boo::ObjToken<boo::IShaderDataBinding> m_redToAlphaDataBind;
};

class CDecal {
  friend class CDecalManager;
  static bool sMoveRedToAlphaBuffer;
  static CRandom16 sDecalRandom;

  TLockedToken<CDecalDescription> x0_description;
  zeus::CTransform xc_transform;
  std::array<CQuadDecal, 2> x3c_decalQuads;
  s32 x54_modelLifetime = 0;
  s32 x58_frameIdx = 0;
  bool x5c_31_quad1Invalid : 1 = false;
  bool x5c_30_quad2Invalid : 1 = false;
  bool x5c_29_modelInvalid : 1 = false;
  zeus::CVector3f x60_rotation;
  bool InitQuad(CQuadDecal& quad, const SQuadDescr& desc);

public:
  CDecal(const TToken<CDecalDescription>& desc, const zeus::CTransform& xf);
  void RenderQuad(CQuadDecal& decal, const SQuadDescr& desc) const;
  void RenderMdl();
  void Render();
  void Update(float dt);

  static void SetGlobalSeed(u16);
  static void SetMoveRedToAlphaBuffer(bool);
};
} // namespace urde
