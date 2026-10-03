#include "Kyoto/Particles/CElementGen.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/Particles/CWarp.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include "alloca.h"
#include "math.h"

#include "dolphin/gx.h"

#pragma inline_max_size(250)
#pragma inline_max_total_size(20000) // for RenderParticles vector inlining

#ifdef __MWERKS__
#define frsqrte(x) static_cast< float >(__frsqrte(x))
#else
extern "C" float frsqrte(float);
#endif

// NOTE: This is a hack, trying to figure out what causes the wack GX codegen in
// CElementGen::RenderParticles. The `const CVector3f&` binding seems to help.
#define GXPositionVector3f(in)                                                                     \
  {                                                                                                \
    const CVector3f& vec = (in);                                                                   \
    GXPosition3f32(vec[0], vec[1], vec[2]);                                                        \
  }

bool CElementGen::sMoveRedToAlphaBuffer;
bool CElementGen::sSubtractBlend;

int CElementGen::mParticleAliveCount;
int CElementGen::mParticleSystemAliveCount;

double CElementGen::kTickTime = 1 / 60.0;
ushort CElementGen::sSeed = 99;
static bool sStaticListInitialized;

struct CParticleListItemViewPointComp {
  bool operator()(const CElementGen::CParticleListItem& a,
                  const CElementGen::CParticleListItem& b) const {
    return a.mViewPoint.GetY() > b.mViewPoint.GetY() ? true : false;
  }
};

// name?

struct CTexturedParticleListItemViewPointComp {
  bool operator()(const CElementGen::CTexturedParticleListItem& a,
                  const CElementGen::CTexturedParticleListItem& b) const {
    return a.mViewPoint.GetY() > b.mViewPoint.GetY() ? true : false;
  }
};

static const GXVtxDescList sVtxDescList[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

CElementGen::CElementGen(TToken< CGenDescription > gen, EModelOrientationType orientType,
                         EOptionalSystemFlags optFlags)
: mGenDesc(gen)
, mLoadedGenDesc(*mGenDesc)
, mOrientType(orientType)
, mInternalStartFrame(0)
, mCurFrame(0)
, mCurSeconds(0.0)
, mPrevFrame(-1)
, mParticleEmission(true)
, mGeneratorRemainder(0.f)
, mMAXP(0)
, mRandomSeed(sSeed)
, mGeneratorRate(1.f)
, mTranslation(CVector3f::Zero())
, mGlobalTranslation(CVector3f::Zero())
, mPOFS(CVector3f::Zero())
, mGlobalScale(1.f, 1.f, 1.f)
, mGlobalScaleTransform(CTransform4f::Identity())
, mGlobalScaleTransformInverse(CTransform4f::Identity())
, mLocalScale(1.f, 1.f, 1.f)
, mLocalScaleTransform(CTransform4f::Identity())
, mLocalScaleTransformInverse(CTransform4f::Identity())
, mOrientation(CTransform4f::Identity())
, mOrientationInverse(CMatrix3f::Identity())
, mGlobalOrientation(CTransform4f::Identity())
, mActiveParticleCount(0)
, mCumulativeParticles(0)
, mRecursiveParticleCount(0)
, mTranslationDirty(false)
, mZTest(true)
, mORNT(false)
, mWarmedUp(false)
, mModelsUseLights(false)
, mEnableOPTS(optFlags & kOSF_Two)
, mEnableADV(false)
, mBackupLightActive(false)
, mRandState(mRandomSeed)
, mSSPO(CVector3f::Zero())
, mSEPO(CVector3f::Zero())
, x2cc(0.f)
, x2d0(0.f)
, mAabbMin(CVector3f::Zero())
, mAabbMax(CVector3f::Zero())
, mMaxSize(0.f)
, mSystemBounds(CAABox::MakeMaxInvertedBox())
, mLightType(kLT_None)
, mLCLR(CColor::White())
, mLINT(1.f)
, mLOFF(CVector3f::Zero())
, mLDIR(1.f, 0.f, 0.f)
, mFalloffType(kFT_Linear)
, mLFOR(1.f)
, mLSLA(45.f)
, mModuColor(0xFFFFFFFF) {
  CGlobalRandom gr(mRandState);

  if (CIntElement* seed = mLoadedGenDesc->mSEED) {
    int seedVal;
    seed->GetValue(mCurFrame, seedVal);
    mRandomSeed = seedVal;
  }
  mRandState.SetSeed(mRandomSeed);
  ++mParticleSystemAliveCount;

  mLIT_ = mLoadedGenDesc->mLIT_;
  mAAPH = mLoadedGenDesc->mAAPH;
  mZBUF = mLoadedGenDesc->mZBUF;
  mORNT = mLoadedGenDesc->mORNT;
  mMBLR = mORNT ? false : mLoadedGenDesc->mMBLR;

  if (CIntElement* mbsp = mLoadedGenDesc->mMBSP) {
    mbsp->GetValue(mCurFrame, mMBSP);
  }

  int idx = 0;
  if (CModVectorElement* vel1 = mLoadedGenDesc->mVEL1) {
    mVELSources[idx] = vel1;
    mHasVMD[idx] = mLoadedGenDesc->mVMD1;
    ++idx;
  }
  if (CModVectorElement* vel2 = mLoadedGenDesc->mVEL2) {
    mVELSources[idx] = vel2;
    mHasVMD[idx] = mLoadedGenDesc->mVMD2;
    ++idx;
  }
  if (CModVectorElement* vel3 = mLoadedGenDesc->mVEL3) {
    mVELSources[idx] = vel3;
    mHasVMD[idx] = mLoadedGenDesc->mVMD3;
    ++idx;
  }
  if (CModVectorElement* vel4 = mLoadedGenDesc->mVEL4) {
    mVELSources[idx] = vel4;
    mHasVMD[idx] = mLoadedGenDesc->mVMD4;
    ++idx;
  }
  for (int i = idx; i < 4; ++i) {
    mVELSources[i] = nullptr;
  }

  if (mLoadedGenDesc->mADV1 || mLoadedGenDesc->mADV2 ||
      mLoadedGenDesc->mADV3 || mLoadedGenDesc->mADV4 ||
      mLoadedGenDesc->mADV5 || mLoadedGenDesc->mADV6 ||
      mLoadedGenDesc->mADV7 || mLoadedGenDesc->mADV8) {
    mEnableADV = true;
  }

  for (int i = 0; i < 16; ++i) {
    mExternalVars[i] = 0.f;
  }

  mCSSD = 0;
  if (CIntElement* cssd = mLoadedGenDesc->mCSSD) {
    cssd->GetValue(0, mCSSD);
  }

  mPISY = 16;
  if (CIntElement* pisy = mLoadedGenDesc->mPISY) {
    pisy->GetValue(0, mPISY);
    if (mPISY <= 0) {
      mPISY = 1;
    }
  }

  mSISY = 16;
  if (CIntElement* sisy = mLoadedGenDesc->mSISY) {
    sisy->GetValue(0, mSISY);
  }

  mSSSD = 0;
  if (CIntElement* sssd = mLoadedGenDesc->mSSSD) {
    sssd->GetValue(0, mSSSD);
  }

  if (mLoadedGenDesc->mSSPO) {
    mLoadedGenDesc->mSSPO->GetValue(0, mSSPO);
    if (!mLoadedGenDesc->mSSPO->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }

  mSESD = 0;
  if (CIntElement* sesd = mLoadedGenDesc->mSESD) {
    sesd->GetValue(0, mSESD);
  }

  if (mLoadedGenDesc->mSEPO) {
    mLoadedGenDesc->mSEPO->GetValue(0, mSEPO);
    if (!mLoadedGenDesc->mSEPO->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }

  if (mLoadedGenDesc->mPOFS) {
    mLoadedGenDesc->mPOFS->GetValue(mCurFrame, mPOFS);
    if (!mLoadedGenDesc->mPOFS->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }

  if (CIntElement* pslt = mLoadedGenDesc->mPSLT) {
    pslt->GetValue(0, mPSLT);
  } else {
    mPSLT = 0x7FFFFF;
  }

  if (CIntElement* maxp = mLoadedGenDesc->mMAXP) {
    maxp->GetValue(mCurFrame, mMAXP);
  }

  int count = mMAXP;
  if (count > 256) {
    count = 256;
  }
  mParticles.reserve(count);

  if (mEnableADV) {
    mAdvValues.assign(count);
  }

  if (mOrientType == kMOT_One) {
    mParentMatrices.assign(mMAXP, CMatrix3f::Identity());
  }

  mLINE = mLoadedGenDesc->mLINE;
  mFXLL = mLoadedGenDesc->mFXLL;

  if (mLoadedGenDesc->mLTYP != nullptr) {
    int ltypVal = 0;
    mLoadedGenDesc->mLTYP->GetValue(mCurFrame, ltypVal);
    switch (LightType(ltypVal)) {
    case kLT_None:
      mLightType = kLT_None;
      break;
    case kLT_Custom:
      mLightType = kLT_Custom;
      break;
    case kLT_Directional:
      mLightType = kLT_Directional;
      break;
    case kLT_Spot:
      mLightType = kLT_Spot;
      break;
    default:
      mLightType = kLT_None;
      break;
    }

    if (mLoadedGenDesc->mLFOT != nullptr) {
      int lfotVal = 1;
      mLoadedGenDesc->mLFOT->GetValue(mCurFrame, lfotVal);
      switch (static_cast< EFalloffType >(lfotVal)) {
      case kFT_Constant:
        mFalloffType = kFT_Constant;
        break;
      case kFT_Linear:
        mFalloffType = kFT_Linear;
        break;
      case kFT_Quadratic:
        mFalloffType = kFT_Quadratic;
        break;
      default:
        mFalloffType = kFT_Linear;
        break;
      }
    }
  }
}

CElementGen::~CElementGen() {
  --mParticleSystemAliveCount;
  mParticleAliveCount -= mParticles.size();

  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    delete *it;
  }
}

void CElementGen::Initialize() {
  if (sStaticListInitialized)
    return;
  mParticleAliveCount = 0;
  mParticleSystemAliveCount = 0;
  sStaticListInitialized = true;
}

void CElementGen::ShutDown() { sStaticListInitialized = false; }

void CElementGen::SetTranslation(const CVector3f& translation) {
  mTranslation = translation;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* ch = *it;
    if (ch->Get4CharId() == 'ELSC') {
      ch->SetTranslation(translation + mSEPO);
    } else if (ch->Get4CharId() == 'SWHC') {
      ch->SetTranslation(translation + mSSPO);
    } else {
      ch->SetTranslation(translation);
    }
  }
}

void CElementGen::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetGlobalTranslation(translation);
  }
}

