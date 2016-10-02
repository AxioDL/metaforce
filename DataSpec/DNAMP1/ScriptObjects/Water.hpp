#ifndef _DNAMP1_WATER_HPP_
#define _DNAMP1_WATER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Water : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    DamageInfo damageInfo;
    Value<atVec3f> unknown1;
    Value<atUint32> unknown2;
    Value<bool> unknown3;
    Value<bool> unknown4;
    UniqueID32 texture1;
    UniqueID32 texture2;
    UniqueID32 texture3;
    UniqueID32 texture4;
    UniqueID32 texture5;
    UniqueID32 texture6;
    Value<atVec3f> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<bool> unknown9;
    Value<atUint32> unknown10;
    Value<bool> unknown11;
    Value<float> unknown12;
    struct FluidUVMotion : BigYAML
    {
        DECL_YAML
        struct FluidLayerMotion : BigYAML
        {
            DECL_YAML
            Value<atUint32> unknown1;
            Value<float> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
            Value<float> unknown5;
        };

        /* BIG FAT WARNING: Do NOT re-order these, even if they seem incorrect */
        FluidLayerMotion layer2;
        FluidLayerMotion layer3;
        FluidLayerMotion layer1;
        Value<float> unknown1;
        Value<float> unknown2;
    } fluidUVMotion;

    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<float> unknown19;
    Value<float> unknown20;
    Value<atVec4f> unknown21;
    Value<atVec4f> unknown22;
    UniqueID32 particle1;
    UniqueID32 particle2;
    UniqueID32 particle3;
    UniqueID32 particle4;
    UniqueID32 particle5;
    Value<atUint32> soundID1; // needs verification
    Value<atUint32> soundID2; // needs verification
    Value<atUint32> soundID3; // needs verification
    Value<atUint32> soundID4; // needs verification
    Value<atUint32> soundID5; // needs verification
    Value<float> unknown23;
    Value<atUint32> unknown24;
    Value<float> unknown25;
    Value<float> unknown26;
    Value<float> unknown27;
    Value<float> unknown28;
    Value<float> unknown29;
    Value<float> unknown30;
    Value<float> unknown31;
    Value<float> unknown32;
    Value<atVec4f> unknown33; // CColor
    UniqueID32 texture34;
    Value<float> unknown35;
    Value<float> unknown36;
    Value<float> unknown37;
    Value<atUint32> unknown38;
    Value<atUint32> unknown39;
    Value<bool> unknown40;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (texture1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture1);
            ent->name = name + "_tex1";
        }
        if (texture2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture2);
            ent->name = name + "_tex2";
        }
        if (texture3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture3);
            ent->name = name + "_tex3";
        }
        if (texture4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture4);
            ent->name = name + "_tex4";
        }
        if (texture5)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture5);
            ent->name = name + "_tex5";
        }
        if (texture6)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture6);
            ent->name = name + "_tex6";
        }
        if (texture34)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture34);
            ent->name = name + "_tex34";
        }
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
        if (particle3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle3);
            ent->name = name + "_part3";
        }
        if (particle4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle4);
            ent->name = name + "_part4";
        }
        if (particle5)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle5);
            ent->name = name + "_part5";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(texture1, pathsOut);
        g_curSpec->flattenDependencies(texture2, pathsOut);
        g_curSpec->flattenDependencies(texture3, pathsOut);
        g_curSpec->flattenDependencies(texture4, pathsOut);
        g_curSpec->flattenDependencies(texture5, pathsOut);
        g_curSpec->flattenDependencies(texture6, pathsOut);
        g_curSpec->flattenDependencies(texture34, pathsOut);
        g_curSpec->flattenDependencies(particle1, pathsOut);
        g_curSpec->flattenDependencies(particle2, pathsOut);
        g_curSpec->flattenDependencies(particle3, pathsOut);
        g_curSpec->flattenDependencies(particle4, pathsOut);
        g_curSpec->flattenDependencies(particle5, pathsOut);
    }
};
}
}

#endif
