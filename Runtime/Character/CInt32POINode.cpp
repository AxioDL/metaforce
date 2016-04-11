#include "CInt32POINode.hpp"

namespace urde
{

CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in), x38_val(in.readUint32Big()), x3c_boneName(in.readString()) {}

}