void CElementGen::SetModulationColor(const CColor& col) {
  mModuColor = col;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetModulationColor(col);
  }
}

const CColor& CElementGen::GetModulationColor() const { return mModuColor; }

void CElementGen::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
  mGlobalScaleTransform = CTransform4f::Scale(mGlobalScale.GetX(), mGlobalScale.GetY(),
                                                  mGlobalScale.GetZ());
  mGlobalScaleTransformInverse = CTransform4f::Scale(
      1.f / mGlobalScale.GetX(), 1.f / mGlobalScale.GetY(), 1.f / mGlobalScale.GetZ());
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetGlobalScale(scale);
  }
}

void CElementGen::SetLocalScale(const CVector3f& scale) {
  mLocalScale = scale;
  mLocalScaleTransform =
      CTransform4f::Scale(mLocalScale.GetX(), mLocalScale.GetY(), mLocalScale.GetZ());
  mLocalScaleTransformInverse = CTransform4f::Scale(
      1.f / mLocalScale.GetX(), 1.f / mLocalScale.GetY(), 1.f / mLocalScale.GetZ());
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetLocalScale(scale);
  }
}

void CElementGen::SetOrientation(const CTransform4f& orientation) {
  mOrientation = orientation;
  mOrientationInverse = mOrientation.GetQuickInverse().BuildMatrix3f();
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetOrientation(orientation);
  }
}

void CElementGen::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation.SetRotation(orientation);
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetGlobalOrientation(mGlobalOrientation);
  }
}

void CElementGen::SetGlobalOrientAndTrans(const CTransform4f& xf) {
  SetGlobalOrientation(xf);
  SetGlobalTranslation(xf.GetTranslation());
}

const bool CElementGen::Update(double dt) {
  CParticleGlobals::SParticleSystem thisSystem('PART', this);

  if (mLoadedGenDesc->mPSWT && !mWarmedUp) {
    int pswt = 0;
    mLoadedGenDesc->mPSWT->GetValue(mCurFrame, pswt);
    InternalUpdate(kTickTime * pswt);
    mWarmedUp = true;
  }

  return InternalUpdate(dt);
}

bool CElementGen::InternalUpdate(double dt) {
  CStopwatch sw;
  CGlobalRandom gr(mRandState);

  int frameUpdateCount = 0;
  double t = mCurFrame * kTickTime;
  double dt1 = close_enough(dt, kTickTime) ? kTickTime : dt;

  CParticleGlobals::SetEmitterTime(mCurFrame);

  if (mLoadedGenDesc->mPSTS) {
    float psts = 1.0f;
    mLoadedGenDesc->mPSTS->GetValue(mCurFrame, psts);
    dt1 *= psts;
    dt1 = rstl::max_val(0.0, dt1);
  }

  mCurSeconds += dt1;

  if (mMBLR && dt > 0.0) {
    if (mLoadedGenDesc->mMBSP) {
      mLoadedGenDesc->mMBSP->GetValue(mCurFrame, mMBSP);
    }
  }

  while (t < mCurSeconds && !close_enough(t, mCurSeconds)) {
    mAabbMin = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    mAabbMax = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    mMaxSize = 0.0f;

    CParticleGlobals::SetEmitterTime(mCurFrame);
    UpdateExistingParticles();
    CParticleGlobals::SetParticleLifetime(mPSLT);

    if (mCurFrame < mPSLT && mParticleEmission) {
      float grte = 0.0f;
      if (mLoadedGenDesc->mGRTE->GetValue(mCurFrame, grte)) {
        mParticles.clear();
        return true;
      }

      grte = rstl::max_val(0.0f, grte * mGeneratorRate);
      mGeneratorRemainder += grte;
      int genCount = static_cast< int >(floor(mGeneratorRemainder));
      mGeneratorRemainder -= static_cast< float >(genCount);

      if (mLoadedGenDesc->mMAXP) {
        mLoadedGenDesc->mMAXP->GetValue(mCurFrame, mMAXP);
      }

      CreateNewParticles(genCount);
    }

    if (mTranslationDirty) {
      UpdatePSTranslationAndOrientation();
    }

    if (mLightType != kLT_None) {
      UpdateLightParameters();
    }

    UpdateChildParticleSystems(kTickTime);

    ++frameUpdateCount;
    t += kTickTime;
    ++mCurFrame;
  }

  if (close_enough(t, mCurSeconds)) {
    mCurSeconds = t;
    mTimeDeltaScale = 1.0f;
  } else {
    UpdateChildParticleSystems(dt1 - (double)frameUpdateCount * kTickTime);
    mTimeDeltaScale = 1.0f - static_cast< float >((t - mCurSeconds) / kTickTime);
  }

  BuildParticleSystemBounds();

  x2cc = sw.GetElapsedTime();
  return false;
}

void CElementGen::UpdateLightParameters() {
  mLoadedGenDesc->mLCLR->GetValue(mCurFrame, mLCLR);
  mLoadedGenDesc->mLINT->GetValue(mCurFrame, mLINT);
  if (mLightType != kLT_Directional) {
    mLoadedGenDesc->mLOFF->GetValue(mCurFrame, mLOFF);
    mLoadedGenDesc->mLFOR->GetValue(mCurFrame, mLFOR);
    if (mLightType == kLT_Spot) {
      mLoadedGenDesc->mLSLA->GetValue(mCurFrame, mLSLA);
    }
  }
  if (mLightType != kLT_Custom) {
    mLoadedGenDesc->mLDIR->GetValue(mCurFrame, mLDIR);
  }
}

void CElementGen::UpdateAdvanceAccessParameters(int particleFrame, int particleIndex) {
  float* params = mAdvValues[particleFrame].values;
  CParticleGlobals::mParticleAccessParameters = params;
  if (mLoadedGenDesc->mADV1) {
    mLoadedGenDesc->mADV1->GetValue(particleIndex, params[0]);
  }
  if (mLoadedGenDesc->mADV2) {
    mLoadedGenDesc->mADV2->GetValue(particleIndex, params[1]);
  }
  if (mLoadedGenDesc->mADV3) {
    mLoadedGenDesc->mADV3->GetValue(particleIndex, params[2]);
  }
  if (mLoadedGenDesc->mADV4) {
    mLoadedGenDesc->mADV4->GetValue(particleIndex, params[3]);
  }
  if (mLoadedGenDesc->mADV5) {
    mLoadedGenDesc->mADV5->GetValue(particleIndex, params[4]);
  }
  if (mLoadedGenDesc->mADV6) {
    mLoadedGenDesc->mADV6->GetValue(particleIndex, params[5]);
  }
  if (mLoadedGenDesc->mADV7) {
    mLoadedGenDesc->mADV7->GetValue(particleIndex, params[6]);
  }
  if (mLoadedGenDesc->mADV8) {
    mLoadedGenDesc->mADV8->GetValue(particleIndex, params[7]);
  }
}

bool CElementGen::UpdateVelocitySource(int velIdx, int frame, CParticle& particle) {
  bool reset;
  if (mHasVMD[velIdx]) {
    CVector3f localVel = mOrientationInverse * particle.mVel;
    CVector3f localPos = mOrientationInverse * (particle.mPos - mTranslation);
    reset = mVELSources[velIdx]->GetValue(frame, localVel, localPos);
    particle.mVel = mOrientation.Rotate(localVel);
    particle.mPos = mOrientation.Rotate(localPos) + mTranslation;
  } else {
    reset = mVELSources[velIdx]->GetValue(frame, particle.mVel, particle.mPos);
  }
  if (reset) {
    particle.mEndFrame = -1;
    return true;
  }
  return false;
}

