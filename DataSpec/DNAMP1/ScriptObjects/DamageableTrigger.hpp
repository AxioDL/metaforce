#ifndef _DNAMP1_DAMAGABLETRIGGER_HPP_
#define _DNAMP1_DAMAGABLETRIGGER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
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

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (texture1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture1);
            ent->name = name + "_texture1";
        }
        if (texture2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture2);
            ent->name = name + "_texture2";
        }
        if (texture3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture3);
            ent->name = name + "_texture3";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(texture1, pathsOut);
        g_curSpec->flattenDependencies(texture2, pathsOut);
        g_curSpec->flattenDependencies(texture3, pathsOut);
    }
};
}
}

#endif
