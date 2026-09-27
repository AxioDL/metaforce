#include "Kyoto/Animation/CHalfTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CHalfTransition::CHalfTransition(CInputStream& in)
: mId(in.Get< uint >()), mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