void CElementGen::UpdateExistingParticles() {
  AUTO(p, mParticles.begin());
  mActiveParticleCount = 0;
  CParticleGlobals::SetEmitterTime(mCurFrame);
  CParticleGlobals::mParticleAccessParameters = NULL;

  while (p != mParticles.end()) {
    if (p->mEndFrame < mCurFrame) {
      --mParticleAliveCount;
      if (p + 1 == mParticles.end()) {
        mParticles.mCount = mParticles.size() - 1;
        break;
      }
      *p = mParticles.back();
      if (mOrientType == kMOT_One) {
        mParentMatrices[mActiveParticleCount] = mParentMatrices[mParticles.size() - 1];
      }
      if (mEnableADV) {
        mAdvValues[mActiveParticleCount] = mAdvValues[mParticles.size() - 1];
      }
      --mParticles.mCount;
      if (p != mParticles.end()) {
        if (p->mEndFrame < mCurFrame) {
          continue;
        }
      }
    }
    p->mPrevPos = p->mPos;
    p->mPos += p->mVel;
    int particleFrame = mCurFrame - p->mStartFrame;
    CParticleGlobals::mCurrentParticle = &*p;
    CParticleGlobals::SetParticleLifetime(p->mEndFrame - p->mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(particleFrame);
    if (mEnableADV) {
      UpdateAdvanceAccessParameters(mActiveParticleCount, particleFrame);
    }
    ++mActiveParticleCount;
    if (mVELSources[0] != NULL) {
      UpdateVelocitySource(0, particleFrame, *p);
      if (mVELSources[1] != NULL) {
        UpdateVelocitySource(1, particleFrame, *p);
        if (mVELSources[2] != NULL) {
          UpdateVelocitySource(2, particleFrame, *p);
          if (mVELSources[3] != NULL) {
            UpdateVelocitySource(3, particleFrame, *p);
          }
        }
      }
    }
    if (mLINE) {
      if (CRealElement* leng = mLoadedGenDesc->mLENG) {
        leng->GetValue(particleFrame, p->mLineLengthOrSize);
      }
      if (CRealElement* widt = mLoadedGenDesc->mWIDT) {
        widt->GetValue(particleFrame, p->mLineWidthOrRota);
      }
    } else {
      if (CRealElement* rota = mLoadedGenDesc->mROTA) {
        rota->GetValue(particleFrame, p->mLineWidthOrRota);
      }
      if (CRealElement* size = mLoadedGenDesc->mSIZE) {
        size->GetValue(particleFrame, p->mLineLengthOrSize);
      }
    }
    if (CColorElement* colr = mLoadedGenDesc->mCOLR) {
      colr->GetValue(particleFrame, p->mColor);
    }
    AccumulateBounds(p->mPos, p->mLineLengthOrSize);
    ++p;
  }

  if (mParticles.size() > 0) {
    rstl::list< CWarp* >::iterator it = mModifiersList.begin();
    rstl::list< CWarp* >::iterator endIt = mModifiersList.end();
    while (it != endIt) {
      if ((*it)->UpdateWarp()) {
        CParticle* base = mParticles.data();
        (*it)->ModifyParticles(mParticles.size(), sizeof(CParticle), &base->mEndFrame,
                               &base->mPrevPos, &base->mPos, &base->mVel, &base->mColor,
                               &base->mLineLengthOrSize, &base->mLineWidthOrRota);
      }
      ++it;
    }
  }
}

void CElementGen::CreateNewParticles(int count) {
  if (!sStaticListInitialized) {
    Initialize();
  }

  int newCount = count;
  if (mParticles.size() >= mMAXP) {
    return;
  }

  if (newCount + mParticles.size() > mMAXP) {
    newCount = mMAXP - mParticles.size();
  }

  if (newCount + mParticleAliveCount > 0xa00) {
    newCount = 0xa00 - mParticleAliveCount;
  }

  CGlobalRandom gr(mRandState);
  mParticles.reserve(newCount + mParticles.size());

  if (mEnableADV && mAdvValues.capacity() < newCount + mParticles.size()) {
    int newCap = mAdvValues.capacity() * 2;
    mAdvValues.reserve(rstl::min_val(mMAXP, newCap));
    while (mAdvValues.size() < mAdvValues.capacity()) {
      mAdvValues.push_back(CAdvancedValues());
    }
  }

  CParticleGlobals::mParticleAccessParameters = NULL;

  for (int i = 0; i < newCount; ++i) {
    // TODO: The x1c_vel values somehow get frsp'd, despite being
    // floats from CVector3f::Zero and being stored as floats in CParticle.
    mParticles.push_back(CParticle());

    int particleIndex = mParticles.size() - 1;
    ++mParticleAliveCount;
    ++mActiveParticleCount;
    ++mCumulativeParticles;

    if (mOrientType == kMOT_One) {
      mParentMatrices[particleIndex] = mOrientation.BuildMatrix3f();
    }

    CParticle& particle = mParticles[particleIndex];
    particle.mStartFrame = mCurFrame;

    if (CIntElement* ltme = mLoadedGenDesc->mLTME) {
      ltme->GetValue(0, particle.mEndFrame);
    }

    CParticleGlobals::SetParticleLifetime(particle.mEndFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
    CParticleGlobals::mCurrentParticle = &particle;

    if (mEnableADV) {
      UpdateAdvanceAccessParameters(particleIndex, 0);
    }

    particle.mEndFrame += mCurFrame;

    if (CColorElement* colr = mLoadedGenDesc->mCOLR) {
      colr->GetValue(0, particle.mColor);
    } else {
      particle.mColor = CColor(0xFFFFFFFF);
    }

    if (CEmitterElement* emtr = mLoadedGenDesc->mEMTR) {
      emtr->GetValue(mCurFrame, particle.mPos, particle.mVel);
      particle.mPos =
          mOrientation.Rotate(particle.mPos) +
          ((mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation) +
          mPOFS;
      particle.mVel = mOrientation.Rotate(particle.mVel);
    } else {
      const CVector3f& compPos =
          (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation;
      particle.mPos = compPos + mPOFS;
      particle.mVel = CVector3f::Zero();
    }

    particle.mPrevPos = particle.mPos;

    if (mLINE) {
      if (CRealElement* leng = mLoadedGenDesc->mLENG) {
        leng->GetValue(0, particle.mLineLengthOrSize);
      } else {
        particle.mLineLengthOrSize = 1.f;
      }
      if (CRealElement* widt = mLoadedGenDesc->mWIDT) {
        widt->GetValue(0, particle.mLineWidthOrRota);
      } else {
        particle.mLineWidthOrRota = 1.f;
      }
    } else {
      if (CRealElement* rota = mLoadedGenDesc->mROTA) {
        rota->GetValue(0, particle.mLineWidthOrRota);
      } else {
        particle.mLineWidthOrRota = 0.f;
      }
      if (CRealElement* size = mLoadedGenDesc->mSIZE) {
        size->GetValue(0, particle.mLineLengthOrSize);
      } else {
        particle.mLineLengthOrSize = 0.1f;
      }
    }

    AccumulateBounds(particle.mPos, particle.mLineLengthOrSize);
  }
}

void CElementGen::UpdatePSTranslationAndOrientation() {
  CGlobalRandom gr(mRandState);
  if (mPSLT >= mCurFrame) {
    if (CVectorElement* pofs = mLoadedGenDesc->mPOFS) {
      pofs->GetValue(mCurFrame, mPOFS);
    }
    if (CVectorElement* sspo = mLoadedGenDesc->mSSPO) {
      sspo->GetValue(mCurFrame, mSSPO);
    }
    if (CVectorElement* sepo = mLoadedGenDesc->mSEPO) {
      sepo->GetValue(mCurFrame, mSEPO);
    }
  }
}

CElementGen* CElementGen::ConstructChildParticleSystem(TToken< CGenDescription > desc) const {
  CElementGen* child =
      rs_new CElementGen(desc, kMOT_Normal, mEnableOPTS ? kOSF_Two : kOSF_One);
  child->mModelsUseLights = mModelsUseLights;
  child->SetGlobalTranslation(mGlobalTranslation);
  child->SetGlobalOrientation(mGlobalOrientation);
  child->SetGlobalScale(mGlobalScale);
  child->SetLocalScale(mLocalScale);
  child->SetTranslation(mTranslation);
  child->SetOrientation(mOrientation);
  child->SetParticleEmission(mParticleEmission);
  child->SetModulationColor(mModuColor);
  return child;
}

void CElementGen::UpdateChildParticleSystems(double dt) {
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }

  CGlobalRandom gr(mRandState);

  // ICTS - child particle systems spawned at CSSD frame
  if (mLoadedGenDesc->mICTS && mPrevFrame != mCurFrame && mCurFrame == mCSSD) {
    int ncsyVal = 1;
    if (CIntElement* ncsy = mLoadedGenDesc->mNCSY) {
      ncsy->GetValue(mCurFrame, ncsyVal);
    }
    mActivePartChildren.reserve(ncsyVal + mActivePartChildren.size());
    for (int i = 0; i < ncsyVal; ++i) {
      TLockedToken< CGenDescription > icsToken = mLoadedGenDesc->mICTS->GetToken();
      bool descOPTS = icsToken->mOPTS;
      if (mEnableOPTS && descOPTS) {
        break;
      }
      mActivePartChildren.push_back(ConstructChildParticleSystem(icsToken));
    }
  }

  // IITS - child particle systems spawned periodically
  if (mLoadedGenDesc->mIITS && mPrevFrame != mCurFrame && mCurFrame < mPSLT &&
      mParticleEmission == true && mCurFrame >= mSISY &&
      ((mCurFrame - mSISY) % mPISY) == 0) {
    TLockedToken< CGenDescription > iitsToken = mLoadedGenDesc->mIITS->GetToken();
    bool iitsOPTS = iitsToken->mOPTS;
    if (!(mEnableOPTS && iitsOPTS)) {
      mActivePartChildren.reserve(mActivePartChildren.size() + 1);
      mActivePartChildren.push_back(ConstructChildParticleSystem(iitsToken));
    }
  }

  // KSSM - spawn system keyframe data
  if (mLoadedGenDesc->mKSSM.get() != nullptr && mPrevFrame != mCurFrame &&
      mCurFrame < mPSLT) {
    const ushort backupSeed = sSeed;
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        mLoadedGenDesc->mKSSM->GetSpawnedSystemsAtFrame(mCurFrame);
    mActivePartChildren.reserve(spawns.size() + mActivePartChildren.size());
    for (int i = 0; i < spawns.size(); ++i) {
      TLockedToken< CGenDescription > kssmToken = *spawns[i].GetToken();
      bool kssmOPTS = kssmToken->mOPTS;
      if (mEnableOPTS && kssmOPTS) {
        continue;
      }
      sSeed = backupSeed + i;
      mActivePartChildren.push_back(ConstructChildParticleSystem(kssmToken));
    }
    sSeed = backupSeed;
  }

  // IDTS - child particle systems spawned at death
  if (mCurFrame == mPSLT && mPrevFrame != mCurFrame && mLoadedGenDesc->mIDTS) {
    int ndsyVal = 1;
    if (CIntElement* ndsy = mLoadedGenDesc->mNDSY) {
      ndsy->GetValue(0, ndsyVal);
    }
    mActivePartChildren.reserve(ndsyVal + mActivePartChildren.size());
    for (int i = 0; i < ndsyVal; ++i) {
      TLockedToken< CGenDescription > idtsToken = mLoadedGenDesc->mIDTS->GetToken();
      bool idtsOPTS = idtsToken->mOPTS;
      if (mEnableOPTS && idtsOPTS) {
        break;
      }
      mActivePartChildren.push_back(ConstructChildParticleSystem(idtsToken));
    }
  }

  // SSWH - swoosh child particle system
  if (mLoadedGenDesc->mSSWH && mPrevFrame != mCurFrame && mCurFrame == mSSSD) {
    CParticleSwoosh* swoosh = rs_new CParticleSwoosh(*mLoadedGenDesc->mSSWH, 0);
    swoosh->SetGlobalTranslation(mGlobalTranslation);
    swoosh->SetGlobalScale(mGlobalScale);
    swoosh->SetLocalScale(mLocalScale);
    swoosh->SetTranslation(mTranslation + mSSPO);
    swoosh->SetOrientation(mOrientation);
    const bool emission = mParticleEmission;
    swoosh->SetParticleEmission(emission);
    mActivePartChildren.reserve(mActivePartChildren.size() + 1);
    mActivePartChildren.push_back(swoosh);
  }

  // SELC - electric child particle system
  if (mLoadedGenDesc->mSELC && mPrevFrame != mCurFrame && mCurFrame == mSESD) {
    CParticleElectric* electric = rs_new CParticleElectric(*mLoadedGenDesc->mSELC);
    electric->SetGlobalTranslation(mGlobalTranslation);
    electric->SetGlobalScale(mGlobalScale);
    electric->SetLocalScale(mLocalScale);
    electric->SetTranslation(mTranslation + mSEPO);
    electric->SetOrientation(mOrientation);
    electric->SetParticleEmission(mParticleEmission);
    mActivePartChildren.reserve(mActivePartChildren.size() + 1);
    mActivePartChildren.push_back(electric);
  }

  // Update all child particle systems
  AUTO(it, mActivePartChildren.begin());
  while (it != mActivePartChildren.end()) {
    CParticleGen* child = *it;
    child->Update(dt);
    if (child->IsSystemDeletable() == true) {
      delete child;
      it = mActivePartChildren.erase(it);
    } else {
      ++it;
    }
  }

  mPrevFrame = mCurFrame;
}

void CElementGen::SetParticleEmission(bool emission) {
  mParticleEmission = emission;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetParticleEmission(emission);
  }
}

void CElementGen::ForceParticleCreation(int amount) {
  CParticleGlobals::SParticleSystem thisSystem('PART', this);
  CParticleGlobals::SetEmitterTime(mCurFrame);
  CreateNewParticles(amount);
}

void CElementGen::EndLifetime() {
  mPSLT = 0;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      static_cast< CElementGen* >(child)->EndLifetime();
    } else {
      child->SetParticleEmission(false);
    }
  }
}

