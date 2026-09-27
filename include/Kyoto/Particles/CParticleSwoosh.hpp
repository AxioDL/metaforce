#ifndef _CPARTICLESWOOSH
#define _CPARTICLESWOOSH

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"

#include "dolphin/gx/GXEnum.h"

class CParticleSwoosh : public CParticleGen {
  friend class CParticleElectric;

public:
  struct SSwooshData {
    SSwooshData(const CVector3f& translation = CVector3f::Zero(),
                const CVector3f& offset = CVector3f::Zero(), float irot = 0.f, float rotm = 0.f,
                int startFrame = 0, bool active = false,
                const CTransform4f& orient = CTransform4f::Identity(),
                const CVector3f& velocity = CVector3f::Zero(), float leftRadius = 0.f,
                float rightRadius = 0.f, const CColor& color = CColor(0.f, 0.f, 0.f, 0.f))
    : mActive(active)
    , mLeftRad(leftRadius)
    , mRightRad(rightRadius)
    , mTranslation(translation)
    , mOffset(offset)
    , mUseOffset(offset)
    , mInitialRot(irot)
    , mRotm(rotm)
    , mOrientation(orient)
    , mColor(color)
    , mStartFrame(startFrame)
    , mVelocity(velocity) {}

    bool mActive;
    float mLeftRad;
    float mRightRad;
    CVector3f mTranslation;    // Updated by system's velocity sources or user code
    CVector3f mOffset;         // Updated by POFS once per system update (also resets x24_useOffset)
    CVector3f mUseOffset;      // Combination of POFS and NPOS, once per particle instance
    float mInitialRot;         // Rotation bias once per system update
    float mRotm;               // Rotation bias once per particle instance
    CTransform4f mOrientation; // Updated by user code
    int mFrame;                // Frame index of evaluated data
    CColor mColor;             // Updated by COLR
    int mStartFrame;
    CVector3f mVelocity;
  };
  CParticleSwoosh(TToken< CSwooshDescription > desc, int i);
  ~CParticleSwoosh();

  const bool Update(double dt) override;
  void Render() override;
  void Render2SidedNoSplineNoGaps();
  void Render2SidedNoSplineGaps();
  void Render2SidedSpline();
  void Render3SidedSolidNoSplineNoGaps();
  void Render3SidedSolidSpline();
  void RenderNSidedNoSpline();
  void RenderNSidedSpline();
  void SetOrientation(const CTransform4f& orientation) override;
  void UpdateTranslationAndOrientation();
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
  void SetParticleEmission(bool emission) override;
  void SetModulationColor(const CColor& col) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override;
  bool GetParticleEmission() const override;
  const CColor& GetModulationColor() const override;
  bool IsSystemDeletable() const override;
  rstl::optional_object< CAABox > GetBounds() const override;
  int GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  static int GetAliveParticleSystemCount();

  void SetWarmUp() { mForceOneUpdate = true; }
  void SetRenderGaps(bool gaps) { mRenderGaps = gaps; }

  const int GetSwooshCount() const { return mSwooshes.size(); }
  int GetCurParticle() const { return mCurParticle; }
  rstl::vector< SSwooshData >& Swooshes() { return mSwooshes; }
  const rstl::vector< SSwooshData >& GetSwooshes() const { return mSwooshes; }

  bool IsLargeEnough() const;
  void UpdateSwooshTranslation(const CVector3f& translation);

  int WrapIndex(int index);
  float GetLeftRadius(int index);
  float GetRightRadius(int index);
  CVector3f GetSplinePoint(const CVector3f& p0, const CVector3f& p1, const CVector3f& p2,
                           const CVector3f& p3, float t) const;
  void UpdateBounds(const CVector3f& pos);
  void UpdateMaxRadius(float radius);

private:
  TLockedToken< CSwooshDescription > mDesc;
  int mCurFrame;
  int mPSLT;
  double mCurTime;
  CVector3f mTranslation;
  CTransform4f mOrientation;
  CTransform4f mInvOrientation;
  CVector3f mGlobalTranslation;
  CTransform4f mGlobalOrientation;
  CVector3f mGlobalScale;
  CTransform4f mScaleXf;
  CTransform4f mInvScaleXf;
  CVector3f mLocalScale;
  int mCurParticle;
  rstl::vector< SSwooshData > mSwooshes;
  rstl::vector< CVector3f > mP0;
  rstl::vector< CVector3f > mP1;
  rstl::vector< CVector3f > mP2;
  rstl::vector< CVector3f > mP3;
  int mParticleCount;
  int mSPLN;
  int mLENG;
  int mSIDE;
  GXPrimitive mPrim;
  CRandom16 mRand;
  float x1c4_;
  float x1c8_;
  float mTSPNFloat;
  bool mEmitting : 1;
  bool mAALP : 1;
  bool mForceOneUpdate : 1;
  bool mRenderGaps : 1;
  bool mLLRD : 1;
  bool mVLS1 : 1;
  bool mVLS2 : 1;
  bool mConstantTex : 1;
  bool mConstantUv : 1;
  float mUMin;
  float mVMin;
  float mUMax;
  float mVMax;
  CTexture* mTex;
  float mUvSpan;
  int mTSPNInt;
  CVector3f mAabbMin;
  CVector3f mAabbMax;
  float mMaxRadius;
  CColor mModuColor;

  static uint mSwooshAliveCount;
};

CHECK_SIZEOF(CParticleSwoosh, 0x210)
NESTED_CHECK_SIZEOF(CParticleSwoosh, SSwooshData, 0x80)

#endif // _CPARTICLESWOOSH
