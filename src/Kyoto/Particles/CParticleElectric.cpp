#include "Kyoto/Particles/CParticleElectric.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "dolphin/gx/GXVert.h"
#include "float.h"
#include "math.h"
#include "rstl/math.hpp"

ushort CParticleElectric::sSeed = 99;

CParticleElectric::CParticleElectric(TToken< CElectricDescription > desc)
: mElecDesc(desc)
, mCurrentFrame(0)
, mLIFE(0)
, mCurTime(0.0)
, mTranslation(CVector3f::Zero())
, mOrientation(CTransform4f::Identity())
, mInvOrientation(CTransform4f::Identity())
, mGlobalTranslation(CVector3f::Zero())
, mGlobalOrientation(CTransform4f::Identity())
, mGlobalScale(1.f, 1.f, 1.f)
, mLocalScale(1.f, 1.f, 1.f)
, mCachedXf(CTransform4f::Identity())
, x128_(0.f)
, mRenderTime(0.f)
, mMinBounds(CVector3f::Zero())
, mMaxBounds(CVector3f::Zero())
, x148_(0.f)
, mRandState(sSeed++)
, mSSEG(8)
, mSCNT(1)
, x158_(0)
, mGenRem(0.f)
, mSystemBounds(CAABox::MakeMaxInvertedBox())
, mModuColor(CColor::White())
, mAllocated(false)
, mEmitting(true)
, mHaveGPSM(false)
, mHaveEPSM(false)
, mHaveSSWH(false)
, mHaveLWD(false)
, mTransformDirty(true) {
  if (mElecDesc->mSSEG) {
    mElecDesc->mSSEG->GetValue(mCurrentFrame, mSSEG);
  }
  if (mElecDesc->mSCNT) {
    mElecDesc->mSCNT->GetValue(mCurrentFrame, mSCNT);
  }
  if (mSCNT > 32) {
    mSCNT = 32;
  }
  if (mElecDesc->mLIFE) {
    mElecDesc->mLIFE->GetValue(0, mLIFE);
  } else {
    mLIFE = 0x7fffff;
  }
  if (mElecDesc->mSSWH) {
    mHaveSSWH = true;
    for (int i = 0; i < mSCNT; ++i) {
      mSwooshGenerators.push_back(
          rs_new CParticleSwoosh(mElecDesc->mSSWH->GetToken(), mSSEG));
      CParticleSwoosh& swoosh = *mSwooshGenerators.back();
      const int count = swoosh.GetSwooshCount();
      for (int j = 0; j < count; ++j) {
        swoosh.SetWarmUp();
        swoosh.Update(0.0);
      }
    }
  }
  ++mSSEG;
  mCalculatedVerts =
      rstl::vector< CVector3f, rstl::aligned_allocator >(mSSEG, CVector3f::Zero(),
                                                        rstl::aligned_allocator());
  mFractalOffsets =
      rstl::vector< CVector3f >(mSSEG, CVector3f::Zero(), rstl::rmemory_allocator());
  mFractalMags = rstl::vector< float >(mSSEG, 0.f, rstl::rmemory_allocator());
  if (mElecDesc->mGPSM) {
    mHaveGPSM = true;
    mGpsmGenerators.reserve(mSCNT);
    for (int i = 0; i < mSCNT; ++i) {
      mGpsmGenerators.push_back(rs_new CElementGen(mElecDesc->mGPSM->GetToken()));
      mGpsmGenerators.back()->SetParticleEmission(false);
    }
  }
  if (mElecDesc->mEPSM) {
    mHaveEPSM = true;
    mEpsmGenerators.reserve(mSCNT);
    for (int i = 0; i < mSCNT; ++i) {
      mEpsmGenerators.push_back(rs_new CElementGen(mElecDesc->mEPSM->GetToken()));
      mEpsmGenerators.back()->SetParticleEmission(false);
    }
  }
  if (mElecDesc->mLWD1 || mElecDesc->mLWD2 || mElecDesc->mLWD3) {
    mHaveLWD = true;
    for (int i = 0; i < mSCNT; ++i) {
      mLineManagers.push_back(rs_new CLineManager);
    }
  }
}

