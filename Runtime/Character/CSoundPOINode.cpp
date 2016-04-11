#include "CSoundPOINode.hpp"

namespace urde
{

CSoundPOINode::CSoundPOINode(CInputStream& in)
: CPOINode(in),
  x38_sfxId(in.readUint32Big()),
  x3c_falloff(in.readFloatBig()),
  x40_maxDist(in.readFloatBig())
{}

CSoundPOINode::CSoundPOINode(const std::string& name, u16 a,
                             const CCharAnimTime& time, u32 b, bool c,
                             float d, u32 e, u32 f, u32 sfxId, float falloff, float maxDist)
: CPOINode(name, a, time, b, c, d, e, f),
  x38_sfxId(sfxId), x3c_falloff(falloff), x40_maxDist(maxDist) {}

}
