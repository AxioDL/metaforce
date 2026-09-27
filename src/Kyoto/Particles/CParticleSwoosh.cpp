#include "Kyoto/Particles/CParticleSwoosh.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"

#include "Kyoto/Math/CAbsAngle.hpp"
#include "dolphin/gx/GXVert.h"
#include "float.h"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"

uint CParticleSwoosh::mSwooshAliveCount = 0;
static float kFrameTime = 1.f / 60.f;

#pragma sym on

CParticleSwoosh::CParticleSwoosh(const TToken< CSwooshDescription > desc, const int leng)
: mDesc(desc)
, mCurFrame(0)
, mPSLT(0)
, mCurTime(0.0)
, mTranslation(0.f, 0.f, 0.f)
, mOrientation(CTransform4f::Identity())
, mInvOrientation(CTransform4f::Identity())
, mGlobalTranslation(0.f, 0.f, 0.f)
, mGlobalOrientation(CTransform4f::Identity())
, mGlobalScale(1.f, 1.f, 1.f)
, mScaleXf(CTransform4f::Identity())
, mInvScaleXf(CTransform4f::Identity())
, mLocalScale(1.f, 1.f, 1.f)
, mCurParticle(0)
, mParticleCount(0)
, mSPLN(0)
, mRand(mDesc->mCRND ? CCast::ToInt16(CStopwatch::GetGlobalMicros())
                                  : CCast::ToInt16(99))
, x1c4_(0.f)
, x1c8_(0.f)
, mEmitting(true)
, mForceOneUpdate(false)
, mRenderGaps(false)
, mConstantTex(false)
, mConstantUv(false)
, mUMin(0.f)
, mVMin(0.f)
, mUMax(0.f)
, mVMax(0.f)
, mTex(nullptr)
, mUvSpan(1.f)
, mTSPNInt(0)
, mAabbMin(CVector3f::Zero())
, mAabbMax(CVector3f::Zero())
, mMaxRadius(0.f)
, mModuColor(0xffffffff) {
  int i = 0;
  CGlobalRandom _(mRand);
  mSwooshAliveCount++;

  if (leng > 0) {
    mLENG = leng;
  } else if (mDesc->mLENG) {
    mDesc->mLENG->GetValue(0, mLENG);
  }
  ++mLENG;

  if (mDesc->mSIDE) {
    mDesc->mSIDE->GetValue(0, mSIDE);
  }

  mLLRD = mDesc->mLLRD;
  mVLS1 = mDesc->mVLS1;
  mVLS2 = mDesc->mVLS2;

  if (IsLargeEnough()) {
    if (mDesc->mPSLT) {
      mDesc->mPSLT->GetValue(0, mPSLT);
    } else {
      mPSLT = 0x7fffff;
    }

    mAALP = mDesc->mAALP;

    if (mDesc->mSPLN) {
      mDesc->mSPLN->GetValue(0, mSPLN);
    }
    if (mSPLN < 0) {
      mSPLN = 0;
    }

    mSwooshes.clear();
    mSwooshes.reserve(mLENG);
    for (i = 0; i < mSwooshes.capacity(); i++) {
      mSwooshes.push_back(SSwooshData());
    }

    SetOrientation(CTransform4f::Identity());

    mP0.clear();
    mP0.reserve(mSIDE);

    for (i = 0; i < mP0.capacity(); i++) {
      mP0.push_back(CVector3f::Zero());
    }
    mP1.clear();
    mP1.reserve(mSIDE);
    for (i = 0; i < mP1.capacity(); i++) {
      mP1.push_back(CVector3f::Zero());
    }

    mP2.clear();
    mP2.reserve(mSIDE);
    for (i = 0; i < mP2.capacity(); i++) {
      mP2.push_back(CVector3f::Zero());
    }
    mP3.clear();
    mP3.reserve(mSIDE);
    for (i = 0; i < mP3.capacity(); i++) {
      mP3.push_back(CVector3f::Zero());
    }
  }
}

CParticleSwoosh::~CParticleSwoosh() { --mSwooshAliveCount; }

bool CParticleSwoosh::IsLargeEnough() const { return mLENG >= 2 && mSIDE >= 2; }

const bool CParticleSwoosh::Update(double dt) {
  if (!IsLargeEnough()) {
    return false;
  }
  CStopwatch timer;
  CParticleGlobals::SetParticleLifetime(mLENG);
  CParticleGlobals::SetEmitterTime(mCurFrame);
  CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
  CGlobalRandom random(mRand);
  double evalTime = mCurFrame * kFrameTime;
  float timeScale = 1.f;
  if (mDesc->mTIME) {
    mDesc->mTIME->GetValue(mCurFrame, timeScale);
  }
  double advance = dt * timeScale;
  if (advance < 0.0) {
    advance = 0.0;
  }
  mCurTime += advance;
  while (mForceOneUpdate || evalTime < mCurTime) {
    mForceOneUpdate = false;
    if (++mCurParticle >= mSwooshes.capacity()) {
      mCurParticle = 0;
    }
    if (mEmitting && mCurFrame < mPSLT) {
      UpdateSwooshTranslation(mTranslation);
      if (mDesc->mIROT) {
        mDesc->mIROT->GetValue(mCurFrame, mSwooshes[mCurParticle].mInitialRot);
      } else {
        mSwooshes[mCurParticle].mInitialRot = 0.f;
      }
      mSwooshes[mCurParticle].mRotm = 0.f;
      mSwooshes[mCurParticle].mStartFrame = mCurFrame;
      if (!mSwooshes[mCurParticle].mActive) {
        ++mParticleCount;
        mSwooshes[mCurParticle].mActive = true;
      }
      mSwooshes[mCurParticle].mOrientation = mOrientation;
      if (mDesc->mIVEL) {
        mDesc->mIVEL->GetValue(mCurFrame, mSwooshes[mCurParticle].mVelocity);
        mSwooshes[mCurParticle].mVelocity =
            mOrientation * mSwooshes[mCurParticle].mVelocity;
      }
      if (mDesc->mPOFS) {
        mDesc->mPOFS->GetValue(mCurFrame, mSwooshes[mCurParticle].mOffset);
      }
      mSwooshes[mCurParticle].mUseOffset = mSwooshes[mCurParticle].mOffset;
      if (mDesc->mCOLR) {
        mDesc->mCOLR->GetValue(mCurFrame, mSwooshes[mCurParticle].mColor);
      } else {
        mSwooshes[mCurParticle].mColor = CColor(1.f, 1.f, 1.f, 1.f);
      }
      int tspn = 0;
      if (mDesc->mTSPN) {
        mDesc->mTSPN->GetValue(mCurFrame, tspn);
      }
      mTSPNFloat = tspn;
    } else if (mSwooshes[mCurParticle].mActive) {
      mParticleCount = rstl::max_val(0, mParticleCount - 1);
      mSwooshes[mCurParticle].mActive = false;
    }
    UpdateTranslationAndOrientation();
    evalTime += kFrameTime;
    ++mCurFrame;
  }
  x1c4_ = timer.GetElapsedTime();
  return false;
}