CParticleElectric::~CParticleElectric() {}

void CParticleElectric::CalculatePoints() {
  rstl::reserved_vector< CVector3f, 4 > points;
  CVector3f pos = CVector3f::Zero();
  CVector3f vel = CVector3f::Zero();
  if (mElecDesc->mIEMT) {
    mElecDesc->mIEMT->GetValue(mCurrentFrame, pos, vel);
  }
  if (mOverrideIPos) {
    pos = *mOverrideIPos;
  }
  if (mOverrideIVel) {
    vel = *mOverrideIVel;
  }
  if (vel.IsNonZero()) {
    points.push_back(pos);
    points.push_back(pos + vel);
    points.push_back(pos + vel * 2.f);
  } else {
    points.push_back(pos);
  }
  CVector3f fpos(0.f, 1.f, 0.f);
  CVector3f fvel = CVector3f::Zero();
  if (mElecDesc->mFEMT) {
    mElecDesc->mFEMT->GetValue(mCurrentFrame, fpos, fvel);
  }
  if (mOverrideFPos) {
    fpos = *mOverrideFPos;
  }
  if (mOverrideFVel) {
    fvel = *mOverrideFVel;
  }
  if (fvel.IsNonZero()) {
    if (points.size() == 3) {
      points[2] = fpos + fvel;
      points.push_back(fpos);
    } else {
      points.push_back(fpos + fvel * 2.f);
      points.push_back(fpos + fvel);
      points.push_back(fpos);
    }
  } else {
    points.push_back(fpos);
  }
  if (points.size() == 4) {
    const int segments = mSSEG - 1;
    const float segDiv = 1.f / static_cast< float >(segments);
    mCalculatedVerts[0] = points[0];
    float curDiv = segDiv;
    for (int i = 1; i < segments; ++i) {
      const float t = segDiv * mRandState.Range(-0.45f, 0.45f) + curDiv;
      const CVector3f point = CMath::GetBezierPoint(points[0], points[1], points[2], points[3], t);
      mCalculatedVerts[i] = point;
      curDiv += segDiv;
    }
    mCalculatedVerts[segments] = points[3];
  } else {
    mCalculatedVerts[0] = pos;
    const int segments = mSSEG - 1;
    const float segDiv = 1.f / static_cast< float >(segments);
    CVector3f accum = mCalculatedVerts[0];
    const CVector3f segDelta = (fpos - pos) * segDiv;
    for (int i = 1; i < segments; ++i) {
      const float r = mRandState.Range(-0.45f, 0.45f);
      mCalculatedVerts[i] = accum + segDelta * r;
      accum += segDelta;
    }
    mCalculatedVerts[segments] = fpos;
  }
  for (int i = 0; i < mSSEG; ++i) {
    mFractalMags[i] = 0.f;
  }
  float ampl = 1.f;
  if (mElecDesc->mAMPL) {
    mElecDesc->mAMPL->GetValue(mCurrentFrame, ampl);
    ampl *= 2.f;
  }
  float ampd = 0.f;
  if (mElecDesc->mAMPD) {
    mElecDesc->mAMPD->GetValue(mCurrentFrame, ampd);
  }
  CalculateFractal(0, mCalculatedVerts.size() - 1, ampl, ampd);
  CVector3f v0 = mCalculatedVerts[0] - mCalculatedVerts[1];
  CVector3f v1 = mCalculatedVerts[mCalculatedVerts.size() - 1] - mCalculatedVerts[1];
  CVector3f up = CVector3f::Up();
  if (v0.CanBeNormalized() && v1.CanBeNormalized()) {
    v0.Normalize();
    v1.Normalize();
    float dot = CVector3f::Dot(v0, v1);
    dot = dot < 0.f ? -dot : dot;
    if (close_enough(dot, 1.f)) {
      up = CTransform4f::LookAt(mCalculatedVerts[0], mCalculatedVerts[1]).GetUp();
    } else {
      up = CVector3f::Cross(v0, v1).AsNormalized();
    }
  } else if (!(mCalculatedVerts[0] == mCalculatedVerts[1])) {
    up = CTransform4f::LookAt(mCalculatedVerts[0], mCalculatedVerts[1]).GetUp();
  }
  const float commonRand = mRandState.Range(0.f, 360.f);
  for (int i = 1; i < mCalculatedVerts.size() - 1; ++i) {
    const CVector3f delta = mCalculatedVerts[i] - mCalculatedVerts[i - 1];
    if (delta.IsNonZero()) {
      const float randomAngle = mFractalMags[i] / ampl * 16.f * mRandState.Range(-1.f, 1.f);
      const CRelAngle angle = CRelAngle::FromDegrees(randomAngle + commonRand);
      const CQuaternion rot = CQuaternion::AxisAngle(delta, angle);
      mFractalOffsets[i] = rot.Transform(mFractalMags[i] * up);
    }
  }
  for (int i = 1; i < mCalculatedVerts.size() - 1; ++i) {
    mCalculatedVerts[i] += mFractalOffsets[i];
  }
  if (mElecDesc->mZERY) {
    for (int i = 0; i < mCalculatedVerts.size(); ++i) {
      mCalculatedVerts[i].SetY(0.f);
    }
  }
}