void CElementGen::DestroyParticles() {
  mParticleAliveCount -= mParticles.size();
  mParticles.clear();
  if (mOrientType == kMOT_One) {
    mParentMatrices.clear();
  }
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->DestroyParticles();
  }
}

bool CElementGen::IsSystemDeletable() const {
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    if (!(*it)->IsSystemDeletable()) {
      return false;
    }
  }
  if (mPSLT < mCurFrame && static_cast< int >(mActiveParticleCount) == 0) {
    return true;
  }
  return false;
}

void CElementGen::Render() {
  CStopwatch timer;

  mBackupLightActive = CGraphics::GetLightMask();
  CGraphics::DisableAllLights();

  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->Render();
  }

  CParticleGlobals::SParticleSystem sys('PART', this);

  if (mParticles.size() > 0) {
    if (mLoadedGenDesc->mPMDL || mLoadedGenDesc->mPMUS) {
      RenderModels();
    }
    if (mLINE) {
      RenderLines();
    } else {
      RenderParticles();
    }
  }

  x2d0 = timer.GetElapsedTime();
}

void CElementGen::RenderBasicParticlesNoRotTS(const CTransform4f& xf) {
  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    CVector3f viewPos = xf * particle.mPos;
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    uint color = particle.mColor.GetColor_u32();
    float x = viewPos.GetX() + halfSize;
    float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotTS(const CTransform4f& xf) {
  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    CVector3f viewPos = xf * particle.mPos;
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.mColor.GetColor_u32();
    float sinT = CMath::FastSinR(theta) * halfSize;
    float cosT = CMath::FastCosR(theta) * halfSize;
    float sinPlusCos = sinT + cosT;
    float sinMinusCos = sinT - cosT;

    float x = viewPos.GetX();
    float y = viewPos.GetY();
    float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesNoRotNoTS(const CTransform4f& xf) {
  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    uint color = particle.mColor.GetColor_u32();
    float x = viewPos.GetX() + halfSize;
    float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotNoTS(const CTransform4f& xf) {
  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.mColor.GetColor_u32();
    float sinT = CMath::FastSinR(theta) * halfSize;
    float cosT = CMath::FastCosR(theta) * halfSize;
    float sinPlusCos = sinT + cosT;
    float sinMinusCos = sinT - cosT;
    float x = viewPos.GetX();
    float y = viewPos.GetY();
    float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderParticles() {
  CGlobalRandom gr(mRandState);

  if (IsIndirectTextured()) {
    RenderParticlesIndirectTexture();
    return;
  }

  if (mLoadedGenDesc->mSIZE != nullptr && mLoadedGenDesc->mSIZE->IsConstant()) {
    float sizeVal = 1.f;
    mLoadedGenDesc->mSIZE->GetValue(0, sizeVal);
    if (sizeVal == 0.f) {
      sizeVal = 1.f;
      mLoadedGenDesc->mSIZE->GetValue(1, sizeVal);
      if (sizeVal == 0.f) {
        return;
      }
    }
  }

  CGraphics::SetCullMode(kCM_None);

  bool hasModuColor = mModuColor.GetColor_u32() != 0xFFFFFFFF;

  CTransform4f systemViewCopy(CGraphics::GetViewMatrix());
  CTransform4f systemModelMatrix(systemViewCopy);
  systemModelMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemModelMatrix.GetQuickInverse() * mGlobalOrientation);
  systemModelMatrix = CTransform4f::Translate(mGlobalTranslation) * mGlobalScaleTransform *
                      systemModelMatrix * mLocalScaleTransform;

  if (mORNT) {
    CGraphics::SetModelMatrix(systemModelMatrix * systemCameraCopy);
  } else {
    CGraphics::SetModelMatrix(systemModelMatrix);
  }

  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);

  bool constUVs = true;
  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  bool noRota = mLoadedGenDesc->mROTA == nullptr;
  if (mLoadedGenDesc->mROTA != nullptr && mLoadedGenDesc->mROTA->IsConstant()) {
    float value = 1.f;
    mLoadedGenDesc->mROTA->GetValue(0, value);
    if (0.f == value) {
      value = 1.f;
      mLoadedGenDesc->mROTA->GetValue(1, value);
      if (0.f == value) {
        noRota = true;
      }
    }
  }

  if (mLoadedGenDesc->mTEXR != nullptr) {
    int partFrame = mCurFrame - mParticles[0].mStartFrame;
    TToken< CTexture > tex = mLoadedGenDesc->mTEXR->GetValueTexture(partFrame);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }

    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
    constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  CGX::SetVtxDescv(sVtxDescList);

  GXTevStageID nextStage;
  if (hasModuColor) {
    CGX::SetNumChans(2);
    nextStage = GX_TEVSTAGE2;
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanAmbColor(CGX::Channel1, CColor(0.f, 0.f, 0.f, 1.f).GetGXColor());
    CGX::SetChanMatColor(CGX::Channel1, mModuColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
    nextStage = GX_TEVSTAGE1;
  }

  bool moveRedToAlphaBuffer = sMoveRedToAlphaBuffer;
  if (sSubtractBlend) {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, false);
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    if (moveRedToAlphaBuffer) {
      CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(nextStage);
      CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
      nextStage = static_cast< GXTevStageID >(nextStage + 1);
    }
  } else if (moveRedToAlphaBuffer) {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(nextStage);
    CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
  } else if (mAAPH) {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, mZBUF);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  CGX::SetNumTevStages(static_cast< uchar >(nextStage));
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  if (constUVs) {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  } else {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  }

  int particleCount = mParticles.size();
  int mbspVal = rstl::max_val(1, mMBSP);

  if (!mMBLR) {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, static_cast< ushort >(particleCount * 4));
  } else {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, static_cast< ushort >(mbspVal * (particleCount * 4)));
  }

  bool noTS = !close_enough(mTimeDeltaScale, 1.f, 0.00001f);
  bool SORT = mLoadedGenDesc->mSORT;
  CParticleListItem* sortItems = nullptr;

  if (SORT) {
    sortItems =
        static_cast< CParticleListItem* >(alloca(particleCount * sizeof(CParticleListItem)));
    for (int i = 0; i < particleCount; ++i) {
      CParticle& particle = mParticles[i];
      const CVector3f& delta = particle.mPos - particle.mPrevPos;
      const CVector3f& pos = delta * mTimeDeltaScale + particle.mPrevPos;
      // TODO: I think this is the constructor, but need the copy to be elided
      sortItems[i].mViewPoint = systemCameraCopy * pos;
      sortItems[i].mPartIdx = static_cast< ushort >(i);
    }
    rstl::sort(sortItems, sortItems + particleCount, CParticleListItemViewPointComp());
  }

  CParticleGlobals::SetEmitterTime(mCurFrame);

  if (!mMBLR) {
    if (!SORT && constUVs && !mORNT) {
      if (noRota) {
        if (noTS) {
          RenderBasicParticlesNoRotNoTS(systemCameraCopy);
        } else {
          RenderBasicParticlesNoRotTS(systemCameraCopy);
        }
      } else {
        if (noTS) {
          RenderBasicParticlesRotNoTS(systemCameraCopy);
        } else {
          RenderBasicParticlesRotTS(systemCameraCopy);
        }
      }
    } else if (!mORNT) {
      CParticleListItem* sortIt = sortItems;
      for (int i = 0; i < particleCount; ++i) {
        CParticle& particle = SORT ? mParticles[sortIt->mPartIdx] : mParticles[i];
        CVector3f viewPoint =
            systemCameraCopy *
            ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);

        uint color = particle.mColor.GetColor_u32();

        if (!constUVs) {
          int partFrame = mCurFrame - particle.mStartFrame - 1;
          CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
          CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
          mLoadedGenDesc->mTEXR->GetValueUV(partFrame, uvs);

          if (noRota) {
            float x = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetX();
            float y = viewPoint.GetY();
            float z = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetZ();

            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            x -= particle.mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            z -= particle.mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            x += particle.mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          } else {
            float halfSize = 0.5f * particle.mLineLengthOrSize;
            float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
            float sinT = CMath::FastSinR(theta) * halfSize;
            float cosT = CMath::FastCosR(theta) * halfSize;

            float sinPlusCos = sinT + cosT;
            float cosMinusSin = cosT - sinT;
            float sinMinusCos = sinT - cosT;
            float negSinPlusCos = -sinT + cosT;

            float vpX = viewPoint.GetX();
            float vpY = viewPoint.GetY();
            float vpZ = viewPoint.GetZ();

            GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            GXPosition3f32(negSinPlusCos + vpX, vpY, (-cosT - sinT) + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        } else if (noRota) {
          float x = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetX();
          float y = viewPoint.GetY();
          float z = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetZ();

          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          x -= particle.mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          z -= particle.mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          x += particle.mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        } else {
          float halfSize = 0.5f * particle.mLineLengthOrSize;
          float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
          float sinT = CMath::FastSinR(theta) * halfSize;
          float cosT = CMath::FastCosR(theta) * halfSize;

          float sinPlusCos = sinT + cosT;
          float sinMinusCos = sinT - cosT;

          float vpX = viewPoint.GetX();
          float vpY = viewPoint.GetY();
          float vpZ = viewPoint.GetZ();

          GXPosition3f32(vpX + sinPlusCos, vpY, vpZ - sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          GXPosition3f32(vpX + sinMinusCos, vpY, vpZ + sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          GXPosition3f32(vpX - sinPlusCos, vpY, vpZ + sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          GXPosition3f32(vpX - sinMinusCos, vpY, vpZ - sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }

#if NONMATCHING
        if (SORT) {
          ++sortIt;
        }
#else
        sortIt++;
#endif
      }
    } else {
      // ORNT
      CVector3f fwd = systemViewCopy.GetForward();
      CVector3f origin = systemViewCopy.GetTranslation();
      float canBeNormEps = FLT_EPSILON;

      CParticleListItem* sortIt = sortItems;
      for (int i = 0; i < particleCount; ++i) {
        CParticle* particle = SORT ? &mParticles[sortIt->mPartIdx] : &mParticles[i];

        CVector3f delta = particle->mPos - particle->mPrevPos;
        const CVector3f& pos = delta * mTimeDeltaScale + particle->mPrevPos;

        uint color = particle->mColor.GetColor_u32();
        float rota = mLoadedGenDesc->mROTA == nullptr ? 1.f : particle->mLineWidthOrRota;
        float velMagSq = particle->mVel.MagSquared();

        CVector3f dir;
        if (velMagSq > canBeNormEps) {
          dir = particle->mVel * frsqrte(velMagSq);
        } else {
          const CVector3f& dv = particle->mPos - particle->mPrevPos;
          float dMagSq = dv.MagSquared();
          if (dMagSq > canBeNormEps) {
            dir = dv * frsqrte(dMagSq);
          } else {
            dir = CVector3f(0.f, 0.f, 1.f);
          }
        }

        CVector3f fore = dir * particle->mLineLengthOrSize;

        CVector3f right;
        if (mLoadedGenDesc->mRSOP) {
          right = CVector3f::Cross(dir, fwd);
          float crMagSq = right.MagSquared();

          if (crMagSq > canBeNormEps) {
            float invMag = frsqrte(crMagSq);
            right *= particle->mLineLengthOrSize * rota * invMag;
          } else {
            CVector3f camDelta = origin - particle->mPos;
            right = CVector3f::Cross(dir, camDelta.AsNormalized());
            float cr2MagSq = right.MagSquared();

            if (cr2MagSq > canBeNormEps) {
              float invMag = frsqrte(cr2MagSq);
              right *= particle->mLineLengthOrSize * rota * invMag;
            }
          }
        } else {
          right = CVector3f::Cross(fore, fwd) * rota;
        }

        if (!constUVs) {
          int partFrame = mCurFrame - particle->mStartFrame - 1;
          CParticleGlobals::SetParticleLifetime(particle->mEndFrame - particle->mStartFrame);
          CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
          mLoadedGenDesc->mTEXR->GetValueUV(partFrame, uvs);

          CVector3f p = pos + right * 0.5f + fore * 0.5f;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);

          p -= fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);

          p -= right;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);

          p += fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
        } else {
          CVector3f p = pos + right * 0.5f + fore * 0.5f;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          p -= fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          p -= right;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          p += fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }

#if NONMATCHING
        if (SORT) {
          ++sortIt;
        }
#else
        sortIt++;
#endif
      }
    }
  } else {
    // MBLR
    float mbspFac = 1.f / static_cast< float >(mbspVal);
    for (int i = 0; i < particleCount; ++i) {
      CParticle* particle = &mParticles[i];

      CVector3f delta = particle->mPos - particle->mPrevPos;
      CVector3f vec = delta * mTimeDeltaScale + particle->mPrevPos;
      CVector3f mbspVec = delta * mbspFac;

      CColor col = particle->mColor;
      col.SetAlpha(col.GetAlpha() * mbspFac);
      uint color = col.GetColor_u32();

      if (!constUVs) {
        int partFrame = mCurFrame - particle->mStartFrame - 1;
        CParticleGlobals::SetParticleLifetime(particle->mEndFrame - particle->mStartFrame);
        CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
        mLoadedGenDesc->mTEXR->GetValueUV(partFrame, uvs);

        if (noRota) {
          for (int j = 0; j < mbspVal; ++j) {
            vec += mbspVec;
            CVector3f viewPoint = systemCameraCopy * vec;
            float x = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetX();
            float y = viewPoint.GetY();
            float z = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetZ();

            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            x -= particle->mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            z -= particle->mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            x += particle->mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        } else {
          for (int j = 0; j < mbspVal; ++j) {
            vec += mbspVec;
            CVector3f viewPoint = systemCameraCopy * vec;
            float halfSize = 0.5f * particle->mLineLengthOrSize;
            float theta = particle->mLineWidthOrRota * (M_PIF / 180.f);

            float sinT = CMath::FastSinR(theta) * halfSize;
            float cosT = CMath::FastCosR(theta) * halfSize;

            float sinPlusCos = sinT + cosT;
            float negSinPlusCos = -sinT + cosT;
            float sinMinusCos = sinT - cosT;
            float cosMinusSin = cosT - sinT;

            float vpX = viewPoint.GetX();
            float vpY = viewPoint.GetY();
            float vpZ = viewPoint.GetZ();

            GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            GXPosition3f32(negSinPlusCos + vpX, vpY, (-cosT - sinT) + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        }
      } else if (noRota) {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          CVector3f viewPoint = systemCameraCopy * vec;
          float x = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetX();
          float y = viewPoint.GetY();
          float z = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetZ();

          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          x -= particle->mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          z -= particle->mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          x += particle->mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      } else {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          CVector3f viewPoint = systemCameraCopy * vec;
          float halfSize = 0.5f * particle->mLineLengthOrSize;
          float theta = particle->mLineWidthOrRota * (M_PIF / 180.f);

          float sinT = CMath::FastSinR(theta) * halfSize;
          float cosT = CMath::FastCosR(theta) * halfSize;
          float sinPlusCos = sinT + cosT;
          float sinMinusCos = sinT - cosT;

          float vpX = viewPoint.GetX();
          float vpY = viewPoint.GetY();
          float vpZ = viewPoint.GetZ();

          GXPosition3f32(vpX + sinPlusCos, vpY, vpZ - sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          GXPosition3f32(vpX + sinMinusCos, vpY, vpZ + sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          GXPosition3f32(vpX - sinPlusCos, vpY, vpZ + sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          GXPosition3f32(vpX - sinMinusCos, vpY, vpZ - sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      }
    }
  }

  CGX::End();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(static_cast< GXTevStageID >(nextStage - 1), GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

void CElementGen::RenderParticlesFlameThrower(CElementGen* const* gens, int count) {
  CTransform4f viewCopy(CGraphics::GetViewMatrix());
  CTransform4f systemModelMatrix(viewCopy);
  systemModelMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemModelMatrix.GetQuickInverse());

  CGraphics::SetModelMatrix(systemModelMatrix);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

  const bool moveRedToAlpha = sMoveRedToAlphaBuffer;
  SUVElementSet uvs;

  CElementGen* const* genPtr = gens;
  for (GXTexMapID mapId = GX_TEXMAP0; mapId < count;
       ++genPtr, mapId = static_cast< GXTexMapID >(mapId + 1)) {
    const TLockedToken< CTexture >& tex =
        (*genPtr)->mLoadedGenDesc->mTEXR->GetValueTexture(0);
    (*tex)->Load(mapId, CTexture::kCM_Repeat);
  }

  CGX::SetVtxDescv(sVtxDescList);

  int nStages = 1;
  CGX::SetNumChans(1);

  GXTevStageID stage = GX_TEVSTAGE0;
  if (moveRedToAlpha) {
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    stage = GX_TEVSTAGE1;
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nStages = 2;
  }
  CGX::SetNumTevStages(nStages);
  CGX::SetNumTexGens(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

  int totalParticles = 0;
  for (int i = 0; i < count; ++i) {
    totalParticles += gens[i]->GetParticleCount();
  }

  CTexturedParticleListItem* sortItems = static_cast< CTexturedParticleListItem* >(
      alloca(totalParticles * sizeof(CTexturedParticleListItem)));

  int activeCount = 0;
  for (int i = 0; i < count; ++i) {
    CElementGen* gen = gens[i];
    int numParts = gen->GetParticleCount();
    float timeDelta = gen->mTimeDeltaScale;

    for (int j = 0; j < numParts; ++j) {
      CParticle& particle = gen->mParticles[j];
      if (particle.mEndFrame == -1)
        continue;

      // ?
      sortItems[activeCount].mViewPoint = systemCameraCopy * particle.mPos;

      // TODO: this should be the constructor
      sortItems[activeCount].mViewPoint =
          systemCameraCopy *
          ((particle.mPos - particle.mPrevPos) * timeDelta + particle.mPrevPos);
      sortItems[activeCount].mTexMapIdx = i;
      sortItems[activeCount].mPartIdx = j;
      activeCount++;
    }
  }

  rstl::sort(sortItems, sortItems + activeCount, CTexturedParticleListItemViewPointComp());

  CGenDescription* genDesc = nullptr;
  ushort lastMap = 0xFFFF;
  CElementGen* gen = nullptr;
  int emitterTime = 0;

  float cosMinusSin;
  float sinPlusCos;
  for (int i = 0; i < activeCount; ++i) {
    CTexturedParticleListItem* readPtr = &sortItems[i];
    ushort map = readPtr->mTexMapIdx;

    if (lastMap != map) {
      gen = gens[map];
      emitterTime = gen->GetEmitterTime();
      CGenDescription* desc = gens[map]->mLoadedGenDesc;
      genDesc = desc;

      if (!moveRedToAlpha) {
        if (desc->mAAPH) {
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
        } else {
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
        }
      } else {
        CGX::SetTevOrder(stage, GX_TEXCOORD0, static_cast< GXTexMapID >(map), GX_COLOR_NULL);
      }

      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, static_cast< GXTexMapID >(map), GX_COLOR0A0);
      lastMap = map;
    }

    CParticle& particle = gen->mParticles[readPtr->mPartIdx];

    int elapsed = emitterTime - particle.mStartFrame - 1;
    CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(elapsed);

    genDesc->mTEXR->GetValueUV(elapsed, uvs);

    CGX::Begin(GX_QUADS, GX_VTXFMT6, 4);

    float halfSize = 0.5f * particle.mLineLengthOrSize;
    float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.mColor.GetColor_u32();
    float sinT = halfSize * CMath::FastSinR(theta);
    float cosT = halfSize * CMath::FastCosR(theta);

    sinPlusCos = sinT + cosT;
    cosMinusSin = cosT - sinT;
    float sinMinusCos = sinT - cosT;
    float negSinPlusCos = -sinT + cosT;
    float negCos = -cosT;

    GXPosition3f32(sinPlusCos + readPtr->mViewPoint.GetX(), readPtr->mViewPoint.GetY(),
                   cosMinusSin + readPtr->mViewPoint.GetZ());
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMax);

    GXPosition3f32(sinMinusCos + readPtr->mViewPoint.GetX(), readPtr->mViewPoint.GetY(),
                   sinPlusCos + readPtr->mViewPoint.GetZ());
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMax);

    GXPosition3f32(readPtr->mViewPoint.GetX() - sinPlusCos, readPtr->mViewPoint.GetY(),
                   readPtr->mViewPoint.GetZ() - cosMinusSin);
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMin);

    GXPosition3f32(negSinPlusCos + readPtr->mViewPoint.GetX(), readPtr->mViewPoint.GetY(),
                   (negCos - sinT) + readPtr->mViewPoint.GetZ());
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMin);

    CGX::End();
  }

  if (moveRedToAlpha) {
    GXSetTevSwapMode(stage, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }

  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

void CElementGen::RenderParticlesIndirectTexture() {
  CTransform4f systemViewCopy(CGraphics::GetViewMatrix());
  systemViewCopy.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemViewCopy.GetQuickInverse() * mGlobalOrientation);
  systemViewCopy = CTransform4f::Translate(mGlobalTranslation) * mGlobalScaleTransform *
                   systemViewCopy * mLocalScaleTransform;
  CGraphics::SetModelMatrix(systemViewCopy);

  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

  if (mAAPH) {
    CGX::SetZMode(true, GX_LEQUAL, false);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetZMode(true, GX_LEQUAL, mZBUF);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  }

  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;
  SUVElementSet uvsInd;
  uvsInd.xMin = 0.f;
  uvsInd.xMax = 1.f;
  uvsInd.yMin = 0.f;
  uvsInd.yMax = 1.f;

  TToken< CTexture > texToken =
      mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  texToken->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CTexture* cachedTex = *texToken;

  bool constTexr = mLoadedGenDesc->mTEXR->HasConstantTexture();
  mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
  bool constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();

  TToken< CTexture > indTexToken =
      mLoadedGenDesc->mTIND->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  indTexToken->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  CTexture* cachedIndTex = *indTexToken;

  bool constIndTexr = mLoadedGenDesc->mTIND->HasConstantTexture();
  bool constIndUVs = mLoadedGenDesc->mTIND->HasConstantUV();
  mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvsInd);

  CGX::SetNumTexGens(3);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);

  float indMtx[2][3] = {
      {0.5f, 0.f, 0.f},
      {0.f, 0.5f, 0.f},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, 1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);

  CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetNumTevStages(2);

  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);

  if (!mLoadedGenDesc->mCIND) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_CPREV);
  } else {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  }

  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);

  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumChans(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);

  CParticleListItem* sortItems = nullptr;
  int particleCount = mParticles.size();
  bool SORT = mLoadedGenDesc->mSORT;

  if (SORT) {
    sortItems =
        static_cast< CParticleListItem* >(alloca(particleCount * sizeof(CParticleListItem)));
    CParticleListItem* cur = sortItems;
    for (int i = 0; i < particleCount; ++i) {
      CParticle& particle = mParticles[i];
      const CVector3f& delta = particle.mPos - particle.mPrevPos;
      const CVector3f& pos = delta * mTimeDeltaScale + particle.mPrevPos;
      cur->mViewPoint = systemCameraCopy * pos;
      cur->mPartIdx = static_cast< ushort >(i);
      cur++;
    }

    static CParticleListItemViewPointComp sComp;
    rstl::sort(sortItems, sortItems + particleCount, sComp);
  }

  static const GXVtxDescList skIndVtxDescList[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skIndVtxDescList);

  CParticleListItem* sortIt = sortItems;
  for (int i = 0; i < particleCount; ++i) {
    CParticle* particle = SORT ? &mParticles[sortIt->mPartIdx] : &mParticles[i];
    const CVector3f& viewPoint =
        systemCameraCopy *
        ((particle->mPos - particle->mPrevPos) * mTimeDeltaScale + particle->mPrevPos);

    float vpX = viewPoint.GetX();
    float vpY = viewPoint.GetY();
    float vpZ = viewPoint.GetZ();

    if (!constTexr) {
      TToken< CTexture > tex =
          mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - particle->mStartFrame);
      if (*tex != cachedTex) {
        tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        cachedTex = *tex;
      }
    }

    if (!constIndTexr) {
      TToken< CTexture > tex =
          mLoadedGenDesc->mTIND->GetValueTexture(mCurFrame - particle->mStartFrame);
      if (*tex != cachedIndTex) {
        tex->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
        cachedIndTex = *tex;
      }
    }

    if (!constUVs) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[i].mStartFrame, uvs);
    }

    if (!constIndUVs) {
      mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - mParticles[i].mStartFrame,
                                              uvsInd);
    }

    float size = particle->mLineLengthOrSize * 0.5f;

    CGraphics::CClippedScreenRect clipRect = CGraphics::ClipScreenRectFromMS(
        CVector3f(vpX - size, vpY, vpZ - size), CVector3f(size + vpX, vpY, size + vpZ), kTF_RGB565);

    int width = clipRect.GetTexWidth();
    int height = clipRect.GetHeight();
    float minU = clipRect.GetMinU();
    float maxU = clipRect.GetMaxU();
    float minV = clipRect.GetMinV();
    float maxV = clipRect.GetMaxV();

    if (clipRect.IsValid()) {
      void* dest = CGraphics::GetDolphinSpareBuffer();
      GXSetTexCopySrc(static_cast< u16 >(clipRect.GetX()), static_cast< u16 >(clipRect.GetY()),
                      static_cast< u16 >(clipRect.GetWidth()), static_cast< u16 >(height));
      GXSetTexCopyDst(static_cast< u16 >(width), static_cast< u16 >(height), GX_TF_RGB565,
                      GX_FALSE);

      size_t bufSize = CGraphics::GetSpareBufferSize();
      size_t texBufSize = GXGetTexBufferSize(width, height, GX_TF_RGB565, GX_FALSE, 0);
      if (texBufSize <= bufSize) {
        const bool useVideoFilter = CGraphics::GetUseVideoFilter();
        CGraphics::SetUseVideoFilter(false);
        GXCopyTex(dest, GX_FALSE);
        CGraphics::SetUseVideoFilter(useVideoFilter);
        GXPixModeSync();

        CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, NULL,
                                           CGraphics::kSpareBufferTexMapID);

        uint color = particle->mColor.GetColor_u32();
        CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);

        GXPosition3f32(size + vpX, vpY, size + vpZ);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMax);
        GXTexCoord2f32(maxU, minV);
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMax);

        GXPosition3f32(vpX - size, vpY, size + vpZ);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMax);
        GXTexCoord2f32(minU, minV);
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMax);

        GXPosition3f32(vpX - size, vpY, vpZ - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMin);
        GXTexCoord2f32(minU, maxV);
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMin);

        GXPosition3f32(size + vpX, vpY, vpZ - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMin);
        GXTexCoord2f32(maxU, maxV);
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMin);

        CGX::End();
      }
    }

