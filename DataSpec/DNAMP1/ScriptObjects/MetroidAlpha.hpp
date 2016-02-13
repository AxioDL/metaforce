#ifndef _DNAMP1_METROIDALPHA_HPP_
#define _DNAMP1_METROIDALPHA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct MetroidAlpha : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    AnimationParameters animationParameters1;
    AnimationParameters animationParameters2;
    AnimationParameters animationParameters3;
    AnimationParameters animationParameters4;
    Value<bool> unknown8;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
        animationParameters1.nameANCS(pakRouter, name + "_animp1");
        animationParameters2.nameANCS(pakRouter, name + "_animp2");
        animationParameters3.nameANCS(pakRouter, name + "_animp3");
        animationParameters4.nameANCS(pakRouter, name + "_animp4");
    }
};
}
}

#endif
