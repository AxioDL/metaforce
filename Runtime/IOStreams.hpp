#pragma once

#include "Runtime/CMemoryInStream.hpp"
#include "Runtime/CMemoryStreamOut.hpp"
#include "Runtime/CZipInputStream.hpp"

namespace zeus {
class CVector2f;
class CVector3f;
class CVector4f;
class CTransform;
class CMatrix3f;
class CMatrix4f;
class CAABox;
class COBBox;
class CQuaternion;
class CColor;
}
namespace metaforce {
// Custom helpers for input/output
template <>
zeus::CVector2f cinput_stream_helper(CInputStream& in);
template <>
zeus::CVector3f cinput_stream_helper(CInputStream& in);
template <>
zeus::CVector4f cinput_stream_helper(CInputStream& in);
template <>
zeus::CQuaternion cinput_stream_helper(CInputStream& in);
template <>
zeus::CAABox cinput_stream_helper(CInputStream& in);
template <>
zeus::COBBox cinput_stream_helper(CInputStream& in);
template <>
zeus::CColor cinput_stream_helper(CInputStream& in);
template <>
zeus::CTransform cinput_stream_helper(CInputStream& in);
template <>
zeus::CMatrix3f cinput_stream_helper(CInputStream& in);
template <>
zeus::CMatrix4f cinput_stream_helper(CInputStream& in);
}