void CParticleSwoosh::SetOrientation(const CTransform4f& orientation) {
  mOrientation = orientation;
  mInvOrientation = mOrientation.GetQuickInverse();
  mSwooshes[mCurParticle].mOrientation = mOrientation;
}

void CParticleSwoosh::UpdateTranslationAndOrientation() {
  if (mParticleCount <= 0) {
    return;
  }
  mMaxRadius = 0.f;
  mAabbMin = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
  mAabbMax = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  CParticleGlobals::SetParticleLifetime(mLENG);
  CParticleGlobals::SetEmitterTime(mCurFrame);
  int i = 0;
  CVector3f offset(0.f, 0.f, 0.f);
  CVector3f localVel(0.f, 0.f, 0.f);
  CVector3f localTrans(0.f, 0.f, 0.f);
  for (i = 0; i < mSwooshes.capacity(); ++i) {
    SSwooshData& swoosh = mSwooshes.data()[i];
    if (!swoosh.mActive) {
      continue;
    }
    swoosh.mFrame = mCurFrame - swoosh.mStartFrame;
    CParticleGlobals::UpdateParticleLifetimeTweenValues(swoosh.mFrame);
    if (mDesc->mSROT == true) {
      if (mDesc->mIROT) {
        mDesc->mIROT->GetValue(mCurFrame, swoosh.mInitialRot);
      }
      swoosh.mRotm = 0.f;
    } else {
      if (mDesc->mROTM) {
        mDesc->mROTM->GetValue(mCurFrame, swoosh.mRotm);
      } else {
        swoosh.mRotm = 0.f;
      }
    }
    if (mDesc->mVELM) {
      if (mVLS1) {
        localVel = mInvOrientation * swoosh.mVelocity;
        localTrans = mInvOrientation * (swoosh.mTranslation - mTranslation);
        mDesc->mVELM->GetValue(swoosh.mFrame, localVel, localTrans);
        swoosh.mVelocity = mOrientation * localVel;
        swoosh.mTranslation = mOrientation * localTrans + mTranslation;
      } else {
        mDesc->mVELM->GetValue(swoosh.mFrame, swoosh.mVelocity, swoosh.mTranslation);
      }
    }
    if (mDesc->mVLM2) {
      if (mVLS2) {
        localVel = mInvOrientation * swoosh.mVelocity;
        localTrans = mInvOrientation * (swoosh.mTranslation - mTranslation);
        mDesc->mVLM2->GetValue(swoosh.mFrame, localVel, localTrans);
        swoosh.mVelocity = mOrientation * localVel;
        swoosh.mTranslation = mOrientation * localTrans + mTranslation;
      } else {
        mDesc->mVLM2->GetValue(swoosh.mFrame, swoosh.mVelocity, swoosh.mTranslation);
      }
    }
    if (swoosh.mFrame > 0) {
      swoosh.mTranslation += swoosh.mVelocity;
    }
    if (mDesc->mNPOS) {
      mDesc->mNPOS->GetValue(swoosh.mFrame, offset);
      swoosh.mUseOffset = swoosh.mOffset + offset;
    }
    if (mDesc->mCOLR) {
      mDesc->mCOLR->GetValue(swoosh.mFrame, swoosh.mColor);
    }
    swoosh.mLeftRad = GetLeftRadius(i);
    UpdateMaxRadius(swoosh.mLeftRad);
    if (mLLRD) {
      swoosh.mRightRad = swoosh.mLeftRad;
    } else {
      swoosh.mRightRad = GetRightRadius(i);
      UpdateMaxRadius(swoosh.mRightRad);
    }
    UpdateBounds(swoosh.mTranslation + swoosh.mUseOffset);
  }
}

void CParticleSwoosh::SetTranslation(const CVector3f& translation) {
  mTranslation = translation;
  UpdateSwooshTranslation(mTranslation);
}

void CParticleSwoosh::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
}

void CParticleSwoosh::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation = orientation.GetRotation();
}

void CParticleSwoosh::SetLocalScale(const CVector3f& scale) { mLocalScale = scale; }

int CParticleSwoosh::GetParticleCount() const { return mParticleCount; }

void CParticleSwoosh::SetModulationColor(const CColor& col) { mModuColor = col; }

const CColor& CParticleSwoosh::GetModulationColor() const { return mModuColor; }

bool CParticleSwoosh::IsSystemDeletable() const {
  if (mEmitting && mCurFrame < mPSLT) {
    return false;
  }

  if (GetParticleCount() >= 2) {
    return false;
  }

  return true;
}

