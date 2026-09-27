#include "MetroidPrime/CSimpleShadow.hpp"

#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"

#include "MetaRender/CCubeRenderer.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"

CSimpleShadow::CSimpleShadow(float scale, float userAlpha, float maxObjHeight, float displacement)
: mXf(CTransform4f::Identity())
, mScale(scale)
, mRadius(1.f)
, mUserAlpha(userAlpha)
, mHeightAlpha(1.f)
, mMaxObjHeight(maxObjHeight)
, mDisplacement(displacement)
, mCollision(false)
, mAlwaysCalculateRadius(true)
, mRadiusCalculated(false) {}

void CSimpleShadow::Calculate(const CAABox& aabb, const CTransform4f& xf,
                              const CStateManager& mgr) {
  mCollision = false;
  const CVector3f extent = aabb.GetMaxPoint() - aabb.GetMinPoint();
  const float halfHeight = extent.GetZ() / 2.f;
  const float xExtent = extent.GetX();
  const float yExtent = extent.GetY();

  CVector3f pos = xf.GetTranslation() + CVector3f(0.f, 0.f, halfHeight);
  CVector3f dir(0.0f, 0.0f, -1.0f);
  CRayCastResult res =
      mgr.RayStaticIntersection(pos, dir, mMaxObjHeight,
                                CMaterialFilter::MakeExclude(CMaterialList(kMT_SeeThrough)));
  float height = mMaxObjHeight;
  if (res.IsValid()) {
    mCollision = true;
    height = res.GetTime();
  }

  CRayCastResult closestResult = res;

  if (height > 0.1f + halfHeight) {
    TEntityList nearList;
    mgr.BuildNearList(nearList, pos, dir, mMaxObjHeight,
                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Platform)), nullptr);

    TUniqueId cid = kInvalidUniqueId;
    CRayCastResult resD =
        CGameCollision::RayDynamicIntersection(mgr, cid, pos, dir, mMaxObjHeight,
                                               CMaterialFilter::GetPassEverything(), nearList);
    if (resD.IsValid() && resD.GetTime() < height) {
      closestResult = resD;
      mCollision = true;
      height = resD.GetTime();
    }
  }

  if (mCollision) {
    mHeightAlpha = 1.f - height / mMaxObjHeight;
    CVector3f normalVector = closestResult.GetPlane().GetNormal();
    mXf = CTransform4f::LookAt(normalVector, CVector3f::Zero());
    mXf.SetTranslation(closestResult.GetPoint() + mDisplacement * normalVector);
    if (mAlwaysCalculateRadius || !mRadiusCalculated) {
      mRadius = sqrtf(xExtent * xExtent + yExtent * yExtent) / 2.f;
      mRadiusCalculated = true;
    }
  }
}

void CSimpleShadow::Render(const CTexture* tex) const {
  if (!mCollision)
    return;

  CGraphics::DisableAllLights();
  gpRender->SetModelMatrix(mXf);
  tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  float radius = mRadius * mScale;
  CGraphics::StreamBegin(kP_Quads);
  CGraphics::StreamColor(CCast::ToUint8((mHeightAlpha * mUserAlpha) * 255.f) - 0x100);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, -radius));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, -radius));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, radius));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, radius));
  CGraphics::StreamEnd();
}

const CTransform4f& CSimpleShadow::GetTransform() const { return mXf; }

void CSimpleShadow::SetUserAlpha(float alpha) { mUserAlpha = alpha; }

float CSimpleShadow::GetMaxObjectHeight() const { return mMaxObjHeight; }

void CSimpleShadow::SetAlwaysCalculateRadius(bool b) { mAlwaysCalculateRadius = b; }

CAABox CSimpleShadow::GetBounds() const {
  const CVector3f& translation = mXf.GetTranslation();
  float extent = mRadius * mScale;
  return CAABox(translation - CVector3f(extent, extent, extent),
                translation + CVector3f(extent, extent, extent));
}

CAABox CSimpleShadow::GetMaxShadowBox(const CAABox& aabb) const {
  float extent = mRadius * mScale;
  CVector3f center = aabb.GetCenterPoint();
  CAABox expandedAABB = aabb;
  expandedAABB.AccumulateBounds(center + CVector3f(extent, extent, -GetMaxObjectHeight()));
  expandedAABB.AccumulateBounds(center + CVector3f(-extent, -extent, -GetMaxObjectHeight()));
  return expandedAABB;
}

bool CSimpleShadow::Valid() const { return mCollision; }
