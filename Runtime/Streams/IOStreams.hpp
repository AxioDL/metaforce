#pragma once

#include "Runtime/Streams/CMemoryInStream.hpp"
#include "Runtime/Streams/CMemoryStreamOut.hpp"
#include "Runtime/Streams/CZipInputStream.hpp"
#include "Runtime/Streams/ContainerReaders.hpp"
#include "Runtime/rstl.hpp"

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
} // namespace zeus
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

template <>
void coutput_stream_helper(const zeus::CVector3f& v, COutputStream& out);

} // namespace metaforce