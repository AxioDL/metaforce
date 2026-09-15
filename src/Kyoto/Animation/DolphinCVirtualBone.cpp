#include "Kyoto/Animation/CPoseAsTransforms.hpp"
#include "Kyoto/Animation/CVirtualBone.hpp"
#include "dolphin/mtx.h"

#include "Kyoto/Basics/CBasics.hpp"

#include "rstl/math.hpp"

#include <string.h>

static rstl::reserved_vector< SSkinWeighting, 3 > StreamInSkinWeighting(CInputStream& in) {
  rstl::reserved_vector< SSkinWeighting, 3 > weights;
  const int weightCount = in.Get< int >();

  if (weightCount > weights.capacity()) {
    for (int i = 0; i < weights.capacity(); ++i) {
      weights.push_back(SSkinWeighting(in));
    }

    for (int i = weights.capacity(); i < weightCount; ++i) {
      SSkinWeighting tmp(in);
    }
  } else {
    for (int i = 0; i < weightCount; ++i) {
      weights.push_back(SSkinWeighting(in));
    }
  }

  return weights;
}

CVirtualBone::CVirtualBone(CInputStream& in)
: x0_weights(StreamInSkinWeighting(in))
, x1c_vertexCount(in.ReadLong())
, x20_xf(CTransform4f::Identity())
, x50_rotation(CMatrix3f::Identity()) {}

static ConstMtxPtr TransformToMtx(const CTransform4f& xf) { return xf.GetCStyleMatrix(); }

#ifdef __MWERKS__
void TransformFromMatrixDelta(register CTransform4f* xf, register const CMatrix3f* rot,
                              register const CVector3f* point) {
  asm volatile {
    psq_l f0, 0(point), 0, 0;
    psq_l f1, 8(rot), 1, 0;
    psq_l f3, 20(rot), 1, 0;
    psq_l f4, 8(point), 1, 0;

    ps_merge00 f1, f1, f0;

    psq_l f5, 32(rot), 1, 0;

    ps_merge01 f3, f3, f0;

    psq_l f0, 0(rot), 0, 0;
    psq_l f2, 12(rot), 0, 0;
    ps_merge00 f5, f5, f4;

    psq_l f4, 24(rot), 0, 0;

    psq_st f0, 0(xf), 0, 0;
    psq_st f1, 8(xf), 0, 0;
    psq_st f2, 16(xf), 0, 0;
    psq_st f3, 24(xf), 0, 0;
    psq_st f4, 32(xf), 0, 0;
    psq_st f5, 40(xf), 0, 0;
  }
}
#else
void TransformFromMatrixDelta(CTransform4f* xf, const CMatrix3f* rot, const CVector3f* point) {
  *xf = CTransform4f(*rot, *point);
}
#endif