void CParticleElectric::CalculateFractal(int start, int end, float ampl, float ampd) {
  float mag;
  const float ratio =
      static_cast< float >(end - start) / static_cast< float >(mFractalMags.size());
  const float startMag = mFractalMags[start];
  const int mid = (start + end) / 2;
  mag = ratio * ampl;
  const float endMag = mFractalMags[end];
  const float base = (startMag + endMag) / 2.f + mag * mRandState.Float() - mag / 2.f;
  mFractalMags[mid] = base + (ampd * mRandState.Float() - ampd / 2.f);
  if ((start + end) % 2 == 1) {
    mFractalMags[end - 1] = mFractalMags[end];
  }
  if (mid - start > 1) {
    CalculateFractal(start, mid, ampl, ampd);
  }
  if (end - mid > 1) {
    CalculateFractal(mid, end, ampl, ampd);
  }
}

const bool CParticleElectric::Update(double dt) {
  CGlobalRandom random(mRandState);
  bool updated = false;
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mGpsmGenerators[i]->IsSystemDeletable()) {
        break;
      }
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mEpsmGenerators[i]->IsSystemDeletable()) {
        break;
      }
    }
  }
  const bool emitting = mEmitting && mCurrentFrame < mLIFE;
  double evalTime = mCurrentFrame * (1.0 / 60.0);
  mCurTime += dt;
  if (mTransformDirty) {
    UpdateCachedTransform();
    const CTransform4f orientation = mCachedXf.GetRotation();
    const CVector3f translation = mCachedXf.GetTranslation();
    if (mHaveSSWH) {
      for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
        mSwooshGenerators[it->mIdx]->SetGlobalTranslation(translation);
        mSwooshGenerators[it->mIdx]->SetGlobalOrientation(orientation);
        mSwooshGenerators[it->mIdx]->SetGlobalScale(mGlobalScale);
        mSwooshGenerators[it->mIdx]->SetLocalScale(mLocalScale);
      }
    }
    if (mHaveGPSM) {
      for (int i = 0; i < mSCNT; ++i) {
        mGpsmGenerators[i]->SetGlobalTranslation(translation);
        mGpsmGenerators[i]->SetGlobalOrientation(orientation);
        mGpsmGenerators[i]->SetGlobalScale(mGlobalScale);
        mGpsmGenerators[i]->SetLocalScale(mLocalScale);
      }
    }
    if (mHaveEPSM) {
      for (int i = 0; i < mSCNT; ++i) {
        mEpsmGenerators[i]->SetGlobalTranslation(translation);
        mEpsmGenerators[i]->SetGlobalOrientation(orientation);
        mEpsmGenerators[i]->SetGlobalScale(mGlobalScale);
        mEpsmGenerators[i]->SetLocalScale(mLocalScale);
      }
    }
    updated = true;
  }
  while (evalTime < mCurTime) {
    CParticleGlobals::SetEmitterTime(mCurrentFrame);
    UpdateElectricalEffects();
    if (emitting) {
      AddElectricalEffects();
    }
    if (mHaveGPSM) {
      if (mCurrentFrame >= mLIFE) {
        for (int i = 0; i < mSCNT; ++i) {
          mGpsmGenerators[i]->EndLifetime();
        }
      }
      for (int i = 0; i < mSCNT; ++i) {
        mGpsmGenerators[i]->Update(1.0 / 60.0);
      }
    }
    if (mHaveEPSM) {
      if (mCurrentFrame >= mLIFE) {
        for (int i = 0; i < mSCNT; ++i) {
          mEpsmGenerators[i]->EndLifetime();
        }
      }
      for (int i = 0; i < mSCNT; ++i) {
        mEpsmGenerators[i]->Update(1.0 / 60.0);
      }
    }
    updated = true;
    evalTime += 1.0 / 60.0;
    ++mCurrentFrame;
  }
  if (updated) {
    BuildBounds();
  }
  return updated;
}

