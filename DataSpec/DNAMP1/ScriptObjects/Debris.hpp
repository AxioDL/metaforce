#ifndef _DNAMP1_DEBRIS_HPP_
#define _DNAMP1_DEBRIS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Debris : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<float> unknown1;
    Value<atVec3f> unknown2;
    Value<atVec4f> unknown3; // CColor
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    Value<bool> unknown8;
    UniqueID32 model;
    ActorParameters actorParameters;
    UniqueID32 particle;
    Value<atVec3f> unknown9;
    Value<bool> unknown10;
    Value<bool> unknown11;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_model";
        }
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(model, pathsOut);
        g_curSpec->flattenDependencies(particle, pathsOut);
        actorParameters.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

#endif