void CParticleSwoosh::Render() {
  if (mLENG >= 2 && mParticleCount > 1) {
    CStopwatch timer;
    CParticleGlobals::SetParticleLifetime(mLENG);
    CGlobalRandom random(mRand);
    CGraphics::DisableAllLights();
    CGraphics::SetDepthWriteMode(true, kE_LEqual, mDesc->mZBUF);
    if (mAALP) {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    } else {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    }
    CGraphics::SetModelMatrix(CTransform4f::Translate(mGlobalTranslation) *
                              mGlobalOrientation * mScaleXf *
                              CTransform4f::Scale(mLocalScale));
    CGraphics::SetCullMode(kCM_None);
    if (mDesc->mTEXR) {
      {
        TToken< CTexture > tex = mDesc->mTEXR->GetValueTexture(mCurFrame);
        tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        mTex = tex.operator->();
      }
      SUVElementSet uvs;
      mDesc->mTEXR->GetValueUV(mCurFrame, uvs);
      mUMin = uvs.xMin;
      mVMin = uvs.yMin;
      mUMax = uvs.xMax;
      mVMax = uvs.yMax;
      mConstantTex = mDesc->mTEXR->HasConstantTexture();
      mConstantUv = mDesc->mTEXR->HasConstantUV();
      if (mDesc->mTSPN) {
        mDesc->mTSPN->GetValue(mCurFrame, mTSPNInt);
      }
      if (mTSPNInt <= 0) {
        mTSPNInt = mSwooshes.size() - 1;
      }
      mUvSpan = 1.f;
      if (mTSPNInt > 0) {
        mUvSpan = 1.f / mTSPNInt;
      }
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    } else {
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    }
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    if (mSIDE == 2) {
      if (mSPLN <= 0) {
        if (mRenderGaps) {
          Render2SidedNoSplineGaps();
        } else {
          Render2SidedNoSplineNoGaps();
        }
      } else {
        Render2SidedSpline();
      }
    } else if (mSIDE == 3) {
      if (mSPLN > 0) {
        Render3SidedSolidSpline();
      } else {
        Render3SidedSolidNoSplineNoGaps();
      }
    } else {
      if (mSPLN > 0) {
        RenderNSidedSpline();
      } else {
        RenderNSidedNoSpline();
      }
    }
    x1c8_ = timer.GetElapsedTime();
    CGraphics::SetCullMode(kCM_Front);
  }
}

static inline float fast_sine(float x) {
  float c1 = 0.99980587f;
  float c3 = -0.16621658f;
  float c5 = 0.008087108f;
  float c7 = -0.000152977f;
  const float x2 = x * x;
  float power = x;
  float result = power * c1;
  power *= x2;
  result += power * c3;
  power *= x2;
  result += power * c5;
  power *= x2;
  result += power * c7;
  return result;
}

static inline float fast_cosine(float x) {
  float c0 = 1.f;
  float c2 = -0.49998003f;
  float c4 = 0.041620344f;
  float c6 = -0.0013636103f;
  float c8 = 0.000020169435f;
  const float x2 = x * x;
  float power = x2;
  float result = c0;
  result += power * c2;
  power *= x2;
  result += power * c4;
  power *= x2;
  result += power * c6;
  power *= x2;
  result += power * c8;
  return result;
}

