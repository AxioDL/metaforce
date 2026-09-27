#include "Kyoto/Animation/CAnimation.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"

CAnimation::CAnimation(CInputStream& in)
: mName(in)
, mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {
  
}