void CParticleElectric::Render() {
  const float startTime = CStopwatch::GetGlobalTime();
  if (!mElectricManagers.empty()) {
    if (mTransformDirty) {
      UpdateCachedTransform();
    }
    if (mHaveSSWH) {
      RenderSwooshes();
    }
    if (mHaveLWD) {
      RenderLines();
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGpsmGenerators[i]->Render();
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEpsmGenerators[i]->Render();
    }
  }
  mRenderTime = CStopwatch::GetGlobalTime() - startTime;
}

void CParticleElectric::RenderSwooshes() {
  for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
    mSwooshGenerators[it->mIdx]->Render();
  }
}

void CParticleElectric::RenderLines() {
  CGraphics::DisableAllLights();
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  CGraphics::SetModelMatrix(CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation *
                            CTransform4f::Translate(mTranslation) * mOrientation *
                            CTransform4f::Scale(mGlobalScale) *
                            CTransform4f::Scale(mLocalScale));
  CGraphics::SetCullMode(kCM_None);
  SetupLineGXMaterial();
  for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
    CLineManager& line = *mLineManagers[it->mIdx];
    if (mElecDesc->mLWD1) {
      DrawLineStrip(line.mVerts, line.mWidth1, line.mColor1);
    }
    if (mElecDesc->mLWD2) {
      DrawLineStrip(line.mVerts, line.mWidth2, line.mColor2);
    }
    if (mElecDesc->mLWD3) {
      DrawLineStrip(line.mVerts, line.mWidth3, line.mColor3);
    }
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetLineWidth(1.f, kTO_Zero);
}

void CParticleElectric::SetupLineGXMaterial() {
  static const GXVtxDescList vtxDesc[] = {{GX_VA_POS, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
}

void CParticleElectric::DrawLineStrip(
    const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
    const CColor& color) {
  CGraphics::SetLineWidth(width, kTO_Zero);
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(GetModulationColor(), color).GetGXColor());
  const int count = verts.size();
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, count);
  for (int i = 0; i < count; ++i) {
    const CVector3f& vert = verts[i];
    GXPosition3f32(vert[kDX], vert[kDY], vert[kDZ]);
  }
  CGX::End();
}

void CParticleElectric::AddElectricalEffects() {
  float rate = 0.f;
  if (mElecDesc->mGRAT) {
    if (mElecDesc->mGRAT->GetValue(mCurrentFrame, rate)) {
      mElectricManagers.clear();
      for (int i = 0; i < mAllocated.size(); ++i) {
        mAllocated[i] = false;
      }
      return;
    } else {
      rate = rstl::max_val(0.f, rate);
    }
  }
  mGenRem += rate;
  const int count = static_cast< int >(floor(mGenRem));
  mGenRem -= count;
  CreateNewParticles(count);
}