void CParticleSwoosh::Render2SidedNoSplineNoGaps() {
  CGraphics::StreamBegin(kP_TriangleStrip);
  int particleCount = mParticleCount;
  int curIdx = mCurParticle;
  float uvOffset = 0.f;
  if (mDesc->mTEXR) {
    float angle = 0.f;
    float cosine = 0.f;
    float sine = 0.f;
    float leftCos = 0.f;
    float leftRadius = 0.f;
    float leftSin = 0.f;
    float rightRadius = 0.f;
    float rightSin = 0.f;
    if (mDesc->mORNT) {
      const CTransform4f inverse =
          (CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation * mScaleXf)
              .GetInverse();
      const CTransform4f camera = inverse * CGraphics::GetViewMatrix();
      const CVector3f camToParticle = camera.GetTranslation();
      CVector3f dotVec = CVector3f::Zero();
      for (int i = 0; i < mSwooshes.size(); ++i) {
        const SSwooshData& swoosh = mSwooshes[curIdx];
        --curIdx;
        if (curIdx < 0) {
          curIdx = mSwooshes.size() - 1;
        }
        if (swoosh.mActive) {
          --particleCount;
          int otherIdx = curIdx - 1;
          if (otherIdx < 0) {
            otherIdx = mSwooshes.size() - 1;
          }
          const SSwooshData& otherSwoosh = mSwooshes[otherIdx];
          CVector3f delta = otherSwoosh.mTranslation - swoosh.mTranslation;
          if (otherIdx == mCurParticle) {
            delta = swoosh.mTranslation -
                    mSwooshes[(curIdx + 1) % mSwooshes.size()].mTranslation;
          }
          if (delta.CanBeNormalized()) {
            CVector3f deltaCross = CVector3f::Cross(delta, camToParticle - swoosh.mTranslation);
            if (deltaCross.CanBeNormalized()) {
              deltaCross.Normalize();
              dotVec = (CVector3f::Dot(deltaCross, dotVec) < 0.f ? -1.f : 1.f) * deltaCross;
              const CVector3f v0 =
                  (swoosh.mTranslation + swoosh.mUseOffset) + swoosh.mLeftRad * dotVec;
              const CVector3f v1 =
                  (swoosh.mTranslation + swoosh.mUseOffset) + -1.f * (swoosh.mRightRad * dotVec);
              const uint color = CColor::Modulate(swoosh.mColor, mModuColor).GetColor_u32();
              CGraphics::StreamColor(color);
              CGraphics::StreamTexcoord(uvOffset, mVMin);
              CGraphics::StreamVertex(v0);
              CGraphics::StreamTexcoord(uvOffset, mVMax);
              CGraphics::StreamVertex(v1);
              if (uvOffset >= 1.f && particleCount) {
                CGraphics::StreamEnd();
                CGraphics::StreamBegin(kP_TriangleStrip);
                uvOffset -= 1.f;
                CGraphics::StreamColor(color);
                CGraphics::StreamTexcoord(uvOffset, mVMin);
                CGraphics::StreamVertex(v0);
                CGraphics::StreamTexcoord(uvOffset, mVMax);
                CGraphics::StreamVertex(v1);
              }
            }
          }
          if (mTSPNInt > 0) {
            uvOffset += mUvSpan;
          } else {
            uvOffset = mUvSpan * CCast::ToReal32(i);
          }
        }
      }
    } else {
      for (int i = 0; i < mSwooshes.size(); ++i) {
        const SSwooshData& swoosh = mSwooshes[curIdx];
        --curIdx;
        if (curIdx < 0) {
          curIdx = mSwooshes.size() - 1;
        }
        if (swoosh.mActive) {
          --particleCount;
          angle = M_PIF * (swoosh.mInitialRot + swoosh.mRotm) / 180.f;
          angle = fabs(angle) > M_PIF ? CMath::WrapPi(angle) : angle;
          cosine = fast_cosine(angle);
          sine = fast_sine(angle);
          leftRadius = swoosh.mLeftRad;
          rightRadius = swoosh.mRightRad;
          leftSin = leftRadius;
          leftSin *= sine;
          leftCos = leftRadius;
          leftCos *= cosine;
          rightSin = rightRadius;
          rightSin *= -sine;
          const CVector3f v0 = swoosh.mOrientation * CVector3f(leftRadius * cosine, 0.f, leftSin) +
                               swoosh.mTranslation + swoosh.mUseOffset;
          const CVector3f v1 =
              swoosh.mOrientation * CVector3f(rightRadius * -cosine, 0.f, rightSin) +
              swoosh.mTranslation + swoosh.mUseOffset;
          const uint color = CColor::Modulate(swoosh.mColor, mModuColor).GetColor_u32();
          CGraphics::StreamColor(color);
          CGraphics::StreamTexcoord(uvOffset, mVMin);
          CGraphics::StreamVertex(v0);
          CGraphics::StreamTexcoord(uvOffset, mVMax);
          CGraphics::StreamVertex(v1);
          if (uvOffset >= 1.f && particleCount) {
            CGraphics::StreamEnd();
            CGraphics::StreamBegin(kP_TriangleStrip);
            uvOffset -= 1.f;
            CGraphics::StreamColor(color);
            CGraphics::StreamTexcoord(uvOffset, mVMin);
            CGraphics::StreamVertex(v0);
            CGraphics::StreamTexcoord(uvOffset, mVMax);
            CGraphics::StreamVertex(v1);
          }
          if (mTSPNInt > 0) {
            uvOffset += mUvSpan;
          } else {
            uvOffset = mUvSpan * CCast::ToReal32(i);
          }
        }
      }
    }
  } else {
    float angle = 0.f;
    float cosine = 0.f;
    float sine = 0.f;
    float leftCos = 0.f;
    float leftSin = 0.f;
    float leftRadius = 0.f;
    float rightSin = 0.f;
    float rightRadius = 0.f;
    for (int i = 0; i < mSwooshes.size(); ++i) {
      const SSwooshData& swoosh = mSwooshes.data()[curIdx];
      --curIdx;
      if (curIdx < 0) {
        curIdx = mSwooshes.size() - 1;
      }
      if (swoosh.mActive) {
        angle = M_PIF * (swoosh.mInitialRot + swoosh.mRotm) / 180.f;
        angle = fabs(angle) > M_PIF ? CMath::WrapPi(angle) : angle;
        cosine = fast_cosine(angle);
        sine = fast_sine(angle);
        leftRadius = swoosh.mLeftRad;
        rightRadius = swoosh.mRightRad;
        leftSin = leftRadius;
        leftSin *= sine;
        leftCos = leftRadius;
        leftCos *= cosine;
        rightSin = rightRadius;
        rightSin *= -sine;
        const CVector3f v0 = swoosh.mOrientation * CVector3f(leftRadius * cosine, 0.f, leftSin) +
                             swoosh.mTranslation + swoosh.mUseOffset;
        const CVector3f v1 = swoosh.mOrientation * CVector3f(rightRadius * -cosine, 0.f, rightSin) +
                             swoosh.mTranslation + swoosh.mUseOffset;
        CGraphics::StreamColor(CColor::Modulate(swoosh.mColor, mModuColor).GetColor_u32());
        CGraphics::StreamVertex(v0);
        CGraphics::StreamVertex(v1);
      }
    }
  }
  CGraphics::StreamEnd();
}

void CParticleSwoosh::Render2SidedNoSplineGaps() {
  int i = 0;
  bool streaming = false;
  int curIdx = mCurParticle;
  float angle;
  float cosine;
  float sine;
  float leftCos;
  float leftSin;
  float leftRadius;
  float rightSin;
  float rightRadius;
  for (i = 0; i < mSwooshes.size(); ++i) {
    const SSwooshData& swoosh = mSwooshes[curIdx];
    const SSwooshData& next = mSwooshes[WrapIndex(curIdx - 1)];
    --curIdx;
    if (curIdx < 0) {
      curIdx = mSwooshes.size() - 2;
    }
    if (!swoosh.mActive) {
      if (streaming == true) {
        streaming = false;
        CGraphics::StreamEnd();
      }
      continue;
    }
    if (!streaming) {
      if (next.mActive != true || i >= mSwooshes.size() - 2) {
        continue;
      }
      streaming = true;
      CGraphics::StreamBegin(kP_TriangleStrip);
    }
    angle = (swoosh.mInitialRot + swoosh.mRotm) * (M_PIF / 180.f);
    angle = fabs(angle) > M_PIF ? CMath::WrapPi(angle) : angle;
    cosine = fast_cosine(angle);
    sine = fast_sine(angle);
    leftRadius = swoosh.mLeftRad;
    rightRadius = swoosh.mRightRad;
    leftSin = leftRadius;
    leftSin *= sine;
    leftCos = leftRadius;
    leftCos *= cosine;
    rightSin = rightRadius;
    rightSin *= -sine;
    const CVector3f v0 = swoosh.mOrientation * CVector3f(leftCos, 0.f, leftSin) +
                         swoosh.mTranslation + swoosh.mUseOffset;
    const CVector3f v1 = swoosh.mOrientation * CVector3f(rightRadius * -cosine, 0.f, rightSin) +
                         swoosh.mTranslation + swoosh.mUseOffset;
    CGraphics::StreamColor(CColor::Modulate(swoosh.mColor, mModuColor).GetColor_u32());
    CGraphics::StreamTexcoord(1.f, mVMin);
    CGraphics::StreamVertex(v0);
    CGraphics::StreamTexcoord(1.f, mVMax);
    CGraphics::StreamVertex(v1);
    if (i < mSwooshes.size() - 1) {
      CGraphics::StreamTexcoord(0.f, mVMin);
      CGraphics::StreamVertex(v0);
      CGraphics::StreamTexcoord(0.f, mVMax);
      CGraphics::StreamVertex(v1);
    }
  }
  if (streaming == true) {
    CGraphics::StreamEnd();
  }
}
void CParticleSwoosh::Render2SidedSpline() { RenderNSidedSpline(); }

