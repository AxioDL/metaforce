#include "CBoolPOINode.hpp"

namespace urde
{

CBoolPOINode::CBoolPOINode()
: CPOINode("root", 1, CCharAnimTime(), -1, false, 1.f, -1, 0) {}

CBoolPOINode::CBoolPOINode(CInputStream& in)
: CPOINode(in), x38_val(in.readBool()) {}

}
