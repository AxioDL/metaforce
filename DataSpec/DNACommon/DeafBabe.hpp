#pragma once

#include <athena/Types.hpp>

namespace hecl::blender {
class PyOutStream;
struct ColMesh;
} // namespace hecl::blender

namespace DataSpec {

enum class BspNodeType : atUint32 { Invalid, Branch, Leaf };

template <class DEAFBABE>
void DeafBabeSendToBlender(hecl::blender::PyOutStream& os, const DEAFBABE& db, bool isDcln = false, atInt32 idx = -1);

template <class DEAFBABE>
void DeafBabeBuildFromBlender(DEAFBABE& db, const hecl::blender::ColMesh& colMesh);

} // namespace DataSpec
