#include "CCollidableOBBTree.hpp"
#include "CMaterialFilter.hpp"
#include "CollisionUtil.hpp"
#include "CInternalRayCastStructure.hpp"
#include "CCollisionInfoList.hpp"

namespace urde
{

u32 CCollidableOBBTree::sTableIndex = 0;

CCollidableOBBTree::CCollidableOBBTree(const COBBTree* tree, const urde::CMaterialList& material)
    : CCollisionPrimitive(material),
      x10_tree((COBBTree*)tree) {}

bool CCollidableOBBTree::LineIntersectsLeaf(const COBBTree::CLeafData& leaf, CRayCastInfo& info) const
{
    bool ret = false;
    u16 intersectIdx = 0;
    for (int i=0 ; i<leaf.GetSurfaceVector().size() ; ++i)
    {
        u16 surfIdx = leaf.GetSurfaceVector()[i];
        CCollisionSurface surface = x10_tree->GetSurface(surfIdx);
        if (info.GetMaterialFilter().Passes(GetMaterial()))
        {
            if (CollisionUtil::RayTriangleIntersection(info.GetRay().start, info.GetRay().dir,
                                                       surface.GetVerts(), info.Magnitude()))
            {
                intersectIdx = surfIdx;
                ret = true;
            }
        }
    }

    if (ret)
    {
        CCollisionSurface surf = x10_tree->GetSurface(intersectIdx);
        info.Plane() = surf.GetPlane();
        info.Material() = CMaterialList(surf.GetSurfaceFlags());
    }

    return ret;
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode& n0, const COBBTree::CNode& n1,
                                               CRayCastInfo& info) const
{
    bool ret = false;
    float t0, t1;
    bool intersects0 = false;
    bool intersects1 = false;
    if (CollisionUtil::LineIntersectsOBBox(n0.GetOBB(), info.GetRay(), t0) && t0 < info.GetMagnitude())
        intersects0 = true;
    if (CollisionUtil::LineIntersectsOBBox(n1.GetOBB(), info.GetRay(), t1) && t1 < info.GetMagnitude())
        intersects1 = true;

    if (intersects0 && intersects1)
    {
        if (t0 < t1)
        {
            if (n0.IsLeaf() ?
                LineIntersectsLeaf(n0.GetLeafData(), info) :
                LineIntersectsOBBTree(n0.GetLeft(), n0.GetRight(), info))
            {
                if (info.GetMagnitude() < t1)
                    return true;
                ret = true;
            }
            if (n1.IsLeaf())
            {
                if (LineIntersectsLeaf(n1.GetLeafData(), info))
                    return true;
            }
            else
            {
                if (LineIntersectsOBBTree(n1.GetLeft(), n1.GetRight(), info))
                    return true;
            }
        }
        else
        {
            if (n1.IsLeaf() ?
                LineIntersectsLeaf(n1.GetLeafData(), info) :
                LineIntersectsOBBTree(n1.GetLeft(), n1.GetRight(), info))
            {
                if (info.GetMagnitude() < t0)
                    return true;
                ret = true;
            }
            if (n0.IsLeaf())
            {
                if (LineIntersectsLeaf(n0.GetLeafData(), info))
                    return true;
            }
            else
            {
                if (LineIntersectsOBBTree(n0.GetLeft(), n0.GetRight(), info))
                    return true;
            }
        }
    }
    else if (intersects0)
    {
        return n0.IsLeaf() ?
               LineIntersectsLeaf(n0.GetLeafData(), info) :
               LineIntersectsOBBTree(n0.GetLeft(), n0.GetRight(), info);
    }
    else if (intersects1)
    {
        return n1.IsLeaf() ?
               LineIntersectsLeaf(n1.GetLeafData(), info) :
               LineIntersectsOBBTree(n1.GetLeft(), n1.GetRight(), info);
    }

    return ret;
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode& node, CRayCastInfo& info) const
{
    float t;
    bool ret = false;

    if (CollisionUtil::LineIntersectsOBBox(node.GetOBB(), info.GetRay(), t) && t < info.GetMagnitude())
    {
        if (node.IsLeaf())
        {
            if (LineIntersectsLeaf(node.GetLeafData(), info))
                ret = true;
        }
        else
        {
            if (LineIntersectsOBBTree(node.GetLeft(), node.GetRight(), info))
                ret = true;
        }
        const_cast<COBBTree::CNode&>(node).SetHit(true);
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return ret;
}

CRayCastResult CCollidableOBBTree::LineIntersectsTree(const zeus::CMRay& ray, const CMaterialFilter& filter,
                                                      float maxTime, const zeus::CTransform& xf) const
{
    zeus::CMRay useRay = ray.getInvUnscaledTransformRay(xf);
    CRayCastInfo info(useRay, filter, maxTime);
    if (LineIntersectsOBBTree(x10_tree->GetRoot(), info))
    {
        zeus::CPlane xfPlane = TransformPlane(info.GetPlane(), xf);
        return CRayCastResult(info.GetMagnitude(), ray.start + info.GetMagnitude() * ray.dir,
                              xfPlane, info.GetMaterial());
    }
    else
    {
        return {};
    }
}

zeus::CPlane CCollidableOBBTree::TransformPlane(const zeus::CPlane& pl, const zeus::CTransform& xf)
{
    zeus::CVector3f normal = xf.rotate(pl.normal());
    return zeus::CPlane(normal, (xf * (pl.normal() * pl.d())).dot(normal));
}

bool CCollidableOBBTree::SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf,
                                                     const zeus::CSphere& sphere, const CMaterialList& matList,
                                                     const CMaterialFilter& filter, const zeus::CVector3f& dir,
                                                     double& dOut, CCollisionInfo& infoOut) const
{
    bool ret = false;

    zeus::CAABox aabb(sphere.position - sphere.radius,
                      sphere.position + sphere.radius);
    zeus::CAABox moveAABB = aabb;
    zeus::CVector3f moveVec = float(dOut) * dir;
    moveAABB.accumulateBounds(aabb.max + moveVec);
    moveAABB.accumulateBounds(aabb.min + moveVec);

    zeus::CVector3f center = moveAABB.center();
    zeus::CVector3f extent = moveAABB.extents();

    for (u16 triIdx : leaf.GetSurfaceVector())
    {
        CCollisionSurface surf = x10_tree->GetTransformedSurface(triIdx, xf);
        CMaterialList triMat = GetMaterial();
        triMat.Add(CMaterialList(surf.GetSurfaceFlags()));
        if (filter.Passes(triMat))
        {
            if (CollisionUtil::TriBoxOverlap(center, extent,
                                             surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
            {
                const_cast<CCollidableOBBTree&>(*this).x1c_hits += 1;

                zeus::CVector3f surfNormal = surf.GetNormal();
                if ((sphere.position + moveVec - surf.GetVert(0)).dot(surfNormal) <= sphere.radius)
                {
                    float mag = (sphere.radius - (sphere.position - surf.GetVert(0)).dot(surfNormal)) / dir.dot(surfNormal);
                    zeus::CVector3f intersectPoint = sphere.position + mag * dir;

                    bool outsideEdges[] =
                    {(intersectPoint - surf.GetVert(0)).dot((surf.GetVert(1) - surf.GetVert(0)).cross(surfNormal)) < 0.f,
                     (intersectPoint - surf.GetVert(1)).dot((surf.GetVert(2) - surf.GetVert(1)).cross(surfNormal)) < 0.f,
                     (intersectPoint - surf.GetVert(2)).dot((surf.GetVert(0) - surf.GetVert(2)).cross(surfNormal)) < 0.f};

                    if (mag >= 0.f && !outsideEdges[0] && !outsideEdges[1] && !outsideEdges[2] && mag < dOut)
                    {
                        infoOut = CCollisionInfo(intersectPoint - sphere.radius * surfNormal, matList, triMat, surfNormal);
                        dOut = mag;
                        ret = true;
                    }

                    bool intersects = (sphere.position - surf.GetVert(0)).dot(surfNormal) <= sphere.radius;
                    bool testVert[] = {true, true, true};
                    const u16* edgeIndices = x10_tree->GetTriangleEdgeIndices(triIdx);
                    for (int k=0 ; k<3 ; ++k)
                    {
                        if (intersects || outsideEdges[k])
                        {
                            u16 edgeIdx = edgeIndices[k];
                            if (CMetroidAreaCollider::g_DupPrimitiveCheckCount != CMetroidAreaCollider::g_DupEdgeList[edgeIdx])
                            {
                                CMetroidAreaCollider::g_DupEdgeList[edgeIdx] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                                CMaterialList edgeMat(x10_tree->GetEdgeMaterial(edgeIdx));
                                if (!edgeMat.HasMaterial(EMaterialTypes::NoEdgeCollision))
                                {
                                    int nextIdx = (k + 1) % 3;
                                    zeus::CVector3f edgeVec = surf.GetVert(nextIdx) - surf.GetVert(k);
                                    float edgeVecMag = edgeVec.magnitude();
                                    edgeVec *= zeus::CVector3f(1.f / edgeVecMag);
                                    float dirDotEdge = dir.dot(edgeVec);
                                    zeus::CVector3f edgeRej = dir - dirDotEdge * edgeVec;
                                    float edgeRejMagSq = edgeRej.magSquared();
                                    zeus::CVector3f vertToSphere = sphere.position - surf.GetVert(k);
                                    float vtsDotEdge = vertToSphere.dot(edgeVec);
                                    zeus::CVector3f vtsRej = vertToSphere - vtsDotEdge * edgeVec;
                                    if (edgeRejMagSq > 0.f)
                                    {
                                        float tmp = 2.f * vtsRej.dot(edgeRej);
                                        float tmp2 = 4.f * edgeRejMagSq *
                                            (vtsRej.magSquared() - sphere.radius * sphere.radius) - tmp * tmp;
                                        if (tmp2 >= 0.f)
                                        {
                                            float mag = 0.5f / edgeRejMagSq * (-tmp - std::sqrt(tmp2));
                                            if (mag >= 0.f)
                                            {
                                                float t = mag * dirDotEdge + vtsDotEdge;
                                                if (t >= 0.f && t <= edgeVecMag && mag < dOut)
                                                {
                                                    zeus::CVector3f point = surf.GetVert(k) + t * edgeVec;
                                                    infoOut = CCollisionInfo(point, matList, edgeMat,
                                                                             (sphere.position + mag * dir - point).normalized());
                                                    dOut = mag;
                                                    ret = true;
                                                    testVert[k] = false;
                                                    testVert[nextIdx] = false;
                                                }
                                                else if (t < -sphere.radius && dirDotEdge <= 0.f)
                                                {
                                                    testVert[k] = false;
                                                }
                                                else if (t > edgeVecMag + sphere.radius && dirDotEdge >= 0.0)
                                                {
                                                    testVert[nextIdx] = false;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            testVert[k] = false;
                                            testVert[nextIdx] = false;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    u16 vertIndices[3];
                    x10_tree->GetTriangleVertexIndices(triIdx, vertIndices);

                    for (int k=0 ; k<3 ; ++k)
                    {
                        u16 vertIdx = vertIndices[k];
                        if (testVert[k])
                        {
                            if (CMetroidAreaCollider::g_DupPrimitiveCheckCount != CMetroidAreaCollider::g_DupVertexList[vertIdx])
                            {
                                CMetroidAreaCollider::g_DupVertexList[vertIdx] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                                double d = dOut;
                                if (CollisionUtil::RaySphereIntersection_Double(zeus::CSphere(surf.GetVert(k), sphere.radius),
                                                                                sphere.position, dir, d) && d >= 0.0)
                                {
                                    infoOut = CCollisionInfo(surf.GetVert(k), matList, x10_tree->GetVertMaterial(vertIdx),
                                                             (sphere.position + dir * d - surf.GetVert(k)).normalized());
                                    dOut = d;
                                    ret = true;
                                }
                            }
                        }
                        else
                        {
                            CMetroidAreaCollider::g_DupVertexList[vertIdx] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                        }
                    }
                }
            }
            else
            {
                const u16* edgeIndices = x10_tree->GetTriangleEdgeIndices(triIdx);
                CMetroidAreaCollider::g_DupEdgeList[edgeIndices[0]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupEdgeList[edgeIndices[1]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupEdgeList[edgeIndices[2]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;

                u16 vertIndices[3];
                x10_tree->GetTriangleVertexIndices(triIdx, vertIndices);
                CMetroidAreaCollider::g_DupVertexList[vertIndices[0]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupVertexList[vertIndices[1]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupVertexList[vertIndices[2]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
            }
        }
    }

    return ret;
}

bool CCollidableOBBTree::SphereCollisionMoving(const COBBTree::CNode& node, const zeus::CTransform& xf,
                                               const zeus::CSphere& sphere, const zeus::COBBox& obb,
                                               const CMaterialList& material, const CMaterialFilter& filter,
                                               const zeus::CVector3f& dir, double& dOut, CCollisionInfo& info) const
{
    bool ret = false;

    const_cast<CCollidableOBBTree&>(*this).x14_tries += 1;
    if (obb.OBBIntersectsBox(node.GetOBB()))
    {
        const_cast<COBBTree::CNode&>(node).SetHit(true);
        if (node.IsLeaf())
        {
            if (SphereCollideWithLeafMoving(node.GetLeafData(), xf, sphere, material, filter, dir, dOut, info))
                ret = true;
        }
        else
        {
            if (SphereCollisionMoving(node.GetLeft(), xf, sphere, obb, material, filter, dir, dOut, info))
                ret = true;
            if (SphereCollisionMoving(node.GetRight(), xf, sphere, obb, material, filter, dir, dOut, info))
                ret = true;
        }
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return ret;
}

bool CCollidableOBBTree::AABoxCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf,
                                                    const zeus::CAABox& aabb, const CMaterialList& matList,
                                                    const CMaterialFilter& filter, const CMovingAABoxComponents& components,
                                                    const zeus::CVector3f& dir, double& dOut, CCollisionInfo& infoOut) const
{
    bool ret = false;

    zeus::CAABox movedAABB = components.x6e8_aabb;
    zeus::CVector3f moveVec = float(dOut) * dir;
    movedAABB.accumulateBounds(aabb.min + moveVec);
    movedAABB.accumulateBounds(aabb.max + moveVec);

    zeus::CVector3f center = movedAABB.center();
    zeus::CVector3f extent = movedAABB.extents();

    zeus::CVector3f normal, point;

    for (u16 triIdx : leaf.GetSurfaceVector())
    {
        CCollisionSurface surf = x10_tree->GetTransformedSurface(triIdx, xf);
        CMaterialList triMat = GetMaterial();
        triMat.Add(CMaterialList(surf.GetSurfaceFlags()));
        if (filter.Passes(triMat))
        {
            if (CollisionUtil::TriBoxOverlap(center, extent,
                                             surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
            {
                const_cast<CCollidableOBBTree&>(*this).x1c_hits += 1;

                u16 vertIndices[3];
                x10_tree->GetTriangleVertexIndices(triIdx, vertIndices);

                double d = dOut;
                if (CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(surf, aabb, components.x6c4_vertIdxs,
                                                                                 dir, d, normal, point) && d < dOut)
                {
                    ret = true;
                    infoOut = CCollisionInfo(point, matList, triMat, normal);
                    dOut = d;
                }

                for (int k=0 ; k<3 ; ++k)
                {
                    u16 vertIdx = vertIndices[k];
                    const zeus::CVector3f& vtx = x10_tree->GetVert(vertIdx);
                    if (CMetroidAreaCollider::g_DupPrimitiveCheckCount != CMetroidAreaCollider::g_DupVertexList[vertIdx])
                    {
                        CMetroidAreaCollider::g_DupVertexList[vertIdx] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                        if (movedAABB.pointInside(vtx))
                        {
                            d = dOut;
                            if (CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(vtx, aabb, dir, d,
                                                                                             normal, point) && d < dOut)
                            {
                                CMaterialList vertMat(x10_tree->GetVertMaterial(vertIdx));
                                ret = true;
                                infoOut = CCollisionInfo(point, matList, vertMat, normal);
                                dOut = d;
                            }
                        }
                    }
                }

                const u16* edgeIndices = x10_tree->GetTriangleEdgeIndices(triIdx);
                for (int k=0 ; k<3 ; ++k)
                {
                    u16 edgeIdx = edgeIndices[k];
                    if (CMetroidAreaCollider::g_DupPrimitiveCheckCount != CMetroidAreaCollider::g_DupEdgeList[edgeIdx])
                    {
                        CMetroidAreaCollider::g_DupEdgeList[edgeIdx] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                        CMaterialList edgeMat(x10_tree->GetEdgeMaterial(edgeIdx));
                        if (!edgeMat.HasMaterial(EMaterialTypes::NoEdgeCollision))
                        {
                            d = dOut;
                            const CCollisionEdge& edge = x10_tree->GetEdge(edgeIdx);
                            if (CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(surf.GetVert(k),
                                                                                     surf.GetVert((k + 1) % 3),
                                                                                     components.x0_edges, dir, d, normal, point) &&
                                d < dOut)
                            {
                                ret = true;
                                infoOut = CCollisionInfo(point, matList, edgeMat, normal);
                                dOut = d;
                            }
                        }
                    }
                }
            }
            else
            {
                const u16* edgeIndices = x10_tree->GetTriangleEdgeIndices(triIdx);
                CMetroidAreaCollider::g_DupEdgeList[edgeIndices[0]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupEdgeList[edgeIndices[1]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupEdgeList[edgeIndices[2]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;

                u16 vertIndices[3];
                x10_tree->GetTriangleVertexIndices(triIdx, vertIndices);
                CMetroidAreaCollider::g_DupVertexList[vertIndices[0]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupVertexList[vertIndices[1]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
                CMetroidAreaCollider::g_DupVertexList[vertIndices[2]] = CMetroidAreaCollider::g_DupPrimitiveCheckCount;
            }
        }
    }

    return ret;
}

bool CCollidableOBBTree::AABoxCollisionMoving(const COBBTree::CNode& node, const zeus::CTransform& xf,
                                              const zeus::CAABox& aabb, const zeus::COBBox& obb,
                                              const CMaterialList& material, const CMaterialFilter& filter,
                                              const CMovingAABoxComponents& components, const zeus::CVector3f& dir,
                                              double& dOut, CCollisionInfo& info) const
{
    bool ret = false;

    const_cast<CCollidableOBBTree&>(*this).x14_tries += 1;
    if (obb.OBBIntersectsBox(node.GetOBB()))
    {
        const_cast<COBBTree::CNode&>(node).SetHit(true);
        if (node.IsLeaf())
        {
            if (AABoxCollideWithLeafMoving(node.GetLeafData(), xf, aabb, material, filter, components, dir, dOut, info))
                ret = true;
        }
        else
        {
            if (AABoxCollisionMoving(node.GetLeft(), xf, aabb, obb, material, filter, components, dir, dOut, info))
                ret = true;
            if (AABoxCollisionMoving(node.GetRight(), xf, aabb, obb, material, filter, components, dir, dOut, info))
                ret = true;
        }
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return ret;
}

bool CCollidableOBBTree::SphereCollisionBoolean(const COBBTree::CNode& node, const zeus::CTransform& xf,
                                                const zeus::CSphere& sphere, const zeus::COBBox& obb,
                                                const CMaterialFilter& filter) const
{
    const_cast<CCollidableOBBTree&>(*this).x14_tries += 1;
    if (obb.OBBIntersectsBox(node.GetOBB()))
    {
        const_cast<COBBTree::CNode&>(node).SetHit(true);
        if (node.IsLeaf())
        {
            for (u16 surfIdx : node.GetLeafData().GetSurfaceVector())
            {
                CCollisionSurface surf = x10_tree->GetTransformedSurface(surfIdx, xf);
                CMaterialList triMat = GetMaterial();
                triMat.Add(CMaterialList(surf.GetSurfaceFlags()));
                if (filter.Passes(triMat) && CollisionUtil::TriSphereOverlap(sphere,
                                             surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
                    return true;
            }
        }
        else
        {
            if (SphereCollisionBoolean(node.GetLeft(), xf, sphere, obb, filter))
                return true;
            if (SphereCollisionBoolean(node.GetRight(), xf, sphere, obb, filter))
                return true;
        }
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return false;
}

bool CCollidableOBBTree::AABoxCollisionBoolean(const COBBTree::CNode& node, const zeus::CTransform& xf,
                                               const zeus::CAABox& aabb, const zeus::COBBox& obb,
                                               const CMaterialFilter& filter) const
{
    zeus::CVector3f center = aabb.center();
    zeus::CVector3f extent = aabb.extents();

    const_cast<CCollidableOBBTree&>(*this).x14_tries += 1;
    if (obb.OBBIntersectsBox(node.GetOBB()))
    {
        const_cast<COBBTree::CNode&>(node).SetHit(true);
        if (node.IsLeaf())
        {
            for (u16 surfIdx : node.GetLeafData().GetSurfaceVector())
            {
                CCollisionSurface surf = x10_tree->GetTransformedSurface(surfIdx, xf);
                CMaterialList triMat = GetMaterial();
                triMat.Add(CMaterialList(surf.GetSurfaceFlags()));
                if (filter.Passes(triMat) && CollisionUtil::TriBoxOverlap(center, extent,
                                             surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
                    return true;
            }
        }
        else
        {
            if (AABoxCollisionBoolean(node.GetLeft(), xf, aabb, obb, filter))
                return true;
            if (AABoxCollisionBoolean(node.GetRight(), xf, aabb, obb, filter))
                return true;
        }
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return false;
}

bool CCollidableOBBTree::SphereCollideWithLeaf(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf,
                                               const zeus::CSphere& sphere, const CMaterialList& material,
                                               const CMaterialFilter& filter, CCollisionInfoList& infoList) const
{
    bool ret = false;
    zeus::CVector3f point, normal;

    for (u16 surfIdx : leaf.GetSurfaceVector())
    {
        CCollisionSurface surf = x10_tree->GetTransformedSurface(surfIdx, xf);
        CMaterialList triMat = GetMaterial();
        triMat.Add(CMaterialList(surf.GetSurfaceFlags()));
        if (filter.Passes(triMat))
        {
            const_cast<CCollidableOBBTree&>(*this).x1c_hits += 1;
            if (CollisionUtil::TriSphereIntersection(sphere,
                                                     surf.GetVert(0), surf.GetVert(1), surf.GetVert(2),
                                                     point, normal))
            {
                CCollisionInfo collision(point, material, triMat, normal);
                infoList.Add(collision, false);
                ret = true;
            }
        }
    }

    return ret;
}

bool CCollidableOBBTree::SphereCollision(const COBBTree::CNode& node, const zeus::CTransform& xf,
                                         const zeus::CSphere& sphere, const zeus::COBBox& obb,
                                         const CMaterialList& material, const CMaterialFilter& filter,
                                         CCollisionInfoList& infoList) const
{
    bool ret = false;

    const_cast<CCollidableOBBTree&>(*this).x14_tries += 1;
    if (obb.OBBIntersectsBox(node.GetOBB()))
    {
        const_cast<COBBTree::CNode&>(node).SetHit(true);
        if (node.IsLeaf())
        {
            if (SphereCollideWithLeaf(node.GetLeafData(), xf, sphere, material, filter, infoList))
                ret = true;
        }
        else
        {
            if (SphereCollision(node.GetLeft(), xf, sphere, obb, material, filter, infoList))
                ret = true;
            if (SphereCollision(node.GetRight(), xf, sphere, obb, material, filter, infoList))
                ret = true;
        }
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return ret;
}

bool CCollidableOBBTree::AABoxCollideWithLeaf(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf,
                                              const zeus::CAABox& aabb, const CMaterialList& material,
                                              const CMaterialFilter& filter, const zeus::CPlane* planes,
                                              CCollisionInfoList& infoList) const
{
    bool ret = false;
    zeus::CVector3f center = aabb.center();
    zeus::CVector3f extent = aabb.extents();

    for (u16 surfIdx : leaf.GetSurfaceVector())
    {
        CCollisionSurface surf = x10_tree->GetTransformedSurface(surfIdx, xf);
        CMaterialList triMat = GetMaterial();
        triMat.Add(CMaterialList(surf.GetSurfaceFlags()));
        if (filter.Passes(triMat) && CollisionUtil::TriBoxOverlap(center, extent,
                                     surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
        {
            zeus::CAABox newAABB = zeus::CAABox::skInvertedBox;
            const_cast<CCollidableOBBTree&>(*this).x1c_hits += 1;
            if (CMetroidAreaCollider::ConvexPolyCollision(planes, surf.GetVerts(), newAABB))
            {
                zeus::CPlane plane = surf.GetPlane();
                CCollisionInfo collision(newAABB, triMat, material,
                                         plane.normal(), -plane.normal());
                infoList.Add(collision, false);
                ret = true;
            }
        }
    }

    return ret;
}

bool CCollidableOBBTree::AABoxCollision(const COBBTree::CNode& node, const zeus::CTransform& xf,
                                        const zeus::CAABox& aabb, const zeus::COBBox& obb,
                                        const CMaterialList& material, const CMaterialFilter& filter,
                                        const zeus::CPlane* planes, CCollisionInfoList& infoList) const
{
    bool ret = false;

    const_cast<CCollidableOBBTree&>(*this).x14_tries += 1;
    if (obb.OBBIntersectsBox(node.GetOBB()))
    {
        const_cast<COBBTree::CNode&>(node).SetHit(true);
        if (node.IsLeaf())
        {
            if (AABoxCollideWithLeaf(node.GetLeafData(), xf, aabb, material, filter, planes, infoList))
                ret = true;
        }
        else
        {
            if (AABoxCollision(node.GetLeft(), xf, aabb, obb, material, filter, planes, infoList))
                ret = true;
            if (AABoxCollision(node.GetRight(), xf, aabb, obb, material, filter, planes, infoList))
                ret = true;
        }
    }
    else
    {
        const_cast<CCollidableOBBTree&>(*this).x18_misses += 1;
    }

    return ret;
}

FourCC CCollidableOBBTree::GetPrimType() const
{
    return SBIG('OBBT');
}

CRayCastResult CCollidableOBBTree::CastRayInternal(const CInternalRayCastStructure& rayCast) const
{
    return LineIntersectsTree(rayCast.GetRay(), rayCast.GetFilter(),
                              rayCast.GetMaxTime(), rayCast.GetTransform());
}

zeus::CAABox CCollidableOBBTree::CalculateAABox(const zeus::CTransform& xf) const
{
    return zeus::COBBox::FromAABox(x10_tree->CalculateLocalAABox(), xf).calculateAABox();
}

zeus::CAABox CCollidableOBBTree::CalculateLocalAABox() const
{
    return x10_tree->CalculateLocalAABox();
}

}
