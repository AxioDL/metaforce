#ifndef _DNAMP1_WORLDTELEPORTER_HPP_
#define _DNAMP1_WORLDTELEPORTER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{

struct WorldTeleporter : IScriptObject
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> unknown1;
    UniqueID32 mlvl;
    UniqueID32 mrea;
    AnimationParameters animationParameters;
    Value<atVec3f> unknown2;
    UniqueID32 model1;
    Value<atVec3f> unknown3;
    UniqueID32 model2;
    Value<atVec3f> unknown4;
    Value<bool> unknown5;
    Value<atUint32> soundID; // needs verifcation
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<bool> unknown8;
    UniqueID32 font;
    UniqueID32 strg;
    Value<bool> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    
    /* Trilogy additions (property count 26) */
    String<-1> audioStream;
    Value<bool> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
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
        if (strg)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(strg);
            ent->name = name + "_strg";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(model1, pathsOut);
        g_curSpec->flattenDependencies(model2, pathsOut);
        g_curSpec->flattenDependencies(strg, pathsOut);
    }
};

}

#endif
