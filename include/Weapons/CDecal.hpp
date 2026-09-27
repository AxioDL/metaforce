#ifndef _CDECAL
#define _CDECAL

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/TToken.hpp"
#include "Weapons/CDecalDescription.hpp"

#include <types.h>

class CDecalDescription;
class CDecal {
  static CRandom16 sDecalRandom;
  static bool sMoveRedToAlphaBuffer;

public:
  class CQuadDecal {
  public:
    CQuadDecal() : mInvalid(true), mLifetime(0), mRotation(0.f) {}
    CQuadDecal(int lifetime, float rotation)
    : mInvalid(true), mLifetime(lifetime), mRotation(rotation) {}

    inline bool IsInvalid() const { return mInvalid; }
    inline void SetInvalid(bool invalid) { mInvalid = invalid; }

    inline int GetLifetime() const { return mLifetime; }
    inline void SetLifetime(int lifetime) { mLifetime = lifetime; }

    inline float GetRotation() const { return mRotation; }
    inline void SetRotation(float rotation) { mRotation = rotation; }

    // private:
    bool mInvalid : 1;
    int mLifetime;
    float mRotation;
  };

  static void SetGlobalSeed(ushort seed);
  static void SetMoveRedToAlphaBuffer(const bool move) { sMoveRedToAlphaBuffer = move; }
  CDecal(const TToken< CDecalDescription >& desc, const CTransform4f& xf);

  void RenderQuad(CQuadDecal& quad, const CDecalDescription::SQuadDescr& quadDesc) const;
  void RenderMdl() const;
  void Render() const;
  void Update(float dt);

  bool IsDone() const { return mFlags == 7; }
  CVector3f GetTranslation() const { return mTransform.GetTranslation(); }

private:
  TLockedToken< CDecalDescription > mDescription;
  CTransform4f mTransform;
  CQuadDecal mQuad1;
  CQuadDecal mQuad2;
  int mModelLifetime;
  int mFrameIdx;
  int mFlags;
  mutable CVector3f mRotation;

  void InitQuad(CQuadDecal& quad, const CDecalDescription::SQuadDescr& desc, int flag) {
    if (!desc.mTEX.null()) {
      if (!desc.mLFT.null()) {
        desc.mLFT->GetValue(0, quad.mLifetime);
      } else {
        quad.mLifetime = 0x7FFFFF;
      }

      if (!desc.mROT.null()) {
        desc.mROT->GetValue(0, quad.mRotation);
        quad.mInvalid &= desc.mROT->IsConstant();
      }

      if (!desc.mSZE.null()) {
        quad.mInvalid &= desc.mSZE->IsConstant();
        if (quad.mInvalid) {
          float size = 1.f;
          desc.mSZE->GetValue(0, size);
          quad.mInvalid = size <= 1.f;
        }
      }

      if (!desc.mOFF.null()) {
        quad.mInvalid &= desc.mOFF->IsFastConstant();
      }
    } else {
      quad.mInvalid = false;
      mFlags |= flag;
    }
  }

  void ProcessQuad(CQuadDecal& quad, const CDecalDescription::SQuadDescr& desc, int flag) const {
    if (!desc.mTEX.null() && (mFlags & flag) == 0) {
      CParticleGlobals::SetParticleLifetime(quad.mLifetime);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(mFrameIdx);
      RenderQuad(quad, desc);
    }
  }
};
#endif // _CDECAL
