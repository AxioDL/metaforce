#ifndef _DNAMP1_PLAYERHINT_HPP_
#define _DNAMP1_PLAYERHINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct PlayerHint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    PlayerHintParameters playerHintParameters;
    Value<atUint32> unknown2;
};
}

#endif
