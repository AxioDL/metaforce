#include "CCollisionEdge.hpp"

namespace urde {
CCollisionEdge::CCollisionEdge(CInputStream& in) {
  x0_index1 = in.readUint16Big();
  x2_index2 = in.readUint16Big();
}
} // namespace urde