#if NONMATCHING
    if (SORT) {
      ++sortIt;
    }
#else
    sortIt++;
#endif
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
}

void CElementGen::RenderLines() {
  bool hasModuColor = mModuColor.GetColor_u32() != 0xFFFFFFFF;
  CGlobalRandom gr(mRandState);

  CTransform4f systemViewPointMatrix(CGraphics::GetViewMatrix());
  systemViewPointMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraMatrix(systemViewPointMatrix.GetQuickInverse() * mGlobalOrientation);
  systemViewPointMatrix = CTransform4f::Translate(mGlobalTranslation) *
                          mGlobalScaleTransform * systemViewPointMatrix *
                          mLocalScaleTransform;
  CGraphics::SetModelMatrix(systemViewPointMatrix);

  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  if (mAAPH) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  bool constUVs = true;
  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  const bool widtConst =
      mLoadedGenDesc->mWIDT != nullptr && mLoadedGenDesc->mWIDT->IsConstant();

  if (mLoadedGenDesc->mTEXR != nullptr) {
    int partFrame = mCurFrame - mParticles[0].mStartFrame;
    TToken< CTexture > tex = mLoadedGenDesc->mTEXR->GetValueTexture(partFrame);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }

    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
    constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skDescList);

  if (hasModuColor) {
    CGX::SetNumChans(2);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanMatColor(CGX::Channel1, mModuColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
  }

  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  if (widtConst) {
    float lineWidth = 1.f;
    mLoadedGenDesc->mWIDT->GetValue(0, lineWidth);
    if (lineWidth < 0.f) {
      lineWidth = 0.f;
    } else if (lineWidth > 42.5f) {
      lineWidth = 42.5f;
    }
    CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
    CGX::Begin(GX_LINES, GX_VTXFMT0, static_cast< ushort >(mParticles.size() * 2));
  }

  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];

    if (!constUVs) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame, uvs);
    }

    CVector3f dVec = particle.mPos - particle.mPrevPos;

    if (mFXLL) {
      float mag = dVec.Magnitude();
      if (mag > 0.f) {
        dVec *= 1.f / mag;
      }
    }

    CVector3f p1 = systemCameraMatrix * particle.mPos;
    CVector3f p2 = systemCameraMatrix * (particle.mPos + (particle.mLineLengthOrSize * dVec));

    if (widtConst) {
      uint color = particle.mColor.GetColor_u32();
      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
    } else {
      float lineWidth = particle.mLineWidthOrRota;
      if (lineWidth < 0.f) {
        lineWidth = 0.f;
      } else if (lineWidth > 42.5f) {
        lineWidth = 42.5f;
      }
      CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
      CGX::Begin(GX_LINES, GX_VTXFMT0, 2);
      uint color = particle.mColor.GetColor_u32();
      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      CGX::End();
    }
  }

  if (widtConst) {
    CGX::End();
  }

  CGraphics::SetLineWidth(1.f, kTO_Zero);
}

