#ifndef _DNAMP1_DAMAGABLETRIGGER_HPP_
#define _DNAMP1_DAMAGABLETRIGGER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct DamageableTrigger : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
    Value<atUint32> unknown1;
    UniqueID32 texture1;
    UniqueID32 texture2;
    UniqueID32 texture3;
    Value<bool> lockOn;
    Value<bool> active;
    VisorParameters visorParameters;
};
}
}

#endif
