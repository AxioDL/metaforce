#include "Runtime/Graphics/CSimpleShadow.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"

namespace metaforce {

CSimpleShadow::CSimpleShadow(float scale, float userAlpha, float maxObjHeight, float displacement)
: x30_scale(scale), x38_userAlpha(userAlpha), x40_maxObjHeight(maxObjHeight), x44_displacement(displacement) {}

zeus::CAABox CSimpleShadow::GetMaxShadowBox(const zeus::CAABox& aabb) const {
  float extent = x34_radius * x30_scale;
  zeus::CVector3f center = aabb.center();
  zeus::CAABox expandedAABB = aabb;
  expandedAABB.accumulateBounds({center.x() + extent, center.y() + extent, center.z() - GetMaxObjectHeight()});
  expandedAABB.accumulateBounds({center.x() - extent, center.y() - extent, center.z() - GetMaxObjectHeight()});
  return expandedAABB;
}

zeus::CAABox CSimpleShadow::GetBounds() const {
  float extent = x34_radius * x30_scale;
  return {
      {x0_xf.origin.x() - extent, x0_xf.origin.y() - extent, x0_xf.origin.z() - extent},
      {x0_xf.origin.x() + extent, x0_xf.origin.y() + extent, x0_xf.origin.z() + extent},
  };
}

void CSimpleShadow::Render(TLockedToken<CTexture>& tex) {
  if (!x48_24_collision)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CSimpleShadow::Render", zeus::skGrey);

  CGraphics::DisableAllLights();
  CGraphics::SetModelMatrix(x0_xf);
  tex->Load(GX::TEXMAP0, EClampMode::Repeat);

  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
  CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                          ERglLogicOp::Clear);
  CGraphics::StreamBegin(GX::QUADS);
  float radius = x34_radius * x30_scale;
  CGraphics::StreamColor(zeus::CColor{1.f, x3c_heightAlpha * x38_userAlpha});
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(-radius, 0.f, -radius);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(radius, 0.f, -radius);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(radius, 0.f, radius);
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(-radius, 0.f, radius);
  CGraphics::StreamEnd();
}

void CSimpleShadow::Calculate(const zeus::CAABox& aabb, const zeus::CTransform& xf, const CStateManager& mgr) {
  x48_24_collision = false;
  float halfHeight = (aabb.max.z() - aabb.min.z()) * 0.5f;
  zeus::CVector3f pos = xf.origin + zeus::CVector3f(0.f, 0.f, halfHeight);
  CRayCastResult res = mgr.RayStaticIntersection(pos, zeus::skDown, x40_maxObjHeight,
                                                 CMaterialFilter::MakeExclude({EMaterialTypes::SeeThrough}));
  float height = x40_maxObjHeight;
  if (res.IsValid()) {
    x48_24_collision = true;
    height = res.GetT();
  }

  if (height > 0.1f + halfHeight) {
    TUniqueId cid = kInvalidUniqueId;
    EntityList nearList;
    CRayCastResult resD = CGameCollision::RayDynamicIntersection(mgr, cid, pos, zeus::skDown, x40_maxObjHeight,
                                                                 CMaterialFilter::skPassEverything, nearList);
    if (resD.IsValid() && resD.GetT() < height) {
      x48_24_collision = true;
      height = resD.GetT();
      res = resD;
    }
  }

  if (x48_24_collision) {
    x3c_heightAlpha = 1.f - height / x40_maxObjHeight;
    x0_xf = zeus::lookAt(res.GetPlane().normal(), zeus::skZero3f);
    x0_xf.origin = res.GetPlane().normal() * x44_displacement + res.GetPoint();
    if (x48_25_alwaysCalculateRadius || !x48_26_radiusCalculated) {
      float xExtent = aabb.max.x() - aabb.min.x();
      float yExtent = aabb.max.y() - aabb.min.y();
      x34_radius = std::sqrt(xExtent * xExtent + yExtent * yExtent) * 0.5f;
      x48_26_radiusCalculated = true;
    }
  }
}

} // namespace metaforce