void CElementGen::RenderModels() {
  CParticleGlobals::mParticleAccessParameters = NULL;
  if (mModelsUseLights) {
    CGraphics::SetLightState(mBackupLightActive);
  } else {
    CGraphics::SetAmbientColor(CColor::White());
  }

  bool moveRedToAlphaBuffer = false;
  SUVElementSet uvs;
  CGlobalRandom gr(mRandState);
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  if (mLoadedGenDesc->mPMUS) {
    bool enableMRTA = false;
    if (sMoveRedToAlphaBuffer && mLoadedGenDesc->mPMAB &&
        mLoadedGenDesc->mTEXR != NULL) {
      enableMRTA = true;
    }
    moveRedToAlphaBuffer = enableMRTA;

    if (mLoadedGenDesc->mPMAB) {
      CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
      if (enableMRTA) {
        CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
      } else {
        CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
        CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
      }
    } else {
      CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
      CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
    }

    CGraphics::SetCullMode(kCM_None);

    if (mLoadedGenDesc->mTEXR != NULL) {
      int partFrame = mCurFrame - mParticles[0].mStartFrame;
      TToken< CTexture > tex(mLoadedGenDesc->mTEXR->GetValueTexture(partFrame));
      tex.GetT()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

      if (enableMRTA) {
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
        CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
        CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
        CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
        CGX::SetNumTevStages(2);

        static const GXVtxDescList skDescList[] = {
            {GX_VA_POS, GX_DIRECT},
            {GX_VA_CLR0, GX_DIRECT},
            {GX_VA_TEX0, GX_DIRECT},
            {GX_VA_NULL, GX_NONE},
        };
        CGX::SetVtxDescv(skDescList);
        CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                         GX_AF_NONE);
        CGX::SetNumChans(1);
        CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                            GX_PTIDENTITY);
        CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        CGX::SetNumTexGens(1);
      } else {
        CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
      }

      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
      mLoadedGenDesc->mTEXR->HasConstantUV();
    } else {
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
  }

  CVector3f pmopVec(0.f, 0.f, 0.f);

  CTransform4f orient(CTransform4f::Identity());
  if (!mLoadedGenDesc->mPMOO) {
    orient = mOrientation;
  }
  orient = orient * mGlobalOrientation;

  bool pmrtConst = false;
  if (mLoadedGenDesc->mPMRT != NULL && mLoadedGenDesc->mPMRT->IsFastConstant()) {
    pmrtConst = true;
  }

  CVector3f trans =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mGlobalTranslation;

  CTransform4f rot(CTransform4f::Identity());
  if (pmrtConst) {
    CVector3f pmrtVal(0.f, 0.f, 0.f);
    mLoadedGenDesc->mPMRT->GetValue(mCurFrame, pmrtVal);
    rot = CTransform4f::RotateZ(CRelAngle::FromDegrees(pmrtVal.GetZ()));
    rot.RotateLocalY(CRelAngle::FromDegrees(pmrtVal.GetY()));
    rot.RotateLocalX(CRelAngle::FromDegrees(pmrtVal.GetX()));
  }
  rot = orient * rot;

  CParticleGlobals::SetEmitterTime(mCurFrame);
  CColor col = mModuColor;

  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    if (particle.mEndFrame != -1) {
      int lifetime = particle.mEndFrame - particle.mStartFrame;
      int partFrame = mCurFrame - particle.mStartFrame - 1;
      CParticleGlobals::SetParticleLifetime(lifetime);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
      CParticleGlobals::mCurrentParticle = &particle;

      if (mEnableADV) {
        CParticleGlobals::mParticleAccessParameters = mAdvValues[i].values;
      }

      if (mLoadedGenDesc->mPMOP != NULL) {
        mLoadedGenDesc->mPMOP->GetValue(partFrame, pmopVec);
      }

      CTransform4f partTrans = CTransform4f::Translate(
          trans +
          ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos));

      if (mOrientType == kMOT_One) {
        CTransform4f partRot(mParentMatrices[i], CVector3f::Zero());
        partTrans.AddTranslation((orient * partRot) * pmopVec);
        partTrans *= partRot;
      } else {
        partTrans.AddTranslation(orient * pmopVec);
      }

      if (pmrtConst) {
        partTrans *= rot;
      } else if (mLoadedGenDesc->mPMRT != NULL) {
        CVector3f pmrtVal(0.f, 0.f, 0.f);
        mLoadedGenDesc->mPMRT->GetValue(partFrame, pmrtVal);
        rot = CTransform4f::RotateZ(CRelAngle::FromDegrees(pmrtVal.GetZ()));
        rot.RotateLocalY(CRelAngle::FromDegrees(pmrtVal.GetY()));
        rot.RotateLocalX(CRelAngle::FromDegrees(pmrtVal.GetX()));
        partTrans *= (orient * rot);
      } else {
        partTrans *= rot;
      }

      if (mLoadedGenDesc->mPMSC != NULL) {
        CVector3f pmscVal(0.f, 0.f, 0.f);
        mLoadedGenDesc->mPMSC->GetValue(partFrame, pmscVal);
        partTrans *= CTransform4f::Scale(pmscVal.GetX(), pmscVal.GetY(), pmscVal.GetZ());
      }

      if (mLoadedGenDesc->mPMCL != NULL) {
        mLoadedGenDesc->mPMCL->GetValue(partFrame, col);
        col = CColor::Modulate(col, mModuColor);
      }

      CGraphics::SetModelMatrix(mGlobalScaleTransform * partTrans * mLocalScaleTransform);

      if (mLoadedGenDesc->mPMUS) {
        if (moveRedToAlphaBuffer) {
          CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
          GXPosition3f32(0.5f, 0.f, 0.5f);
          u32 c = col.GetColor_u32();
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);
          GXPosition3f32(-0.5f, 0.f, 0.5f);
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);
          GXPosition3f32(-0.5f, 0.f, -0.5f);
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);
          GXPosition3f32(0.5f, 0.f, -0.5f);
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
          CGX::End();
        } else {
          CGraphics::StreamBegin(kP_Quads);
          CGraphics::StreamColor(col.GetColor_u32());
          CGraphics::StreamTexcoord(uvs.xMax, uvs.yMax);
          CGraphics::StreamVertex(0.5f, 0.f, 0.5f);
          CGraphics::StreamTexcoord(uvs.xMin, uvs.yMax);
          CGraphics::StreamVertex(-0.5f, 0.f, 0.5f);
          CGraphics::StreamTexcoord(uvs.xMin, uvs.yMin);
          CGraphics::StreamVertex(-0.5f, 0.f, -0.5f);
          CGraphics::StreamTexcoord(uvs.xMax, uvs.yMin);
          CGraphics::StreamVertex(0.5f, 0.f, -0.5f);
          CGraphics::StreamEnd();
        }
      } else {
        CModel* model = mLoadedGenDesc->mPMDL->GetObject();
        if (sSubtractBlend) {
          model->Draw(CModelFlags::AlphaBlended(0.5f).DepthCompareUpdate(true, false));
        } else if (mLoadedGenDesc->mPMAB) {
          model->Draw(CModelFlags::Additive(col).DepthCompareUpdate(true, false));
        } else if (1.f == col.GetAlpha()) {
          model->Draw(CModelFlags::Normal());
        } else {
          model->Draw(CModelFlags::AlphaBlended(col.GetAlpha()).DepthCompareUpdate(true, false));
        }
      }
    }
  }

  if (mModelsUseLights) {
    CGraphics::DisableAllLights();
  }

  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

