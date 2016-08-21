#include "CPOINode.hpp"

namespace urde
{

CPOINode::CPOINode(const std::string& name, EPOIType type, const CCharAnimTime& time,
                   u32 index, bool c, float weight, u32 e, u32 f)
: x4_(1),
  x8_name(name),
  x18_type(type),
  x1c_time(time),
  x24_index(index),
  x28_(c),
  x2c_weight(weight),
  x30_charIdx(e),
  x34_flags(f)
{}

CPOINode::CPOINode(CInputStream& in)
: x4_(in.readUint16Big()),
  x8_name(in.readString()),
  x18_type(EPOIType(in.readUint16Big())),
  x1c_time(in),
  x24_index(in.readUint32Big()),
  x28_(in.readBool()),
  x2c_weight(in.readFloatBig()),
  x30_charIdx(in.readUint32Big()),
  x34_flags(in.readUint32Big())
{}

}
