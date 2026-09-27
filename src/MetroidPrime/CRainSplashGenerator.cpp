#include "MetroidPrime/CRainSplashGenerator.hpp"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"

#include "dolphin/gx/GXVert.h"

#include "rstl/math.hpp"

const float CRainSplashGenerator::SSplashLine::skInitialSpeed = 4.f;
const float CRainSplashGenerator::SSplashLine::skInitialHeight = 0.015625f;
const uchar CRainSplashGenerator::SSplashLine::skInitialWidth = 3;

static const GXVtxDescList vtxDescv[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

int CRainSplashGenerator::GetNextBestPt(const int pt, const CVector3f* vertices,
                                        const CVector3f* normals, int count, CRandom16& rand,
                                        float minZ) {
  int nextPt = pt;
  const CVector3f& refVert = vertices[pt];
  float maxDist = 0.f;
  for (int i = 0; i < 3; ++i) {
    const int idx = rand.Range(0, count - 1);
    const CVector3f& norm = normals[idx];
    const CVector3f& vert = vertices[idx];

    const float distSq = (refVert - vert).MagSquared();
    const float normDot = CVector3f::Dot(norm, CVector3f::Up());

    const bool goodNorm = normDot >= 0.f && normDot <= 1.f;

    bool goodZ;
    if (minZ > 0.f) {
      goodZ = vert.GetZ() > minZ;
    } else {
      goodZ = true;
    }

    if (distSq > maxDist && goodNorm && goodZ) {
      nextPt = idx;
      maxDist = distSq;
    }
  }
  return nextPt;
}

CRainSplashGenerator::CRainSplashGenerator(const CVector3f& scale, int maxSplashes, int genRate,
                                           float minZ, float alpha)
: mScale(scale)
, mGenerateTimer(0.f)
, mDt(0.f)
, mMinZ(minZ)
, mAlpha(alpha > 1.0f ? 255.f : alpha * 255.f)
, mCurPoint(0)
, mQueueTail(0)
, mQueueHead(0)
, mQueueSize(0)
, mGenRate(genRate > maxSplashes ? maxSplashes : genRate)
, x48_24(false)
, mRaining(true) {
  mRainSplashes.reserve(maxSplashes);
  for (int i = 0; i < maxSplashes; ++i) {
    mRainSplashes.push_back(SRainSplash());
  }
}

void CRainSplashGenerator::AddPoint(const CVector3f& pos) {
  if (mQueueTail >= mRainSplashes.size()) {
    mQueueTail = 0;
  }
  mRainSplashes[mQueueTail].SetPoint(pos);
  mQueueSize += 1;
  mQueueTail += 1;
}

void CRainSplashGenerator::GeneratePoints(const CVector3f* vertices, const CVector3f* normals,
                                          const int count) {
  if (!mRaining) {
    return;
  }

  if (mGenerateTimer > mGenerateInterval) {
    int pt = mCurPoint;
    for (int i = 0; i < mGenRate; ++i) {
      if (mQueueSize >= mRainSplashes.size()) {
        break;
      }
      const int nextPt = GetNextBestPt(pt, vertices, normals, count, mRandom, mMinZ);
      AddPoint(CVector3f::ByElementMultiply(mScale, vertices[nextPt]));
      pt = nextPt;
    }
    mCurPoint = pt;
    mGenerateTimer = 0.f;
  }
}

void CRainSplashGenerator::UpdateRainSplashRange(CStateManager& mgr, const int start, const int end,
                                                 float dt) {
  for (int i = start; i < end; ++i) {
    SRainSplash& set = mRainSplashes[i];
    set.Update(dt, mgr);
    if (!set.IsActive()) {
      mQueueSize -= 1;
      mQueueHead += 1;
      if (mQueueHead >= mRainSplashes.size()) {
        mQueueHead = 0;
      }
    }
  }
}

void CRainSplashGenerator::UpdateRainSplashes(CStateManager& mgr, float magnitude, float dt) {
  mGenerateTimer += dt;
  mGenerateInterval = 1.f / (70.f * magnitude);
  if (mQueueSize > 0) {
    if (mQueueTail <= mQueueHead) {
      UpdateRainSplashRange(mgr, mQueueHead, static_cast< int >(mRainSplashes.size()), dt);
      UpdateRainSplashRange(mgr, 0, mQueueTail, dt);
    } else {
      UpdateRainSplashRange(mgr, mQueueHead, mQueueTail, dt);
    }
  }
}

void CRainSplashGenerator::Update(float dt, CStateManager& mgr) {
  const CEnvFxManager& envFx = *mgr.GetEnvFxManager();
  EEnvFxType neededFx = mgr.GetWorld()->GetNeededEnvFx();
  mDt = dt;
  mRaining = false;
  if (neededFx != kEFX_None && envFx.IsSplashActive()) {
    if (envFx.GetRainMagnitude()) {
      switch (neededFx) {
      case kEFX_Rain:
        UpdateRainSplashes(mgr, envFx.GetRainMagnitude(), dt);
        mRaining = true;
        break;
      default:
        break;
      }
    }
  }
}

void CRainSplashGenerator::Draw(const CTransform4f& xf) const {
  if (!mRaining) {
    return;
  }

  DoDraw(xf);
}

void CRainSplashGenerator::DoDraw(const CTransform4f& xf) const {
  if (!(mDt <= 0.0f)) {
    CGX::SetVtxDescv(vtxDescv);
    CGX::SetNumChans(1);
    CGX::SetNumTevStages(1);
    CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    CGX::SetNumTexGens(0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGX::SetZMode(true, GX_LEQUAL, false);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::SetModelMatrix(xf);
    if (mQueueSize > 0) {
      if (mQueueTail <= mQueueHead) {
        for (int i = mQueueHead; i < mRainSplashes.size(); ++i) {
          const SRainSplash& splash = mRainSplashes[i];
          splash.Draw(mAlpha, mDt, splash.mPos);
        }
        for (int i = 0; i < mQueueTail; ++i) {
          const SRainSplash& splash = mRainSplashes[i];
          splash.Draw(mAlpha, mDt, splash.mPos);
        }
      } else {
        for (int i = mQueueHead; i < mQueueTail; ++i) {
          const SRainSplash& splash = mRainSplashes[i];
          splash.Draw(mAlpha, mDt, splash.mPos);
        }
      }
    }
    CGX::SetLineWidth(6, GX_TO_ZERO);
  }
}

void CRainSplashGenerator::SSplashLine::SetActive() { mActive = true; }

void CRainSplashGenerator::SSplashLine::Update(float dt, CStateManager& mgr) {
  if (!mActive) {
    return;
  }
  if (mT <= 0.8f) {
    mLineWidth = CCast::ToUint8(5.f * (1.f - mT) + 3.f * mT);
    mT += dt * mSpeed;
  } else if (mLength != 0) {
    mLength -= 1;
  } else {
    mActive = false;
    mT = 0.f;
    mSpeed = mgr.Random()->Range(4.0f, 8.0f);
    mZParabolaHeight = mgr.Random()->Range(0.015625f, 0.03125f);
    mXEnd = mgr.Random()->Range(-0.125f, 0.125f);
    mYEnd = mgr.Random()->Range(-0.125f, 0.125f);
    mLength = static_cast< uchar >(mgr.Random()->Range(1, 2));
  }
}

void CRainSplashGenerator::SSplashLine::Draw(float alpha, float dt, const CVector3f& pos) const {
  if (mT > 0.f) {
    float delta = dt * mSpeed;
    const float trailTime = delta * CCast::ToReal32(mLength);
    float vt = mT - trailTime;
    if (vt < 0.0f) {
      vt = 0.0f;
    }
    int vertCount = static_cast< int >((mT - vt) / delta + 1.f);

    CGX::SetLineWidth(mLineWidth * 6, GX_TO_ZERO);
    CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, vertCount);

    for (int i = 0; i < vertCount; ++i) {
      const float height = -4.f * vt * (vt - 1.f) * mZParabolaHeight;
      GXPosition3f32(vt * mXEnd + pos.GetX(), vt * mYEnd + pos.GetY(), height + pos.GetZ());
      GXColor1u32(static_cast< uint >(vt * alpha) | 0xffffff00);
      vt += delta;
    }
    CGX::End();
  }
}

CRainSplashGenerator::SRainSplash::SRainSplash()
: mLines(SSplashLine()), mPos(CVector3f::Zero()), x70_(0.0f) {}

void CRainSplashGenerator::SRainSplash::Update(float dt, CStateManager& mgr) {
  for (AUTO(it, mLines.begin()); it != mLines.end(); ++it) {
    it->Update(dt, mgr);
  }
}

void CRainSplashGenerator::SRainSplash::Draw(float alpha, float dt, const CVector3f& pos) const {
  for (AUTO(it, mLines.begin()); it != mLines.end(); ++it) {
    it->Draw(alpha, dt, pos);
  }
}

const bool CRainSplashGenerator::SRainSplash::IsActive() const {
  bool ret = false;
  for (AUTO(it, mLines.begin()); it != mLines.end(); ++it) {
    ret |= it->mActive;
  }
  return ret;
}

void CRainSplashGenerator::SRainSplash::SetPoint(const CVector3f& pos) {
  for (AUTO(it, mLines.begin()); it != mLines.end(); ++it) {
    it->SetActive();
  }
  mPos = pos;
}