int CElementGen::GetParticleCountAll() const { return mRecursiveParticleCount; }

int CElementGen::GetParticleCountAllInternal() const {
  int ret = mActiveParticleCount;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      ret += static_cast< CElementGen* >(child)->GetParticleCountAll();
    }
  }
  return ret;
}

int CElementGen::GetSystemCount() {
  int ret = static_cast< int >(mActiveParticleCount) > 0;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      ret += static_cast< CElementGen* >(child)->GetSystemCount();
    } else {
      ret += 1;
    }
  }
  return ret;
}

void CElementGen::AccumulateBounds(const CVector3f& pos, float size) {
  float x = pos.GetX();
  float y = pos.GetY();
  float z = pos.GetZ();
  if (x > mAabbMax.GetX())
    mAabbMax.SetX(x);
  if (x < mAabbMin.GetX())
    mAabbMin.SetX(x);
  if (y > mAabbMax.GetY())
    mAabbMax.SetY(y);
  if (y < mAabbMin.GetY())
    mAabbMin.SetY(y);
  if (z > mAabbMax.GetZ())
    mAabbMax.SetZ(z);
  if (z < mAabbMin.GetZ())
    mAabbMin.SetZ(z);
  if (size > mMaxSize)
    mMaxSize = size;
}

void CElementGen::BuildParticleSystemBounds() {
  CAABox childBounds = CAABox::MakeMaxInvertedBox();
  bool accumulated = false;

  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    rstl::optional_object< CAABox > chBounds = (*it)->GetBounds();
    if (chBounds) {
      accumulated = true;
      const CAABox& bounds = *chBounds;
      childBounds.AccumulateBounds(bounds.GetMinPoint());
      childBounds.AccumulateBounds(bounds.GetMaxPoint());
    }
  }

  mRecursiveParticleCount = GetParticleCountAllInternal();

  if (GetParticleCount() > 0) {
    CVector3f scale = CVector3f::ByElementMultiply(
        CVector3f(mMaxSize, mMaxSize, mMaxSize), mGlobalScale);

    CAABox box = CAABox(mAabbMin, mAabbMax)
                     .GetTransformedAABox((mGlobalScaleTransform * mGlobalOrientation) *
                                          mLocalScaleTransform);

    mSystemBounds = CAABox(box.GetMinPoint() + mGlobalTranslation - scale,
                               box.GetMaxPoint() + mGlobalTranslation + scale);
  } else {
    mSystemBounds = CAABox::MakeMaxInvertedBox();
  }

  if (accumulated) {
    mSystemBounds.AccumulateBounds(childBounds.GetMinPoint());
    mSystemBounds.AccumulateBounds(childBounds.GetMaxPoint());
  }
}

