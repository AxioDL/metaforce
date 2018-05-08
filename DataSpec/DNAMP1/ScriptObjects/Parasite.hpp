#ifndef _DNAMP1_PARASITE_HPP_
#define _DNAMP1_PARASITE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Parasite : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<bool> unknown19;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

#endif
