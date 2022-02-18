#include "Runtime/Collision/CCollisionEdge.hpp"
#include "Runtime/CInputStream.hpp"

namespace metaforce {
CCollisionEdge::CCollisionEdge(CInputStream& in) {
  x0_index1 = in.ReadShort();
  x2_index2 = in.ReadShort();
}
} // namespace metaforce
