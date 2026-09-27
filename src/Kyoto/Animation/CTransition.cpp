#include "Kyoto/Animation/CTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CTransition::CTransition(CInputStream& in)
: mId(in.Get<uint>())
, mAnimA(in.Get<uint>())
, mAnimB(in.Get<uint>())
, mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