void CParticleElectric::CreateNewParticles(int count) {
  int allocIdx = 0;
  for (int i = 0; i < count; ++i) {
    if (mElectricManagers.size() >= mSCNT) {
      return;
    }
    const CTransform4f orientation = mCachedXf.GetRotation();
    const CVector3f translation = mCachedXf.GetTranslation();
    for (; allocIdx < mAllocated.size(); ++allocIdx) {
      if (mAllocated[allocIdx]) {
        continue;
      }
      mAllocated[allocIdx] = true;
      int lifetime = 1;
      if (mElecDesc->mSLIF) {
        mElecDesc->mSLIF->GetValue(mCurrentFrame, lifetime);
      }
      mElectricManagers.push_back(
          CParticleElectricManager(allocIdx, lifetime, mCurrentFrame));
      CParticleElectricManager& manager = mElectricManagers.back();
      int frame = mCurrentFrame - manager.mStartFrame;
      CParticleGlobals::SetParticleLifetime(manager.mEndFrame - manager.mStartFrame);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
      CalculatePoints();
      if (mHaveSSWH) {
        CParticleSwoosh& swoosh = *mSwooshGenerators[allocIdx];
        swoosh.SetParticleEmission(true);
        swoosh.SetGlobalTranslation(translation);
        swoosh.SetGlobalOrientation(orientation);
        swoosh.SetGlobalScale(mGlobalScale);
        swoosh.SetLocalScale(mLocalScale);
        CColor color = CColor::White();
        if (mElecDesc->mCOLR) {
          mElecDesc->mCOLR->GetValue(frame, color);
        }
        swoosh.SetModulationColor(CColor::Modulate(color, mModuColor));
        int curParticle = swoosh.mCurParticle;
        for (int j = 0; j < swoosh.mSwooshes.size(); ++j) {
          curParticle = (curParticle + 1) % swoosh.mSwooshes.size();
          CParticleSwoosh::SSwooshData& data = swoosh.mSwooshes[curParticle];
          data.mTranslation = mCalculatedVerts[j];
        }
      }
      if (mHaveLWD) {
        CLineManager& line = *mLineManagers[allocIdx];
        line.mVerts = mCalculatedVerts;
        UpdateLine(allocIdx, 0);
        if (!mHaveSSWH) {
          mMinBounds = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
          mMaxBounds = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
          for (int j = 0; j < mCalculatedVerts.size(); ++j) {
            const CVector3f& vert = mCalculatedVerts[j];
            mMaxBounds = CVector3f(rstl::max_val(vert.GetX(), mMaxBounds.GetX()),
                                       rstl::max_val(vert.GetY(), mMaxBounds.GetY()),
                                       rstl::max_val(vert.GetZ(), mMaxBounds.GetZ()));
            mMinBounds = CVector3f(rstl::min_val(vert.GetX(), mMinBounds.GetX()),
                                       rstl::min_val(vert.GetY(), mMinBounds.GetY()),
                                       rstl::min_val(vert.GetZ(), mMinBounds.GetZ()));
          }
          line.mBounds = CAABox(mMinBounds, mMaxBounds);
        }
      }
      if (mHaveGPSM) {
        for (int j = 0; j < mSCNT; ++j) {
          if (!mGpsmGenerators[j]->GetParticleEmission()) {
            const CTransform4f scale =
                CTransform4f::Scale(mGlobalScale) * CTransform4f::Scale(mLocalScale);
            mGpsmGenerators[j]->SetTranslation(scale * mCalculatedVerts.front());
            mGpsmGenerators[j]->SetParticleEmission(true);
            manager.mGpsmIdx = j;
            break;
          }
        }
      }
      if (mHaveEPSM) {
        for (int j = 0; j < mSCNT; ++j) {
          if (!mEpsmGenerators[j]->GetParticleEmission()) {
            const CTransform4f scale =
                CTransform4f::Scale(mGlobalScale) * CTransform4f::Scale(mLocalScale);
            mEpsmGenerators[j]->SetTranslation(scale * mCalculatedVerts.back());
            mEpsmGenerators[j]->SetParticleEmission(true);
            manager.mEpsmIdx = j;
            break;
          }
        }
      }
      break;
    }
  }
}

