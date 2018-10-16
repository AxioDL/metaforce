#pragma once

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
    UniqueID32 deathParticle1;
    Value<atUint32> deathParticle1Frames;
    UniqueID32 deathParticle2;
    Value<atUint32> deathParticle2Frames;
    UniqueID32 deathParticle3;
    Value<atUint32> deathParticle3Frames;
    UniqueID32 deathParticle4;
    Value<atUint32> deathParticle4Frames;
    Value<atUint32> deathSFX;
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
        if (deathParticle1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle1);
            ent->name = name + "_deathParticle1";
        }
        if (deathParticle2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle2);
            ent->name = name + "_deathParticle2";
        }
        if (deathParticle3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle3);
            ent->name = name + "_deathParticle3";
        }
        if (deathParticle4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle4);
            ent->name = name + "_deathParticle4";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(model, pathsOut);
        animationParameters.depANCS(pathsOut);
        g_curSpec->flattenDependencies(deathParticle1, pathsOut);
        g_curSpec->flattenDependencies(deathParticle2, pathsOut);
        g_curSpec->flattenDependencies(deathParticle3, pathsOut);
        g_curSpec->flattenDependencies(deathParticle4, pathsOut);
    }
};
}

