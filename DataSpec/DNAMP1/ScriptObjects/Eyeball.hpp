#ifndef _DNAMP1_EYEBALL_HPP_
#define _DNAMP1_EYEBALL_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Eyeball : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown2;
    Value<float> unknown3;
    UniqueID32 wpsc;
    DamageInfo damageInfo;
    UniqueID32 particle1;
    UniqueID32 particle2;
    UniqueID32 texture1;
    UniqueID32 texture2;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5; // always ff
    Value<atUint32> unknown6; // always ff
    Value<atUint32> unknown7; // always ff
    Value<atUint32> unknown8;
    Value<bool> unknown9;
};
}
}

#endif