#ifdef __MWERKS__
void Transform2FromMatrixData(register CTransform4f* xf, register const CMatrix3f* rot,
                              register const CVector3f* point, register float weight0,
                              register const CMatrix3f* rotation1, register const CVector3f* point1,
                              register float weight1) {
  __asm__ {
    fmr f4, weight0;
    psq_l f0, CMatrix3f.m00(rot), 0, 0;
    psq_l f5, CVector3f.mX(point), 0, 0;
    psq_l f1, CMatrix3f.m02(rot), 1, 0;
    ps_merge00 f7, f4, f2;
    psq_l f3, CMatrix3f.m12(rot), 1, 0;
    ps_merge00 f1, f1, f5;
    psq_l f2, CMatrix3f.m10(rot), 0, 0;
    ps_merge01 f3, f3, f5;
    psq_l f4, CMatrix3f.m20(rot), 0, 0;
    ps_muls0 f0, f0, f7;
    psq_l f6, CMatrix3f.m00(rotation1), 0, 0;
    psq_l f8, CVector3f.mX(point1), 0, 0;
    ps_muls0 f1, f1, f7;
    psq_l f9, CMatrix3f.m02(rotation1), 1, 0;
    ps_muls0 f2, f2, f7;
    ps_madds1 f0, f6, f7, f0;
    psq_l f11, CMatrix3f.m12(rotation1), 1, 0;
    ps_merge00 f9, f9, f8;
    psq_l f10, CMatrix3f.m10(rotation1), 0, 0;
    psq_l f6, CVector3f.mZ(point), 1, 0;
    ps_merge01 f11, f11, f8;
    psq_l f5, CMatrix3f.m22(rot), 1, 0;
    ps_muls0 f3, f3, f7;
    ps_madds1 f1, f9, f7, f1;
    psq_st f0, CTransform4f.m00(xf), 0, 0;
    ps_merge00 f5, f5, f6;
    ps_madds1 f2, f10, f7, f2;
    psq_l f6, CMatrix3f.m20(rotation1), 0, 0;
    ps_muls0 f4, f4, f7;
    psq_l f9, CMatrix3f.m22(rotation1), 1, 0;
    ps_madds1 f3, f11, f7, f3;
    psq_l f0, CVector3f.mZ(point1), 1, 0;
    ps_muls0 f5, f5, f7;
    psq_st f1, CTransform4f.m02(xf), 0, 0;
    ps_merge00 f9, f9, f0;
    ps_madds1 f4, f6, f7, f4;
    psq_st f2, CTransform4f.m10(xf), 0, 0;
    ps_madds1 f5, f9, f7, f5;
    psq_st f3, CTransform4f.m12(xf), 0, 0;
    psq_st f4, CTransform4f.m20(xf), 0, 0;
    psq_st f5, CTransform4f.m22(xf), 0, 0;
  }
}
#else
void Transform2FromMatrixData(CTransform4f* xf, const CMatrix3f* rot, const CVector3f* point,
                              float weight0, const CMatrix3f* rotation1, const CVector3f* point1,
                              float weight1) {
  const CMatrix3f rotation(*rot, weight0, *rotation1, weight1);
  *xf = CTransform4f(rotation, *point * weight0 + *point1 * weight1);
}
#endif

void CVirtualBone::BuildFinalPosMatrix(const CPoseAsTransforms& pose,
                                       const CVector3f* points) const {
  switch (x0_weights.size()) {
  case 1: {
    const CSegId id = x0_weights[0].x0_id;
    const CMatrix3f& rotation = pose.GetTransformMinusOffset(id);
    TransformFromMatrixDelta(&x20_xf, &rotation, &points[id.val()]);
    break;
  }
  case 2: {
    const CSegId& id0 = x0_weights[0].x0_id;
    const float weight0 = x0_weights[0].x4_weight;
    const CSegId& id1 = x0_weights[1].x0_id;
    const float weight1 = x0_weights[1].x4_weight;
    const CMatrix3f& rotation0 = pose.GetTransformMinusOffset(id0);
    const CMatrix3f& rotation1 = pose.GetTransformMinusOffset(id1);
    Transform2FromMatrixData(&x20_xf, &rotation0, &points[id0.val()], weight0, &rotation1,
                             &points[id1.val()], weight1);
    break;
  }
  case 3: {
    const CSegId& id0 = x0_weights[0].x0_id;
    const float weight0 = x0_weights[0].x4_weight;
    const CSegId& id1 = x0_weights[1].x0_id;
    const float weight1 = x0_weights[1].x4_weight;
    const CSegId& id2 = x0_weights[2].x0_id;
    const float weight2 = x0_weights[2].x4_weight;
    const CMatrix3f& rotation0 = pose.GetTransformMinusOffset(id0);
    const CMatrix3f& rotation1 = pose.GetTransformMinusOffset(id1);
    CMatrix3f rotation(rotation0, weight0, rotation1, weight1);
    CVector3f offset = weight0 * points[id0.val()] + weight1 * points[id1.val()];
    pose.AccumulateScaledTransform(id2, rotation, weight2);
    offset += weight2 * points[id2.val()];
    x20_xf = CTransform4f(rotation, offset);
    break;
  }
  default:
    x20_xf = CTransform4f::Identity();
    break;
  }
}

void CVirtualBone::BuildAccumulatedTransform(const CPoseAsTransforms& pose,
                                             const CVector3f* points) const {
  BuildFinalPosMatrix(pose, points);
  x50_rotation = pose.GetRotation(x0_weights[0].x0_id);
}

