#ifndef _DNAMP1_FISHCLOUD_HPP_
#define _DNAMP1_FISHCLOUD_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct FishCloud : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> volume;
    Value<bool> unknown1;
    UniqueID32 model;
    AnimationParameters animationParameters;
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
    Value<atUint32> unknown16;
    Value<atVec4f> unknown17; // CColor
    Value<bool> unknown18;
    Value<float> unknown19;
    Value<atUint32> unknown20;
    Value<atUint32> unknown21;
    Value<atUint32> unknown22;
    Value<atUint32> unknown23;
    Value<atUint32> unknown24;
    Value<atUint32> unknown25;
    Value<atUint32> unknown26;
    Value<atUint32> unknown27;
    Value<atUint32> unknown28;
    Value<bool> unknown29;
    Value<bool> unknown30;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_model";
        }
        animationParameters.nameANCS(pakRouter, name + "_animp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(model, pathsOut);
        animationParameters.depANCS(pathsOut);
    }
};
}

#endif