rstl::optional_object< CAABox > CElementGen::GetBounds() const {
  if (GetParticleCountAll() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  return rstl::optional_object< CAABox >(mSystemBounds);
}

int CElementGen::GetEmitterTime() const { return mCurFrame; }

bool CElementGen::SystemHasLight() const { return mLightType != kLT_None; }

CLight CElementGen::GetLight() const {
  switch (mLightType) {
  case kLT_Directional: {
    return CLight::BuildDirectional(
        mLDIR.AsNormalized(),
        CColor(mLCLR.GetRed() * mLINT, mLCLR.GetGreen() * mLINT,
               mLCLR.GetBlue() * mLINT, mLCLR.GetAlpha() * mLINT));
  }
  case kLT_Spot: {
    CLight spotLight =
        CLight::BuildSpot(mLOFF, mLDIR.AsNormalized(),
                          CColor(mLCLR.GetRed() * mLINT, mLCLR.GetGreen() * mLINT,
                                 mLCLR.GetBlue() * mLINT, mLCLR.GetAlpha() * mLINT),
                          mLSLA);
    float quad = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    spotLight.SetAttenuation(constant, linear, quad);
    return spotLight;
  }
  default: {
    float quad = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    return CLight::BuildCustom(mLOFF, CVector3f(1.f, 0.f, 0.f), mLCLR, constant, linear,
                               quad, mLINT, 0.f, 0.f);
  }
  }
}

void CElementGen::SetGeneratorRate(float rate) {
  mGeneratorRate = rstl::max_val(rate, 0.f);
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      child->SetGeneratorRate(mGeneratorRate);
    }
  }
}
float CElementGen::GetGeneratorRate() const { return mGeneratorRate; }



uint CElementGen::Get4CharId() const { return 'PART'; }

int CElementGen::GetNumActiveChildParticles() const { return mActivePartChildren.size(); }

CParticleGen* CElementGen::GetActiveChildParticle(int index) const {
  return mActivePartChildren[index];
}

void CElementGen::SetExternalVar(int index, float val) { mExternalVars[index] = val; }

float CElementGen::GetExternalVar(int index) const { return mExternalVars[index]; }

bool CElementGen::IsIndirectTextured() const {
  return mLoadedGenDesc->mTEXR != nullptr && mLoadedGenDesc->mTIND != nullptr;
}

bool CElementGen::GetParticleEmission() const { return mParticleEmission; }

const CTransform4f& CElementGen::GetGlobalOrientation() const { return mGlobalOrientation; }

const CVector3f& CElementGen::GetGlobalTranslation() const { return mGlobalTranslation; }
