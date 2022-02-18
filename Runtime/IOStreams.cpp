#include "Runtime/IOStreams.hpp"
#include <zeus/zeus.hpp>

namespace metaforce {
template <>
zeus::CVector2f cinput_stream_helper(CInputStream& in) {
  zeus::CVector2f ret;
  ret.x() = in.ReadFloat();
  ret.y() = in.ReadFloat();
  return ret;
}
template <>
zeus::CVector3f cinput_stream_helper(CInputStream& in) {
  zeus::CVector3f ret;
  ret.x() = in.ReadFloat();
  ret.y() = in.ReadFloat();
  ret.z() = in.ReadFloat();
  return ret;
}
template <>
zeus::CVector4f cinput_stream_helper(CInputStream& in) {
  zeus::CVector4f ret;
  ret.x() = in.ReadFloat();
  ret.y() = in.ReadFloat();
  ret.z() = in.ReadFloat();
  ret.w() = in.ReadFloat();
  return ret;
}

template <>
zeus::CQuaternion cinput_stream_helper(CInputStream& in) {
  zeus::CQuaternion ret;
  ret.w() = in.ReadFloat();
  ret.x() = in.ReadFloat();
  ret.y() = in.ReadFloat();
  ret.z() = in.ReadFloat();
  return ret;
}

template <>
zeus::CAABox cinput_stream_helper(CInputStream& in) {
  zeus::CAABox ret;
  ret.min = in.Get<zeus::CVector3f>();
  ret.max = in.Get<zeus::CVector3f>();
  return ret;
}


template <>
zeus::COBBox cinput_stream_helper(CInputStream& in) {
  zeus::COBBox ret;
  ret.transform = in.Get<zeus::CTransform>();
  ret.extents = in.Get<zeus::CVector3f>();
  return ret;
}
template <>
zeus::CColor cinput_stream_helper(CInputStream& in) {
  zeus::CColor ret;
  ret.r() = in.ReadFloat();
  ret.g() = in.ReadFloat();
  ret.b() = in.ReadFloat();
  ret.a() = in.ReadFloat();
  return ret;
}

template <>
zeus::CTransform cinput_stream_helper(CInputStream& in) {
  zeus::CTransform ret;
  auto r0 = in.Get<zeus::CVector4f>();
  auto r1 = in.Get<zeus::CVector4f>();
  auto r2 = in.Get<zeus::CVector4f>();
  ret.basis = zeus::CMatrix3f(r0, r1, r2);
  ret.basis.transpose();
  ret.origin = zeus::CVector3f(r0.w(), r1.w(), r2.w());
  return ret;
}

template <>
zeus::CMatrix3f cinput_stream_helper(CInputStream& in) {
  zeus::CMatrix3f ret;
  ret.m[0] = in.Get<zeus::CVector3f>();
  ret.m[1] = in.Get<zeus::CVector3f>();
  ret.m[2] = in.Get<zeus::CVector3f>();
  return ret.transposed();
}

template <>
zeus::CMatrix4f cinput_stream_helper(CInputStream& in) {
  zeus::CMatrix4f ret;
  ret.m[0] = in.Get<zeus::CVector4f>();
  ret.m[1] = in.Get<zeus::CVector4f>();
  ret.m[2] = in.Get<zeus::CVector4f>();
  ret.m[3] = in.Get<zeus::CVector4f>();
  return ret.transposed();
}
} // namespace metaforce
