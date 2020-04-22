#include "Runtime/Collision/CAreaOctTree.hpp"

#include "Runtime/Collision/CMaterialFilter.hpp"

#include <array>
#include <cfloat>
#include <cmath>
#include <utility>

#include <hecl/hecl.hpp>
#include <zeus/CVector2i.hpp>

namespace urde {

static bool _close_enough(float f1, float f2, float epsilon) { return std::fabs(f1 - f2) <= epsilon; }

static bool BoxLineTest(const zeus::CAABox& aabb, const zeus::CLine& line, float& lT, float& hT) {
  zeus::simd_floats aabbMinF(aabb.min.mSimd);
  zeus::simd_floats aabbMaxF(aabb.max.mSimd);
  zeus::simd_floats lineOrigin(line.origin.mSimd);
  zeus::simd_floats lineDir(line.dir.mSimd);
  const float* aabbMin = aabbMinF.data();
  const float* aabbMax = aabbMaxF.data();
  const float* lorigin = lineOrigin.data();
  const float* ldir = lineDir.data();
  lT = -FLT_MAX;
  hT = FLT_MAX;

  for (int i = 0; i < 3; ++i) {
    if (_close_enough(*ldir, 0.f, 0.000099999997f))
      if (*lorigin < *aabbMin || *lorigin > *aabbMax)
        return false;

    if (*ldir < 0.f) {
      if (*aabbMax - *lorigin < lT * *ldir)
        lT = (*aabbMax - *lorigin) * 1.f / *ldir;
      if (*aabbMin - *lorigin > hT * *ldir)
        hT = (*aabbMin - *lorigin) * 1.f / *ldir;
    } else {
      if (*aabbMin - *lorigin > lT * *ldir)
        lT = (*aabbMin - *lorigin) * 1.f / *ldir;
      if (*aabbMax - *lorigin < hT * *ldir)
        hT = (*aabbMax - *lorigin) * 1.f / *ldir;
    }

    ++aabbMin;
    ++aabbMax;
    ++lorigin;
    ++ldir;
  }

  return lT <= hT;
}

constexpr std::array SomeIndexA{1, 2, 4};

constexpr std::array SomeIndexB{1, 2, 0};

constexpr std::array<std::array<int, 8>, 8> SomeIndexC{{
    {0, 1, 2, 4, 5, 6, 8, 0xA},
    {0, 1, 2, 3, 5, 6, 8, 0xA},
    {0, 1, 2, 4, 5, 6, 9, 0xB},
    {0, 1, 2, 3, 5, 6, 9, 0xC},
    {0, 1, 2, 4, 5, 7, 8, 0xD},
    {0, 1, 2, 3, 5, 7, 8, 0xE},
    {0, 1, 2, 4, 5, 7, 9, 0xF},
    {0, 1, 2, 3, 5, 7, 9, 0xF},
}};

constexpr std::array<std::pair<int, std::array<int, 3>>, 16> SubdivIndex{{
    {0, {0, 0, 0}},
    {1, {0, 0, 0}},
    {1, {1, 0, 0}},
    {2, {0, 1, 0}},
    {2, {1, 0, 0}},
    {1, {2, 0, 0}},
    {2, {0, 2, 0}},
    {2, {2, 0, 0}},
    {2, {2, 1, 0}},
    {2, {1, 2, 0}},
    {3, {0, 2, 1}},
    {3, {1, 0, 2}},
    {3, {0, 1, 2}},
    {3, {2, 1, 0}},
    {3, {2, 0, 1}},
    {3, {1, 2, 0}},
}};

bool CAreaOctTree::Node::LineTestInternal(const zeus::CLine& line, const CMaterialFilter& filter, float lT, float hT,
                                          float maxT, const zeus::CVector3f& vec) const {
  float lowT = (1.f - FLT_EPSILON * 100.f) * lT;
  float highT = (1.f + FLT_EPSILON * 100.f) * hT;
  if (maxT != 0.f) {
    if (lowT < 0.f)
      lowT = 0.f;
    if (highT > maxT)
      highT = maxT;
    if (lowT > highT)
      return true;
  }

  if (x20_nodeType == ETreeType::Leaf) {
    TriListReference triList = GetTriangleArray();
    for (u16 i = 0; i < triList.GetSize(); ++i) {
      CCollisionSurface triangle = x1c_owner.GetMasterListTriangle(triList.GetAt(i));

      // https://en.wikipedia.org/wiki/Möller–Trumbore_intersection_algorithm
      // Find vectors for two edges sharing V0
      zeus::CVector3f e0 = triangle.GetVert(1) - triangle.GetVert(0);
      zeus::CVector3f e1 = triangle.GetVert(2) - triangle.GetVert(0);

      // Begin calculating determinant - also used to calculate u parameter
      zeus::CVector3f P = line.dir.cross(e1);
      float det = P.dot(e0);

      // If determinant is near zero, ray lies in plane of triangle
      // or ray is parallel to plane of triangle
      if (std::fabs(det) < (FLT_EPSILON * 10.f))
        continue;
      float invDet = 1.f / det;

      // Calculate distance from V1 to ray origin
      zeus::CVector3f T = line.origin - triangle.GetVert(0);

      // Calculate u parameter and test bound
      float u = invDet * T.dot(P);

      // The intersection lies outside of the triangle
      if (u < 0.f || u > 1.f)
        continue;

      // Prepare to test v parameter
      zeus::CVector3f Q = T.cross(e0);

      // Calculate T parameter and test bound
      float t = invDet * Q.dot(e1);
      if (t >= highT || t < lowT)
        continue;

      // Calculate V parameter and test bound
      float v = invDet * Q.dot(line.dir);
      if (v < 0.f || u + v > 1.f)
        continue;

      // Do material filter
      CMaterialList matList(triangle.GetSurfaceFlags());
      if (filter.Passes(matList))
        return false;
    }
  } else if (x20_nodeType == ETreeType::Branch) {
    if (GetChildFlags() == 0xA) // 2 leaves
    {
      for (int i = 0; i < 2; ++i) {
        Node child = GetChild(i);
        float tf1 = lT;
        float tf2 = hT;
        if (BoxLineTest(child.GetBoundingBox(), line, tf1, tf2))
          if (!child.LineTestInternal(line, filter, tf1, tf2, maxT, vec))
            return false;
      }
      return true;
    }

    zeus::CVector3f center = x0_aabb.center();

    zeus::CVector3f r6 = line.origin + lT * line.dir;
    zeus::CVector3f r7 = line.origin + hT * line.dir;
    zeus::CVector3f r9 = vec * (center - line.origin);

    int r28 = 0;
    int r25 = 0;
    int r26 = 0;
    for (size_t i = 0; i < 3; ++i) {
      if (r6[i] >= center[i])
        r28 |= SomeIndexA[i];
      if (r7[i] >= center[i])
        r25 |= SomeIndexA[i];
      if (r9[i] < r9[SomeIndexB[i]])
        r26 |= SomeIndexA[i];
    }

    float f21 = lT;
    int r26b = r28;
    const std::pair<int, std::array<int, 3>>& idx = SubdivIndex[SomeIndexC[r26][r28 ^ r25]];
    for (int i = 0; i <= idx.first; ++i) {
      float f22 = (i < idx.first) ? r9[idx.second[i]] : hT;
      if (f22 > lowT && f21 <= f22) {
        Node child = GetChild(r26b);
        if (child.x20_nodeType != ETreeType::Invalid)
          if (!child.LineTestInternal(line, filter, f21, f22, maxT, vec))
            return false;
      }
      if (i < idx.first)
        r26b ^= 1 << idx.second[i];
      f21 = f22;
    }
  }

  return true;
}

void CAreaOctTree::Node::LineTestExInternal(const zeus::CLine& line, const CMaterialFilter& filter, SRayResult& res,
                                            float lT, float hT, float maxT, const zeus::CVector3f& dirRecip) const {
  float lowT = (1.f - FLT_EPSILON * 100.f) * lT;
  float highT = (1.f + FLT_EPSILON * 100.f) * hT;
  if (maxT != 0.f) {
    if (lowT < 0.f)
      lowT = 0.f;
    if (highT > maxT)
      highT = maxT;
    if (lowT > highT)
      return;
  }

  if (x20_nodeType == ETreeType::Leaf) {
    TriListReference triList = GetTriangleArray();
    float bestT = highT;
    bool foundTriangle = false;
    SRayResult tmpRes;

    for (u16 i = 0; i < triList.GetSize(); ++i) {
      CCollisionSurface triangle = x1c_owner.GetMasterListTriangle(triList.GetAt(i));

      // https://en.wikipedia.org/wiki/Möller–Trumbore_intersection_algorithm
      // Find vectors for two edges sharing V0
      zeus::CVector3f e0 = triangle.GetVert(1) - triangle.GetVert(0);
      zeus::CVector3f e1 = triangle.GetVert(2) - triangle.GetVert(0);

      // Begin calculating determinant - also used to calculate u parameter
      zeus::CVector3f P = line.dir.cross(e1);
      float det = P.dot(e0);

      // If determinant is near zero, ray lies in plane of triangle
      // or ray is parallel to plane of triangle
      if (std::fabs(det) < (FLT_EPSILON * 10.f))
        continue;
      float invDet = 1.f / det;

      // Calculate distance from V1 to ray origin
      zeus::CVector3f T = line.origin - triangle.GetVert(0);

      // Calculate u parameter and test bound
      float u = invDet * T.dot(P);

      // The intersection lies outside of the triangle
      if (u < 0.f || u > 1.f)
        continue;

      // Prepare to test v parameter
      zeus::CVector3f Q = T.cross(e0);

      // Calculate T parameter and test bound
      float t = invDet * Q.dot(e1);
      if (t >= bestT || t < lowT)
        continue;

      // Calculate V parameter and test bound
      float v = invDet * Q.dot(line.dir);
      if (v < 0.f || u + v > 1.f)
        continue;

      // Do material filter
      CMaterialList matList(triangle.GetSurfaceFlags());
      if (filter.Passes(matList) && t <= bestT) {
        bestT = t;
        foundTriangle = true;
        tmpRes.x10_surface.emplace(triangle);
        tmpRes.x3c_t = t;
      }
    }

    if (foundTriangle) {
      res = tmpRes;
      res.x0_plane = res.x10_surface->GetPlane();
    }
  } else if (x20_nodeType == ETreeType::Branch) {
    if (GetChildFlags() == 0xA) // 2 leaves
    {
      std::array<SRayResult, 2> tmpRes;
      for (int i = 0; i < 2; ++i) {
        Node child = GetChild(i);
        float tf1 = lT;
        float tf2 = hT;
        if (BoxLineTest(child.GetBoundingBox(), line, tf1, tf2))
          child.LineTestExInternal(line, filter, tmpRes[i], tf1, tf2, maxT, dirRecip);
      }

      if (!tmpRes[0].x10_surface && !tmpRes[1].x10_surface) {
        res = SRayResult();
      } else if (tmpRes[0].x10_surface && tmpRes[1].x10_surface) {
        if (tmpRes[0].x3c_t < tmpRes[1].x3c_t)
          res = tmpRes[0];
        else
          res = tmpRes[1];
      } else if (tmpRes[0].x10_surface) {
        res = tmpRes[0];
      } else {
        res = tmpRes[1];
      }

      if (res.x3c_t > highT)
        res = SRayResult();

      return;
    }

    zeus::CVector3f center = x0_aabb.center(); // r26

    zeus::CVector3f lowPoint = line.origin + lT * line.dir;
    zeus::CVector3f highPoint = line.origin + hT * line.dir;
    std::array<int, 4> comps{-1, -1, -1, 0};
    std::array<float, 3> compT;

    int numComps = 0;
    for (size_t i = 0; i < compT.size(); ++i) {
      if (lowPoint[i] >= center[i] || highPoint[i] <= center[i]) {
        if (highPoint[i] >= center[i] || lowPoint[i] <= center[i]) {
          continue;
        }
      }
      if (_close_enough(line.dir[i], 0.f, 0.000099999997f)) {
        continue;
      }
      comps[numComps++] = static_cast<int>(i);
      compT[i] = dirRecip[i] * (center[i] - line.origin[i]);
    }

    // Sort componentT least to greatest
    switch (numComps) {
    default:
      return;
    case 0:
    case 1:
      break;
    case 2:
      if (compT[comps[1]] < compT[comps[0]])
        std::swap(comps[1], comps[0]);
      break;
    case 3:
      if (compT[0] < compT[1]) {
        if (compT[0] >= compT[2]) {
          comps[0] = 2;
          comps[1] = 0;
          comps[2] = 1;
        } else if (compT[1] < compT[2]) {
          comps[0] = 0;
          comps[1] = 1;
          comps[2] = 2;
        } else {
          comps[0] = 0;
          comps[1] = 2;
          comps[2] = 1;
        }
      } else {
        if (compT[1] >= compT[2]) {
          comps[0] = 2;
          comps[1] = 1;
          comps[2] = 0;
        } else if (compT[0] < compT[2]) {
          comps[0] = 1;
          comps[1] = 0;
          comps[2] = 2;
        } else {
          comps[0] = 1;
          comps[1] = 2;
          comps[2] = 0;
        }
      }
      break;
    }

    zeus::CVector3f lineStart = line.origin + (lT * line.dir);
    int selector = 0;
    if (lineStart.x() >= center.x())
      selector = 1;
    if (lineStart.y() >= center.y())
      selector |= 1 << 1;
    if (lineStart.z() >= center.z())
      selector |= 1 << 2;

    float tmpLoT = lT;
    for (int i = -1; i < numComps; ++i) {
      if (i >= 0)
        selector ^= 1 << comps[i];
      float tmpHiT = (i < numComps - 1) ? compT[comps[i + 1]] : hT;
      if (tmpHiT > lowT && tmpLoT <= tmpHiT) {
        Node child = GetChild(selector);
        if (child.x20_nodeType != ETreeType::Invalid)
          child.LineTestExInternal(line, filter, res, tmpLoT, tmpHiT, maxT, dirRecip);
        if (res.x10_surface) {
          if (res.x3c_t > highT)
            res = SRayResult();
          break;
        }
      }
      tmpLoT = tmpHiT;
    }
  }
}

bool CAreaOctTree::Node::LineTest(const zeus::CLine& line, const CMaterialFilter& filter, float length) const {
  if (x20_nodeType == ETreeType::Invalid)
    return true;

  float f1 = 0.f;
  float f2 = 0.f;
  if (!BoxLineTest(x0_aabb, line, f1, f2))
    return true;

  zeus::CVector3f recip = 1.f / line.dir;
  return LineTestInternal(line, filter, f1 - 0.000099999997f, f2 + 0.000099999997f, length, recip);
}

void CAreaOctTree::Node::LineTestEx(const zeus::CLine& line, const CMaterialFilter& filter, SRayResult& res,
                                    float length) const {
  if (x20_nodeType == ETreeType::Invalid)
    return;

  float lT = 0.f;
  float hT = 0.f;
  if (!BoxLineTest(x0_aabb, line, lT, hT))
    return;

  zeus::CVector3f recip = 1.f / line.dir;
  LineTestExInternal(line, filter, res, lT - 0.000099999997f, hT + 0.000099999997f, length, recip);
}

CAreaOctTree::Node CAreaOctTree::Node::GetChild(int idx) const {
  u16 flags = *reinterpret_cast<const u16*>(x18_ptr);
  const u32* offsets = reinterpret_cast<const u32*>(x18_ptr + 4);
  ETreeType type = ETreeType((flags >> (2 * idx)) & 0x3);

  if (type == ETreeType::Branch) {
    zeus::CAABox pos, neg, res;
    x0_aabb.splitZ(neg, pos);
    if (idx & 4) {
      zeus::CAABox(pos).splitY(neg, pos);
      if (idx & 2) {
        zeus::CAABox(pos).splitX(neg, pos);
        if (idx & 1)
          res = pos;
        else
          res = neg;
      } else {
        zeus::CAABox(neg).splitX(neg, pos);
        if (idx & 1)
          res = pos;
        else
          res = neg;
      }
    } else {
      zeus::CAABox(neg).splitY(neg, pos);
      if (idx & 2) {
        zeus::CAABox(pos).splitX(neg, pos);
        if (idx & 1)
          res = pos;
        else
          res = neg;
      } else {
        zeus::CAABox(neg).splitX(neg, pos);
        if (idx & 1)
          res = pos;
        else
          res = neg;
      }
    }

    return Node(x18_ptr + offsets[idx] + 36, res, x1c_owner, ETreeType::Branch);
  } else if (type == ETreeType::Leaf) {
    const float* aabb = reinterpret_cast<const float*>(x18_ptr + offsets[idx] + 36);
    zeus::CAABox aabbObj(aabb[0], aabb[1], aabb[2], aabb[3], aabb[4], aabb[5]);
    return Node(aabb, aabbObj, x1c_owner, ETreeType::Leaf);
  } else {
    return Node(nullptr, zeus::skNullBox, x1c_owner, ETreeType::Invalid);
  }
}

void CAreaOctTree::SwapTreeNode(u8* ptr, Node::ETreeType type) {
  if (type == Node::ETreeType::Branch) {
    u16* typeBits = reinterpret_cast<u16*>(ptr);
    *typeBits = hecl::SBig(*typeBits);
    u32* offsets = reinterpret_cast<u32*>(ptr + 4);

    for (int i = 0; i < 8; ++i) {
      Node::ETreeType ctype = Node::ETreeType((*typeBits >> (2 * i)) & 0x3);
      offsets[i] = hecl::SBig(offsets[i]);
      SwapTreeNode(ptr + offsets[i] + 36, ctype);
    }
  } else if (type == Node::ETreeType::Leaf) {
    float* aabb = reinterpret_cast<float*>(ptr);
    aabb[0] = hecl::SBig(aabb[0]);
    aabb[1] = hecl::SBig(aabb[1]);
    aabb[2] = hecl::SBig(aabb[2]);
    aabb[3] = hecl::SBig(aabb[3]);
    aabb[4] = hecl::SBig(aabb[4]);
    aabb[5] = hecl::SBig(aabb[5]);

    u16* countIdxs = reinterpret_cast<u16*>(ptr + 24);
    *countIdxs = hecl::SBig(*countIdxs);
    for (u16 i = 0; i < *countIdxs; ++i)
      countIdxs[i + 1] = hecl::SBig(countIdxs[i + 1]);
  }
}

CAreaOctTree::CAreaOctTree(const zeus::CAABox& aabb, Node::ETreeType treeType, const u8* buf, const u8* treeBuf,
                           u32 matCount, const u32* materials, const u8* vertMats, const u8* edgeMats,
                           const u8* polyMats, u32 edgeCount, const CCollisionEdge* edges, u32 polyCount,
                           const u16* polyEdges, u32 vertCount, const float* verts)
: x0_aabb(aabb)
, x18_treeType(treeType)
, x1c_buf(buf)
, x20_treeBuf(treeBuf)
, x24_matCount(matCount)
, x28_materials(materials)
, x2c_vertMats(vertMats)
, x30_edgeMats(edgeMats)
, x34_polyMats(polyMats)
, x38_edgeCount(edgeCount)
, x3c_edges(edges)
, x40_polyCount(polyCount)
, x44_polyEdges(polyEdges)
, x48_vertCount(vertCount)
, x4c_verts(verts) {
  SwapTreeNode(const_cast<u8*>(x20_treeBuf), treeType);

  for (u32 i = 0; i < matCount; ++i)
    const_cast<u32*>(x28_materials)[i] = hecl::SBig(x28_materials[i]);

  for (u32 i = 0; i < edgeCount; ++i)
    const_cast<CCollisionEdge*>(x3c_edges)[i].swapBig();

  for (u32 i = 0; i < polyCount; ++i)
    const_cast<u16*>(x44_polyEdges)[i] = hecl::SBig(x44_polyEdges[i]);

  for (u32 i = 0; i < vertCount * 3; ++i)
    const_cast<float*>(x4c_verts)[i] = hecl::SBig(x4c_verts[i]);
}

std::unique_ptr<CAreaOctTree> CAreaOctTree::MakeFromMemory(const u8* buf, unsigned int size) {
  athena::io::MemoryReader r(buf + 8, size - 8);
  r.readUint32Big();
  r.readUint32Big();
  zeus::CAABox aabb;
  aabb.readBoundingBoxBig(r);
  Node::ETreeType nodeType = Node::ETreeType(r.readUint32Big());
  u32 treeSize = r.readUint32Big();
  const u8* cur = reinterpret_cast<const u8*>(buf) + 8 + r.position();

  const u8* treeBuf = cur;
  cur += treeSize;

  u32 matCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const u32* matBuf = reinterpret_cast<const u32*>(cur);
  cur += 4 * matCount;

  u32 vertMatsCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const u8* vertMatsBuf = cur;
  cur += vertMatsCount;

  u32 edgeMatsCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const u8* edgeMatsBuf = cur;
  cur += edgeMatsCount;

  u32 polyMatsCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const u8* polyMatsBuf = cur;
  cur += polyMatsCount;

  u32 edgeCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const CCollisionEdge* edgeBuf = reinterpret_cast<const CCollisionEdge*>(cur);
  cur += edgeCount * sizeof(edgeCount);

  u32 polyCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const u16* polyBuf = reinterpret_cast<const u16*>(cur);
  cur += polyCount * 2;

  u32 vertCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
  cur += 4;
  const float* vertBuf = reinterpret_cast<const float*>(cur);

  return std::make_unique<CAreaOctTree>(aabb, nodeType, reinterpret_cast<const u8*>(buf + 8), treeBuf, matCount, matBuf,
                                        vertMatsBuf, edgeMatsBuf, polyMatsBuf, edgeCount, edgeBuf, polyCount, polyBuf,
                                        vertCount, vertBuf);
}

CCollisionSurface CAreaOctTree::GetMasterListTriangle(u16 idx) const {
  const CCollisionEdge& e0 = x3c_edges[x44_polyEdges[idx * 3]];
  const CCollisionEdge& e1 = x3c_edges[x44_polyEdges[idx * 3 + 1]];
  u16 vert2 = e1.GetVertIndex2();
  if (e1.GetVertIndex1() != e0.GetVertIndex1())
    if (e1.GetVertIndex1() != e0.GetVertIndex2())
      vert2 = e1.GetVertIndex1();

  u32 material = x28_materials[x34_polyMats[idx]];
  if (material & 0x2000000)
    return CCollisionSurface(GetVert(e0.GetVertIndex2()), GetVert(e0.GetVertIndex1()), GetVert(vert2), material);
  else
    return CCollisionSurface(GetVert(e0.GetVertIndex1()), GetVert(e0.GetVertIndex2()), GetVert(vert2), material);
}

void CAreaOctTree::GetTriangleVertexIndices(u16 idx, u16 indicesOut[3]) const {
  const CCollisionEdge& e0 = x3c_edges[x44_polyEdges[idx * 3]];
  const CCollisionEdge& e1 = x3c_edges[x44_polyEdges[idx * 3 + 1]];
  indicesOut[2] = (e1.GetVertIndex1() != e0.GetVertIndex1() && e1.GetVertIndex1() != e0.GetVertIndex2())
                      ? e1.GetVertIndex1()
                      : e1.GetVertIndex2();

  u32 material = x28_materials[x34_polyMats[idx]];
  if (material & 0x2000000) {
    indicesOut[0] = e0.GetVertIndex2();
    indicesOut[1] = e0.GetVertIndex1();
  } else {
    indicesOut[0] = e0.GetVertIndex1();
    indicesOut[1] = e0.GetVertIndex2();
  }
}

} // namespace urde