void PSMTXROMultS16VecArrayGathered(ROMtx mtx, const ushort* in, volatile void* out,
                                    size_t pointCount);

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void PSMTXROMultS16VecArrayGatheredSingle(ROMtx mtx, const ushort* in, volatile void* out,
                                          size_t pointCount);
#endif

#ifndef __MWERKS__
// The gathered routine loads serialized float vertices despite its historical S16 name.
static CVector3f ReadVertex(const ushort* in, size_t index) {
  float values[3];
  memcpy(values, reinterpret_cast< const uchar* >(in) + index * sizeof(values), sizeof(values));
  return CVector3f(CBasics::SwapBytes(values[0]), CBasics::SwapBytes(values[1]),
                   CBasics::SwapBytes(values[2]));
}
#endif

#ifdef __MWERKS__
void CVirtualBone::BuildPoints(const ushort* in, volatile void* out, int pointCount) const {
  if (pointCount < 3) {
    float* outF = const_cast< float* >(static_cast< volatile float* >(out));
    const CVector3f* inV = reinterpret_cast< const CVector3f* >(in);
    for (int i = 0; i < pointCount; ++i) {
      CVector3f point = x20_xf * inV[i];
      *outF = point.GetX();
      *outF = point.GetY();
      *outF = point.GetZ();
    }
  } else {
    ROMtx mtx;
    PSMTXReorder(TransformToMtx(x20_xf), mtx);
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    PSMTXROMultS16VecArrayGatheredSingle(mtx, in, out, pointCount);
#else
    PSMTXROMultS16VecArrayGathered(mtx, in, out, pointCount);
#endif
  }
}
#else
void CVirtualBone::BuildPoints(const ushort* in, volatile void* out, int pointCount) const {
  volatile float* dest = static_cast< volatile float* >(out);
  for (int i = 0; i < pointCount; ++i) {
    const CVector3f value = x20_xf * ReadVertex(in, i);
    *dest++ = value.GetX();
    *dest++ = value.GetY();
    *dest++ = value.GetZ();
  }
}
#endif

#ifdef __MWERKS__
void CVirtualBone::BuildNormals(const ushort* in, volatile void* out, int normalCount) const {
  if (normalCount < 3) {
    float* outF = const_cast< float* >(static_cast< volatile float* >(out));
    const CVector3f* inV = reinterpret_cast< const CVector3f* >(in);
    for (int i = 0; i < normalCount; ++i) {
      CVector3f normal = x50_rotation * inV[i];
      *outF = normal.GetX();
      *outF = normal.GetY();
      *outF = normal.GetZ();
    }
  } else {
    CTransform4f xf(x50_rotation, CVector3f(0.f, 0.f, 0.f));
    ROMtx mtx;
    PSMTXReorder(TransformToMtx(xf), mtx);
    PSMTXROMultS16VecArrayGathered(mtx, in, out, normalCount);
  }
}
#else
void CVirtualBone::BuildNormals(const ushort* in, volatile void* out, int normalCount) const {
  volatile float* dest = static_cast< volatile float* >(out);
  for (int i = 0; i < normalCount; ++i) {
    const CVector3f value = x50_rotation * ReadVertex(in, i);
    *dest++ = value.GetX();
    *dest++ = value.GetY();
    *dest++ = value.GetZ();
  }
}
#endif

void CVirtualBone::BuildNormals(const CVector3f* in, CVector3f* out, int normalCount) const {
  for (int i = 0; i < normalCount; ++i) {
    out[i] = x50_rotation * in[i];
  }
}