void CParticleElectric::UpdateElectricalEffects() {
  for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end();) {
    CParticleElectricManager& manager = *it;
    if (manager.mSlif <= 1) {
      mAllocated[manager.mIdx] = false;
      if (manager.mGpsmIdx != -1) {
        mGpsmGenerators[manager.mGpsmIdx]->SetParticleEmission(false);
      }
      if (manager.mEpsmIdx != -1) {
        mEpsmGenerators[manager.mEpsmIdx]->SetParticleEmission(false);
      }
      it = mElectricManagers.erase(it);
      continue;
    }
    int frame = mCurrentFrame - manager.mStartFrame;
    CParticleGlobals::SetParticleLifetime(manager.mEndFrame - manager.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
    if (mHaveSSWH) {
      CParticleSwoosh& swoosh = *mSwooshGenerators[manager.mIdx];
      CColor color = CColor::White();
      if (mElecDesc->mCOLR) {
        mElecDesc->mCOLR->GetValue(frame, color);
      }
      swoosh.SetModulationColor(CColor::Modulate(color, mModuColor));
    }
    if (mHaveLWD) {
      UpdateLine(manager.mIdx, frame);
    }
    --manager.mSlif;
    ++it;
  }
}

void CParticleElectric::UpdateLine(int idx, int frame) {
  CColorElement* element = mElecDesc->mLCL1;
  CLineManager& line = *mLineManagers[idx];
  if (element) {
    element->GetValue(frame, line.mColor1);
  }
  if (CColorElement* element = mElecDesc->mLCL2) {
    element->GetValue(frame, line.mColor2);
  }
  if (CColorElement* element = mElecDesc->mLCL3) {
    element->GetValue(frame, line.mColor3);
  }
  if (CRealElement* element = mElecDesc->mLWD1) {
    element->GetValue(frame, line.mWidth1);
  }
  if (CRealElement* element = mElecDesc->mLWD2) {
    element->GetValue(frame, line.mWidth2);
  }
  if (CRealElement* element = mElecDesc->mLWD3) {
    element->GetValue(frame, line.mWidth3);
  }
}

void CParticleElectric::BuildBounds() {
  if (GetParticleCount() <= 0) {
    mSystemBounds = CAABox::MakeMaxInvertedBox();
    return;
  }
  mSystemBounds = CAABox::MakeMaxInvertedBox();
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      rstl::optional_object< CAABox > bounds = mSwooshGenerators[it->mIdx]->GetBounds();
      if (bounds) {
        mSystemBounds.Include(*bounds);
      }
    }
  } else if (mHaveLWD) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      bounds.Include(mLineManagers[it->mIdx]->mBounds);
    }
    if (!bounds.Invalid()) {
      mSystemBounds.Include(bounds.GetTransformedAABox(
          CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation *
          CTransform4f::Translate(mTranslation) * mOrientation *
          CTransform4f::Scale(mGlobalScale)));
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      rstl::optional_object< CAABox > bounds = mGpsmGenerators[i]->GetBounds();
      if (bounds) {
        mSystemBounds.Include(*bounds);
      }
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      rstl::optional_object< CAABox > bounds = mEpsmGenerators[i]->GetBounds();
      if (bounds) {
        mSystemBounds.Include(*bounds);
      }
    }
  }
}

rstl::optional_object< CAABox > CParticleElectric::GetBounds() const {
  if (GetParticleCount() <= 0) {
    return rstl::optional_object_null();
  }
  return mSystemBounds;
}

void CParticleElectric::SetModulationColor(const CColor& color) { mModuColor = color; }