void CParticleSwoosh::Render3SidedSolidNoSplineNoGaps() {
  if (mSwooshes.size() < 2) {
    return;
  }
  rstl::reserved_vector< CVector3f, 2 > p0;
  p0.push_back(CVector3f::Zero());
  p0.push_back(CVector3f::Zero());
  rstl::reserved_vector< CVector3f, 2 > p1;
  p1.push_back(CVector3f::Zero());
  p1.push_back(CVector3f::Zero());
  rstl::reserved_vector< CVector3f, 2 > p2;
  p2.push_back(CVector3f::Zero());
  p2.push_back(CVector3f::Zero());
  static const GXVtxDescList skDescList[] = {{GX_VA_POS, GX_DIRECT},
                                             {GX_VA_CLR0, GX_DIRECT},
                                             {GX_VA_TEX0, GX_DIRECT},
                                             {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(skDescList);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  int cur = 0;
  int prev = 1;
  uint prevColor;
  uint color = 0;
  int curIdx = mCurParticle;
  bool lastActive = false;
  float uv = -mUvSpan;
  for (int i = 0; i < mSwooshes.size(); ++i) {
    const SSwooshData& swoosh = mSwooshes.data()[curIdx];
    --curIdx;
    if (curIdx < 0) {
      curIdx = mSwooshes.size() - 1;
    }
    float rawAngle = M_PIF * (swoosh.mInitialRot + swoosh.mRotm) / 180.f;
    float angle = fabs(rawAngle) > M_PIF ? CMath::WrapPi(rawAngle) : rawAngle;
    const float radius = swoosh.mLeftRad;
    const float cosinea = fast_cosine(angle);
    const float sinea = fast_sine(angle);
    const CVector3f a(radius * cosinea, 0.f, radius * sinea);
    angle += 2.0943952f;
    if (angle > M_PIF) {
      angle -= M_2PIF;
    }
    const float cosineb = fast_cosine(angle);
    const float sineb = fast_sine(angle);
    const CVector3f b(radius * cosineb, 0.f, radius * sineb);
    angle += 2.0943952f;
    if (angle > M_PIF) {
      angle -= M_2PIF;
    }
    const float cosinec = fast_cosine(angle);
    const float sinec = fast_sine(angle);
    const CVector3f c(radius * cosinec, 0.f, radius * sinec);
    const CVector3f offset = swoosh.mTranslation + swoosh.mUseOffset;
    p0[cur] = swoosh.mOrientation * a + offset;
    p1[cur] = swoosh.mOrientation * b + offset;
    p2[cur] = swoosh.mOrientation * c + offset;
    prevColor = color;
    if (!swoosh.mActive) {
      lastActive = false;
    } else if (!lastActive) {
      lastActive = true;
    } else {
      lastActive = true;
      color = CColor::Modulate(swoosh.mColor, mModuColor).GetColor_u32();
      const float prevUv = uv;
      uv += mUvSpan;
      CGX::Begin(GX_QUADS, GX_VTXFMT0, 12);
      GXPosition3f32(p0[cur].GetX(), p0[cur].GetY(), p0[cur].GetZ());
      GXColor1u32(color);
      GXTexCoord2f32(uv, mVMin);
      GXPosition3f32(p1[cur].GetX(), p1[cur].GetY(), p1[cur].GetZ());
      GXColor1u32(color);
      GXTexCoord2f32(uv, mVMax);
      GXPosition3f32(p1[prev].GetX(), p1[prev].GetY(), p1[prev].GetZ());
      GXColor1u32(prevColor);
      GXTexCoord2f32(prevUv, mVMax);
      GXPosition3f32(p0[prev].GetX(), p0[prev].GetY(), p0[prev].GetZ());
      GXColor1u32(prevColor);
      GXTexCoord2f32(prevUv, mVMin);
      GXPosition3f32(p1[cur].GetX(), p1[cur].GetY(), p1[cur].GetZ());
      GXColor1u32(color);
      GXTexCoord2f32(uv, mVMin);
      GXPosition3f32(p2[cur].GetX(), p2[cur].GetY(), p2[cur].GetZ());
      GXColor1u32(color);
      GXTexCoord2f32(uv, mVMax);
      GXPosition3f32(p2[prev].GetX(), p2[prev].GetY(), p2[prev].GetZ());
      GXColor1u32(prevColor);
      GXTexCoord2f32(prevUv, mVMax);
      GXPosition3f32(p1[prev].GetX(), p1[prev].GetY(), p1[prev].GetZ());
      GXColor1u32(prevColor);
      GXTexCoord2f32(prevUv, mVMin);
      GXPosition3f32(p2[cur].GetX(), p2[cur].GetY(), p2[cur].GetZ());
      GXColor1u32(color);
      GXTexCoord2f32(uv, mVMin);
      GXPosition3f32(p0[cur].GetX(), p0[cur].GetY(), p0[cur].GetZ());
      GXColor1u32(color);
      GXTexCoord2f32(uv, mVMax);
      GXPosition3f32(p0[prev].GetX(), p0[prev].GetY(), p0[prev].GetZ());
      GXColor1u32(prevColor);
      GXTexCoord2f32(prevUv, mVMax);
      GXPosition3f32(p2[prev].GetX(), p2[prev].GetY(), p2[prev].GetZ());
      GXColor1u32(prevColor);
      GXTexCoord2f32(prevUv, mVMin);
      CGX::End();
    }
    ++cur;
    ++prev;
    cur &= 1;
    prev &= 1;
  }
}

void CParticleSwoosh::Render3SidedSolidSpline() {
  if (mSwooshes.size() < 2) {
    return;
  }
  static const GXVtxDescList skDescList[] = {{GX_VA_POS, GX_DIRECT},
                                             {GX_VA_CLR0, GX_DIRECT},
                                             {GX_VA_TEX0, GX_DIRECT},
                                             {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(skDescList);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  float uvDelta;
  uint prevColor = 0;
  uint color = 0;
  int curIdx = mCurParticle;
  float curUvSpan = -mUvSpan;
  for (int i = 0; i < mSwooshes.size(); ++i) {
    const SSwooshData& swoosh = mSwooshes.data()[curIdx];
    --curIdx;
    if (curIdx < 0) {
      curIdx = mSwooshes.size() - 1;
    }
    float rawAngle = M_PIF * (swoosh.mInitialRot + swoosh.mRotm) / 180.f;
    float angle = fabs(rawAngle) > M_PIF ? CMath::WrapPi(rawAngle) : rawAngle;
    const float radius = swoosh.mLeftRad;
    const float cosinea = fast_cosine(angle);
    const float sinea = fast_sine(angle);
    const CVector3f a(radius * cosinea, 0.f, radius * sinea);
    angle += 2.0943952f;
    if (angle > M_PIF) {
      angle -= M_2PIF;
    }
    const float cosineb = fast_cosine(angle);
    const float sineb = fast_sine(angle);
    const CVector3f b(radius * cosineb, 0.f, radius * sineb);
    angle += 2.0943952f;
    if (angle > M_PIF) {
      angle -= M_2PIF;
    }
    const float cosinec = fast_cosine(angle);
    const float sinec = fast_sine(angle);
    const CVector3f c(radius * cosinec, 0.f, radius * sinec);
    const CVector3f offset = swoosh.mTranslation + swoosh.mUseOffset;
    if (i == 2) {
      mP3[0] = 2.f * mP1[0] - mP0[0];
      mP3[1] = 2.f * mP1[1] - mP0[1];
      mP3[2] = 2.f * mP1[2] - mP0[2];
    } else {
      mP3[0] = mP2[0];
      mP3[1] = mP2[1];
      mP3[2] = mP2[2];
    }
    mP2[0] = mP1[0];
    mP2[1] = mP1[1];
    mP2[2] = mP1[2];
    mP1[0] = mP0[0];
    mP1[1] = mP0[1];
    mP1[2] = mP0[2];
    mP0[0] = swoosh.mOrientation * a + offset;
    mP0[1] = swoosh.mOrientation * b + offset;
    mP0[2] = swoosh.mOrientation * c + offset;
    const uint priorColor = prevColor;
    prevColor = color;
    if (swoosh.mActive) {
      const float prevUvSpan = curUvSpan;
      color = CColor::Modulate(swoosh.mColor, mModuColor).GetColor_u32();
      curUvSpan += mUvSpan;
      if (i > 1) {
        float uv1 = 0.f;
        CVector3f v01 = CVector3f::Zero();
        CVector3f v11 = CVector3f::Zero();
        CVector3f v21 = CVector3f::Zero();
        uint c1 = 0;
        CGX::Begin(GX_QUADS, GX_VTXFMT0, (mSPLN + 1) * 12);
        uvDelta = prevUvSpan - curUvSpan;
        for (int j = 0; j < mSPLN + 1; ++j) {
          const float t1 = (j + 1) / static_cast< float >(mSPLN + 1);
          float uv0 = uv1;
          CVector3f v00 = v01;
          CVector3f v10 = v11;
          CVector3f v20 = v21;
          uint c0 = c1;
          if (j == 0) {
            const float t0 = j / static_cast< float >(mSPLN + 1);
            v00 = GetSplinePoint(mP0[0], mP1[0], mP2[0], mP3[0], t0);
            v10 = GetSplinePoint(mP0[1], mP1[1], mP2[1], mP3[1], t0);
            v20 = GetSplinePoint(mP0[2], mP1[2], mP2[2], mP3[2], t0);
            c0 = CColor::Lerp(prevColor, priorColor, t0);
            uv0 = t0 * uvDelta + curUvSpan;
          }
          v01 = GetSplinePoint(mP0[0], mP1[0], mP2[0], mP3[0], t1);
          v11 = GetSplinePoint(mP0[1], mP1[1], mP2[1], mP3[1], t1);
          v21 = GetSplinePoint(mP0[2], mP1[2], mP2[2], mP3[2], t1);
          c1 = CColor::Lerp(prevColor, priorColor, t1);
          uv1 = t1 * uvDelta + curUvSpan;
          GXPosition3f32(v00.GetX(), v00.GetY(), v00.GetZ());
          GXColor1u32(c0);
          GXTexCoord2f32(uv0, mVMin);
          GXPosition3f32(v10.GetX(), v10.GetY(), v10.GetZ());
          GXColor1u32(c0);
          GXTexCoord2f32(uv0, mVMax);
          GXPosition3f32(v11.GetX(), v11.GetY(), v11.GetZ());
          GXColor1u32(c1);
          GXTexCoord2f32(uv1, mVMax);
          GXPosition3f32(v01.GetX(), v01.GetY(), v01.GetZ());
          GXColor1u32(c1);
          GXTexCoord2f32(uv1, mVMin);
          GXPosition3f32(v10.GetX(), v10.GetY(), v10.GetZ());
          GXColor1u32(c0);
          GXTexCoord2f32(uv0, mVMin);
          GXPosition3f32(v20.GetX(), v20.GetY(), v20.GetZ());
          GXColor1u32(c0);
          GXTexCoord2f32(uv0, mVMax);
          GXPosition3f32(v21.GetX(), v21.GetY(), v21.GetZ());
          GXColor1u32(c1);
          GXTexCoord2f32(uv1, mVMax);
          GXPosition3f32(v11.GetX(), v11.GetY(), v11.GetZ());
          GXColor1u32(c1);
          GXTexCoord2f32(uv1, mVMin);
          GXPosition3f32(v20.GetX(), v20.GetY(), v20.GetZ());
          GXColor1u32(c0);
          GXTexCoord2f32(uv0, mVMin);
          GXPosition3f32(v00.GetX(), v00.GetY(), v00.GetZ());
          GXColor1u32(c0);
          GXTexCoord2f32(uv0, mVMax);
          GXPosition3f32(v01.GetX(), v01.GetY(), v01.GetZ());
          GXColor1u32(c1);
          GXTexCoord2f32(uv1, mVMax);
          GXPosition3f32(v21.GetX(), v21.GetY(), v21.GetZ());
          GXColor1u32(c1);
          GXTexCoord2f32(uv1, mVMin);
        }
        CGX::End();
      }
    }
  }
}

void CParticleSwoosh::RenderNSidedNoSpline() { RenderNSidedSpline(); }

void CParticleSwoosh::RenderNSidedSpline() {
  if (mDesc->mWIRE) {
    mPrim = GX_LINES;
  } else {
    mPrim = GX_QUADS;
  }
  CGraphics::StreamBegin(static_cast< ERglPrimitive >(mPrim));
  int i = 0;
  bool cros = mDesc->mCROS;
  if (mSIDE < 4 || mSIDE % 2 != 0) {
    cros = false;
  }
  int splineIdx;
  int curIdx = mCurParticle;
  for (i = 0; i < mSwooshes.size() - 1; ++i) {
    int j;
    int k;
    bool prevActive = mSwooshes[WrapIndex(curIdx - 1)].mActive;
    bool active = mSwooshes[WrapIndex(curIdx)].mActive;
    if (active == false || (active == true && prevActive == false)) {
      --curIdx;
      if (curIdx < 0) {
        curIdx = mSwooshes.size() - 1;
      }
      continue;
    }
    const SSwooshData& refSwoosh = mSwooshes.data()[curIdx];
    const float sideDiv = 360.f / mSIDE;
    for (j = 0; j < 4; ++j) {
      int refIdx = 0;
      if (j == 0) {
        refIdx = WrapIndex(curIdx + 1);
        if (!mSwooshes.data()[refIdx].mActive) {
          refIdx = curIdx;
        }
      } else if (j == 1) {
        refIdx = WrapIndex(curIdx);
      } else if (j == 2) {
        refIdx = WrapIndex(curIdx - 1);
      } else if (j == 3) {
        refIdx = WrapIndex(curIdx - 2);
        if (!mSwooshes.data()[refIdx].mActive) {
          refIdx = WrapIndex(curIdx - 1);
        }
      }
      if (mLENG == 2) {
        if (j == 0) {
          refIdx = WrapIndex(curIdx);
        } else if (j == 3) {
          refIdx = WrapIndex(curIdx - 1);
        }
      } else if (curIdx == mCurParticle && j == 0) {
        refIdx = mCurParticle;
      } else if (WrapIndex(mCurParticle + 2) == curIdx && j == 3) {
        refIdx = WrapIndex(mCurParticle + 1);
      } else if (i == mParticleCount - 2 && j == 3) {
        refIdx = 0;
      }
      const SSwooshData& swoosh = mSwooshes.data()[refIdx];
      const CTransform4f& orientation = swoosh.mOrientation;
      for (k = 0; k < mSIDE; ++k) {
        const float sideAngle = sideDiv * k;
        const float rawAngle = M_PIF * (sideAngle + (swoosh.mInitialRot + swoosh.mRotm)) / 180.f;
        const float angle = fabs(rawAngle) > M_PIF ? CMath::WrapPi(rawAngle) : rawAngle;
        const float cosine = fast_cosine(angle);
        const float sine = fast_sine(angle);
        float x;
        float radius;
        if (sideAngle > 0.f && sideAngle <= 180.f) {
          radius = swoosh.mLeftRad;
        } else {
          radius = swoosh.mRightRad;
        }
        x = radius * cosine;
        const float z = radius * sine;
        const CVector3f offset = swoosh.mTranslation + swoosh.mUseOffset;
        if (j == 0) {
          mP0[k] = orientation * CVector3f(x, 0.f, z) + offset;
        } else if (j == 1) {
          mP1[k] = orientation * CVector3f(x, 0.f, z) + offset;
        } else if (j == 2) {
          mP2[k] = orientation * CVector3f(x, 0.f, z) + offset;
        } else if (j == 3) {
          mP3[k] = orientation * CVector3f(x, 0.f, z) + offset;
        }
      }
    }
    if (mDesc->mTEXR) {
      if (mTSPNInt > 0) {
        mUMin = (i % mTSPNInt) * mUvSpan;
      } else {
        mUMin = i * mUvSpan;
      }
    }
    const float segUvSpan = mUvSpan / (mSPLN + 1);
    for (splineIdx = 0; splineIdx < mSPLN + 1; ++splineIdx) {
      const float t0 = splineIdx / static_cast< float >(mSPLN + 1);
      const float t1 = (splineIdx + 1) / static_cast< float >(mSPLN + 1);
      int faces = mSIDE;
      if (mSIDE <= 2) {
        faces = 1;
      } else if (cros == true) {
        faces = mSIDE / 2;
      }
      mUMax = mUMin + segUvSpan;
      for (k = 0; k < faces; ++k) {
        int other = k + 1;
        if (other >= mSIDE) {
          other = 0;
        }
        const uint color = CColor::Modulate(refSwoosh.mColor, mModuColor).GetColor_u32();
        if (cros) {
          other = k + mSIDE / 2;
          const CVector3f& p00 = mP0[k];
          const CVector3f& p10 = mP1[k];
          const CVector3f& p20 = mP2[k];
          const CVector3f& p30 = mP3[k];
          const CVector3f& p01 = mP0[other];
          const CVector3f& p11 = mP1[other];
          const CVector3f& p21 = mP2[other];
          const CVector3f& p31 = mP3[other];
          const CVector3f v0 = GetSplinePoint(p00, p10, p20, p30, t0);
          const CVector3f v1 = GetSplinePoint(p01, p11, p21, p31, t0);
          const CVector3f v2 = GetSplinePoint(p01, p11, p21, p31, t1);
          const CVector3f v3 = GetSplinePoint(p00, p10, p20, p30, t1);
          CGraphics::StreamColor(color);
          CGraphics::StreamTexcoord(mUMin, mVMin);
          CGraphics::StreamVertex(v0);
          CGraphics::StreamTexcoord(mUMin, mVMax);
          CGraphics::StreamVertex(v1);
          CGraphics::StreamTexcoord(mUMax, mVMax);
          CGraphics::StreamVertex(v2);
          CGraphics::StreamTexcoord(mUMax, mVMin);
          CGraphics::StreamVertex(v3);

        } else {
          const CVector3f& p00 = mP0[k];
          const CVector3f& p10 = mP1[k];
          const CVector3f& p20 = mP2[k];
          const CVector3f& p30 = mP3[k];
          const CVector3f& p01 = mP0[other];
          const CVector3f& p11 = mP1[other];
          const CVector3f& p21 = mP2[other];
          const CVector3f& p31 = mP3[other];
          const CVector3f v0 = GetSplinePoint(p00, p10, p20, p30, t0);
          const CVector3f v1 = GetSplinePoint(p01, p11, p21, p31, t0);
          const CVector3f v2 = GetSplinePoint(p01, p11, p21, p31, t1);
          const CVector3f v3 = GetSplinePoint(p00, p10, p20, p30, t1);
          if (mPrim == GX_LINES) {
            CGraphics::StreamVertex(v0);
            CGraphics::StreamVertex(v1);
            CGraphics::StreamVertex(v1);
            CGraphics::StreamVertex(v2);
            CGraphics::StreamVertex(v2);
            CGraphics::StreamVertex(v0);
            CGraphics::StreamVertex(v0);
            CGraphics::StreamVertex(v2);
            CGraphics::StreamVertex(v2);
            CGraphics::StreamVertex(v3);
            CGraphics::StreamVertex(v3);
            CGraphics::StreamVertex(v0);
          } else if (mPrim == GX_QUADS) {
            CGraphics::StreamColor(color);
            CGraphics::StreamTexcoord(mUMin, mVMin);
            CGraphics::StreamVertex(v0);
            CGraphics::StreamTexcoord(mUMin, mVMax);
            CGraphics::StreamVertex(v1);
            CGraphics::StreamColor(color);
            CGraphics::StreamTexcoord(mUMax, mVMax);
            CGraphics::StreamVertex(v2);
            CGraphics::StreamTexcoord(mUMax, mVMin);
            CGraphics::StreamVertex(v3);
          }
        }
      }
      if (mDesc->mTEXR && mSPLN > 0) {
        mUMin += segUvSpan;
      }
    }
    --curIdx;
    if (curIdx < 0) {
      curIdx = mSwooshes.size() - 1;
    }
  }
  CGraphics::StreamEnd();
}

void CParticleSwoosh::SetParticleEmission(const bool emission) { mEmitting = emission; }
int CParticleSwoosh::WrapIndex(int index) {
  for (; index < 0; index += mLENG)
    ;
  for (; index >= mLENG; index -= mLENG)
    ;
  return index;
}

float CParticleSwoosh::GetLeftRadius(int index) {
  float ret = 0.f;
  if (mDesc->mLRAD) {
    mDesc->mLRAD->GetValue(mSwooshes[index].mFrame, ret);
  }

  return ret;
}
float CParticleSwoosh::GetRightRadius(int index) {
  float ret = 0.f;
  if (mDesc->mRRAD) {
    mDesc->mRRAD->GetValue(mSwooshes[index].mFrame, ret);
  }

  return ret;
}

CVector3f CParticleSwoosh::GetSplinePoint(const CVector3f& p0, const CVector3f& p1,
                                          const CVector3f& p2, const CVector3f& p3, float t) const {
  CVector3f ret(p1);
  if (!(t <= 0.f)) {
    if (t >= 1.f) {
      ret = p2;
    } else {
      const float t2 = t * t;
      const float t3 = t2 * t;
      const float halfT3 = 0.5f * t3;
      ret = (-(halfT3) + t2 - 0.5f * t) * p0 + (1.5f * t3 + -2.5f * t2 + 1.f) * p1 +
            (-(1.5f * t3) + 2.f * t2 + 0.5f * t) * p2 + (halfT3 - 0.5f * t2) * p3;
    }
  }
  return ret;
}

rstl::optional_object< CAABox > CParticleSwoosh::GetBounds() const {
  if (GetParticleCount() <= 1) {
    return CAABox(mTranslation + mGlobalTranslation, mTranslation + mGlobalTranslation);
  }
  const CVector3f radius(mMaxRadius, mMaxRadius, mMaxRadius);
  return CAABox(mAabbMin - radius, mAabbMax + radius)
      .GetTransformedAABox(CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation *
                           mScaleXf);
}

void CParticleSwoosh::UpdateBounds(const CVector3f& pos) {
  mAabbMax = CVector3f(rstl::max_val(pos.GetX(), mAabbMax.GetX()),
                           rstl::max_val(pos.GetY(), mAabbMax.GetY()),
                           rstl::max_val(pos.GetZ(), mAabbMax.GetZ()));
  mAabbMin = CVector3f(rstl::min_val(pos.GetX(), mAabbMin.GetX()),
                           rstl::min_val(pos.GetY(), mAabbMin.GetY()),
                           rstl::min_val(pos.GetZ(), mAabbMin.GetZ()));
}

void CParticleSwoosh::UpdateMaxRadius(float radius) {
  mMaxRadius = rstl::max_val(mMaxRadius, radius);
}

CLight CParticleSwoosh::GetLight() const {
  return CLight::BuildLocalAmbient(CVector3f::Zero(), CColor::White());
}

uint CParticleSwoosh::Get4CharId() const { return 'SWHC'; }

void CParticleSwoosh::UpdateSwooshTranslation(const CVector3f& translation) {
  mSwooshes[mCurParticle].mTranslation = mInvScaleXf * translation;
}

void CParticleSwoosh::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
  mScaleXf = CTransform4f::Scale(scale);
  mInvScaleXf = CTransform4f::Scale(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
}

bool CParticleSwoosh::SystemHasLight() const { return false; }
void CParticleSwoosh::DestroyParticles() {}
bool CParticleSwoosh::GetParticleEmission() const { return mEmitting; }
const CTransform4f& CParticleSwoosh::GetOrientation() const { return mOrientation; }
const CVector3f& CParticleSwoosh::GetGlobalScale() const { return mGlobalScale; }
const CTransform4f& CParticleSwoosh::GetGlobalOrientation() const { return mGlobalOrientation; }
const CVector3f& CParticleSwoosh::GetGlobalTranslation() const { return mGlobalTranslation; }
const CVector3f& CParticleSwoosh::GetTranslation() const { return mTranslation; }