// clang-format off
#ifdef __MWERKS__
asm void PSMTXROMultS16VecArrayGathered(ROMtx mtx, const ushort* in, volatile void* out,
                                      size_t pointCount) {
  nofralloc
  xor r11, r11, r11
  addi r11, r11, 0x60
  stwu r1, -0x40(r1)
  stfd f14, 0x8(r1)
  subi r7, r6, 0x1
  stfd f15, 0x10(r1)
  srwi r7, r7, 1
  stfd f16, 0x18(r1)
  stfd f17, 0x20(r1)
  stfd f18, 0x28(r1)
  mtctr r7
  psq_l f0, 0x0(r3), 0, 0
  subi r4, r4, 0x8
  psq_l f1, 0x8(r3), 1, 0
  psq_l f6, 0x24(r3), 0, 0
  psq_lu f8, 0x8(r4), 0, 0
  psq_l f7, 0x2c(r3), 1, 0
  psq_lu f9, 0x8(r4), 0, 0
  ps_madds0 f11, f0, f8, f6
  psq_l f2, 0xc(r3), 0, 0
  ps_madds0 f12, f1, f8, f7
  psq_l f3, 0x14(r3), 1, 0
  ps_madds1 f13, f0, f9, f6
  psq_lu f10, 0x8(r4), 0, 0
  ps_madds1 f14, f1, f9, f7
  psq_l f5, 0x20(r3), 1, 0
  ps_madds1 f11, f2, f8, f11
  ps_madds1 f12, f3, f8, f12
  psq_l f4, 0x18(r3), 0, 0
  ps_madds0 f13, f2, f10, f13
  psq_lu f8, 0x8(r4), 0, 0
  ps_madds0 f14, f3, f10, f14
  ps_madds0 f15, f4, f9, f11
  ps_madds0 f16, f5, f9, f12
  psq_lu f9, 0x8(r4), 0, 0
  ps_madds1 f17, f4, f10, f13
  ps_madds1 f18, f5, f10, f14
  psq_lu f10, 0x8(r4), 0, 0
loop:
  ps_madds0 f11, f0, f8, f6
  psq_st f15, 0x0(r5), 0, 0
  ps_madds0 f12, f1, f8, f7
  psq_st f16, 0x0(r5), 1, 0
  ps_madds1 f13, f0, f9, f6
  psq_st f17, 0x0(r5), 0, 0
  ps_madds1 f14, f1, f9, f7
  psq_st f18, 0x0(r5), 1, 0
  ps_madds1 f11, f2, f8, f11
  ps_madds1 f12, f3, f8, f12
  psq_lu f8, 0x8(r4), 0, 0
  ps_madds0 f13, f2, f10, f13
  ps_madds0 f14, f3, f10, f14
  ps_madds0 f15, f4, f9, f11
  ps_madds0 f16, f5, f9, f12
  psq_lu f9, 0x8(r4), 0, 0
  ps_madds1 f17, f4, f10, f13
  ps_madds1 f18, f5, f10, f14
  psq_lu f10, 0x8(r4), 0, 0
  bdnz loop
  psq_st f15, 0x0(r5), 0, 0
  clrlwi. r7, r6, 31
  psq_st f16, 0x0(r5), 1, 0
  bne done
  psq_st f17, 0x0(r5), 0, 0
  psq_st f18, 0x0(r5), 1, 0
done:
  lfd f14, 0x8(r1)
  lfd f15, 0x10(r1)
  lfd f16, 0x18(r1)
  lfd f17, 0x20(r1)
  lfd f18, 0x28(r1)
  addi r1, r1, 0x40
  blr
}
#else
void PSMTXROMultS16VecArrayGathered(ROMtx mtx, const ushort* in, volatile void* out,
                                     size_t pointCount) {
  volatile float* dest = static_cast< volatile float* >(out);
  for (size_t i = 0; i < pointCount; ++i) {
    const CVector3f value = ReadVertex(in, i);
    for (int j = 0; j < 3; ++j) {
      *dest++ = mtx[0][j] * value.GetX() + mtx[1][j] * value.GetY() +
                mtx[2][j] * value.GetZ() + mtx[3][j];
    }
  }
}
#endif
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
#ifdef __MWERKS__
// PAL point skinning writes each float separately, including the original FIFO pacing.
asm void PSMTXROMultS16VecArrayGatheredSingle(ROMtx mtx, const ushort* in, volatile void* out,
                                                size_t pointCount) {
  nofralloc
  xor r11, r11, r11
  addi r11, r11, 0x60
  stwu r1, -0x48(r1)
  stfd f14, 0x8(r1)
  subi r7, r6, 0x1
  stfd f15, 0x10(r1)
  srwi r7, r7, 1
  stfd f16, 0x18(r1)
  stfd f17, 0x20(r1)
  stfd f18, 0x28(r1)
  stfd f19, 0x30(r1)
  mtctr r7
  psq_l f0, 0x0(r3), 0, 0
  subi r4, r4, 0x8
  psq_l f1, 0x8(r3), 1, 0
  psq_l f6, 0x24(r3), 0, 0
  psq_lu f8, 0x8(r4), 0, 0
  psq_l f7, 0x2c(r3), 1, 0
  psq_lu f9, 0x8(r4), 0, 0
  ps_madds0 f11, f0, f8, f6
  psq_l f2, 0xc(r3), 0, 0
  ps_madds0 f12, f1, f8, f7
  psq_l f3, 0x14(r3), 1, 0
  ps_madds1 f13, f0, f9, f6
  psq_lu f10, 0x8(r4), 0, 0
  ps_madds1 f14, f1, f9, f7
  psq_l f5, 0x20(r3), 1, 0
  ps_madds1 f11, f2, f8, f11
  ps_madds1 f12, f3, f8, f12
  psq_l f4, 0x18(r3), 0, 0
  ps_madds0 f13, f2, f10, f13
  psq_lu f8, 0x8(r4), 0, 0
  ps_madds0 f14, f3, f10, f14
  ps_madds0 f15, f4, f9, f11
  ps_madds0 f16, f5, f9, f12
  psq_lu f9, 0x8(r4), 0, 0
  ps_madds1 f17, f4, f10, f13
  ps_madds1 f18, f5, f10, f14
  psq_lu f10, 0x8(r4), 0, 0
  nop
  nop
  nop
  nop
  nop
  nop
loop:
  ps_merge10 f19, f15, f15
  psq_st f15, 0x0(r5), 1, 0
  ps_madds0 f11, f0, f8, f6
  psq_st f19, 0x0(r5), 1, 0
  ps_madds0 f12, f1, f8, f7
  psq_st f16, 0x0(r5), 1, 0
  ps_merge10 f19, f17, f17
  nop
  nop
  nop
  nop
  nop
  nop
  psq_st f17, 0x0(r5), 1, 0
  ps_madds1 f13, f0, f9, f6
  psq_st f19, 0x0(r5), 1, 0
  ps_madds1 f14, f1, f9, f7
  psq_st f18, 0x0(r5), 1, 0
  ps_madds1 f11, f2, f8, f11
  ps_madds1 f12, f3, f8, f12
  psq_lu f8, 0x8(r4), 0, 0
  ps_madds0 f13, f2, f10, f13
  ps_madds0 f14, f3, f10, f14
  ps_madds0 f15, f4, f9, f11
  ps_madds0 f16, f5, f9, f12
  psq_lu f9, 0x8(r4), 0, 0
  ps_madds1 f17, f4, f10, f13
  ps_madds1 f18, f5, f10, f14
  psq_lu f10, 0x8(r4), 0, 0
  nop
  nop
  nop
  nop
  nop
  nop
  bdnz loop
  ps_merge10 f19, f15, f15
  psq_st f15, 0x0(r5), 1, 0
  psq_st f19, 0x0(r5), 1, 0
  clrlwi. r7, r6, 31
  psq_st f16, 0x0(r5), 1, 0
  bne done
  ps_merge10 f19, f17, f17
  psq_st f17, 0x0(r5), 1, 0
  psq_st f19, 0x0(r5), 1, 0
  psq_st f18, 0x0(r5), 1, 0
done:
  lfd f14, 0x8(r1)
  lfd f15, 0x10(r1)
  lfd f16, 0x18(r1)
  lfd f17, 0x20(r1)
  lfd f18, 0x28(r1)
  lfd f19, 0x30(r1)
  addi r1, r1, 0x48
  blr
}
#else
void PSMTXROMultS16VecArrayGatheredSingle(ROMtx mtx, const ushort* in, volatile void* out,
                                            size_t pointCount) {
  PSMTXROMultS16VecArrayGathered(mtx, in, out, pointCount);
}
#endif
#endif

// clang-format on