bool CParticleElectric::IsSystemDeletable() const {
  if (mEmitting && mCurrentFrame < mLIFE) {
    return false;
  }
  if (!mElectricManagers.empty()) {
    return false;
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mGpsmGenerators[i]->IsSystemDeletable()) {
        return false;
      }
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mEpsmGenerators[i]->IsSystemDeletable()) {
        return false;
      }
    }
  }
  return true;
}

int CParticleElectric::GetParticleCount() const {
  int count = 0;
  if (!mElectricManagers.empty()) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      if (mHaveSSWH) {
        count += mSwooshGenerators[it->mIdx]->GetParticleCount();
      }
      if (mHaveLWD) {
        count += mSSEG;
      }
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      count += mGpsmGenerators[i]->GetParticleCount();
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      count += mEpsmGenerators[i]->GetParticleCount();
    }
  }
  return count;
}

void CParticleElectric::ForceParticleCreation(int count) {
  CGlobalRandom random(mRandState);
  CreateNewParticles(count);
}

void CParticleElectric::SetTranslation(const CVector3f& translation) {
  mTranslation = translation;
  mTransformDirty = true;
}

void CParticleElectric::SetOrientation(const CTransform4f& orientation) {
  mOrientation = orientation;
  mInvOrientation = mOrientation.GetQuickInverse();
  mTransformDirty = true;
}

void CParticleElectric::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
  mTransformDirty = true;
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      mSwooshGenerators[it->mIdx]->SetGlobalTranslation(mGlobalTranslation);
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGpsmGenerators[i]->SetGlobalTranslation(mGlobalTranslation);
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEpsmGenerators[i]->SetGlobalTranslation(mGlobalTranslation);
    }
  }
}

void CParticleElectric::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation = orientation.GetRotation();
  mTransformDirty = true;
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      mSwooshGenerators[it->mIdx]->SetGlobalOrientation(mGlobalOrientation);
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGpsmGenerators[i]->SetGlobalOrientation(mGlobalOrientation);
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEpsmGenerators[i]->SetGlobalOrientation(mGlobalOrientation);
    }
  }
}

void CParticleElectric::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
  mTransformDirty = true;
}

void CParticleElectric::SetLocalScale(const CVector3f& scale) {
  mLocalScale = scale;
  mTransformDirty = true;
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      mSwooshGenerators[it->mIdx]->SetLocalScale(mLocalScale);
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGpsmGenerators[i]->SetLocalScale(mLocalScale);
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEpsmGenerators[i]->SetLocalScale(mLocalScale);
    }
  }
}

void CParticleElectric::UpdateCachedTransform() {
  mCachedXf = CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation *
                 CTransform4f::Translate(mTranslation) * mOrientation;
  mTransformDirty = false;
}

bool CParticleElectric::SystemHasLight() const {
  if (mHaveGPSM) {
    return mGpsmGenerators.front()->SystemHasLight();
  }
  if (mHaveEPSM) {
    return mEpsmGenerators.front()->SystemHasLight();
  }
  return false;
}

CLight CParticleElectric::GetLight() const {
  if (mHaveGPSM) {
    return mGpsmGenerators.front()->GetLight();
  }
  if (mHaveEPSM) {
    return mEpsmGenerators.front()->GetLight();
  }
  return CLight::BuildLocalAmbient(GetGlobalTranslation(), CColor::Orange());
}

void CParticleElectric::SetParticleEmission(const bool emission) { mEmitting = emission; }
uint CParticleElectric::Get4CharId() const { return 'ELSC'; }

void CParticleElectric::DestroyParticles() {}

const CTransform4f& CParticleElectric::GetGlobalOrientation() const {
  return mGlobalOrientation;
}
const CVector3f& CParticleElectric::GetGlobalTranslation() const { return mGlobalTranslation; }
const CTransform4f& CParticleElectric::GetOrientation() const { return mOrientation; }
const CVector3f& CParticleElectric::GetTranslation() const { return mTranslation; }
