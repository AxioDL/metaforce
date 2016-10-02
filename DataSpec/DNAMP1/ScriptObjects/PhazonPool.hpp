#ifndef _DNAMP1_PHAZONPOOL_HPP_
#define _DNAMP1_PHAZONPOOL_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct PhazonPool : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<bool> unknown1;
    UniqueID32 model1;
    UniqueID32 model2;
    UniqueID32 particle1;
    UniqueID32 particle2;
    Value<atUint32> unknown2;
    DamageInfo damageInfo;
    Value<atVec3f> unknown3;
    Value<atUint32> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<bool> unknown8;
    Value<float> unknown9;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
            ent->name = name + "_part1";
        }
        if (particle2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
            ent->name = name + "_part2";
        }
        if (model1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model1);
            ent->name = name + "_model1";
        }
        if (model2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model2);
            ent->name = name + "_model2";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(particle1, pathsOut);
        g_curSpec->flattenDependencies(particle2, pathsOut);
        g_curSpec->flattenDependencies(model1, pathsOut);
        g_curSpec->flattenDependencies(model2, pathsOut);
    }
};
}
}

#endif
