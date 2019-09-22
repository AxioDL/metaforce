#pragma once

#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CDecalDescription.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
struct SQuadDescr;
struct CQuadDecal {
  union {
    struct {
      bool x0_24_invalid : 1;
    };
    u32 _dummy = 0;
  };
  s32 x4_lifetime = 0;
  float x8_rotation = 0.f;
  const SQuadDescr* m_desc = nullptr;
  CQuadDecal() = default;
  CQuadDecal(s32 i, float f) : x4_lifetime(i), x8_rotation(f) { x0_24_invalid = true; }

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
  CQuadDecal x3c_decalQuads[2];
  s32 x54_modelLifetime = 0;
  s32 x58_frameIdx = 0;
  union {
    struct {
      bool x5c_31_quad1Invalid : 1;
      bool x5c_30_quad2Invalid : 1;
      bool x5c_29_modelInvalid : 1;
    };
    u32 x5c_dummy = 0;
  };
  zeus::CVector3f x60_rotation;
  bool InitQuad(CQuadDecal& quad, const SQuadDescr& desc);

public:
  CDecal(const TToken<CDecalDescription>& desc, const zeus::CTransform& xf);
  void RenderQuad(CQuadDecal& decal, const SQuadDescr& desc) const;
  void RenderMdl() const;
  void Render() const;
  void Update(float dt);

  static void SetGlobalSeed(u16);
  static void SetMoveRedToAlphaBuffer(bool);
};
} // namespace urde
