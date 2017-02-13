#ifndef __DNAMP1_EVNT_HPP__
#define __DNAMP1_EVNT_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct EVNT : BigYAML
{
    DECL_YAML
    Delete expl;
    Value<atUint32> version;

    struct POINode : BigYAML
    {
        DECL_YAML
        Value<atUint16> unk0;
        String<-1> name;
        Value<atUint16> type;
        struct CharAnimTime : BigYAML
        {
            enum class Type : atUint32
            {
                NonZero,
                ZeroIncreasing,
                ZeroSteady,
                ZeroDecreasing,
                Infinity
            };

            DECL_YAML
            Value<float> time;
            Value<Type> type;
        };

        CharAnimTime animTime;
        Value<atUint32> idx;
        Value<bool> unk2;
        Value<float> weight;
        Value<atUint32> unk4;
        Value<atUint32> unk5;
    };

    struct BoolPOINode : POINode
    {
        DECL_YAML
        Value<atUint8> value;
    };
    std::vector<BoolPOINode> boolPOINodes;

    struct Int32POINode : POINode
    {
        DECL_YAML
        Value<atUint32> value;
        String<-1> locator;
    };
    std::vector<Int32POINode> int32POINodes;

    struct ParticlePOINode : POINode
    {
        DECL_YAML
        Value<atUint32> duration;
        DNAFourCC type;
        UniqueID32 id;
        String<-1> locator;
        Value<float> scale;
        Value<atUint32> parentMode;
    };
    std::vector<ParticlePOINode> particlePOINodes;

    struct SoundPOINode : POINode
    {
        DECL_YAML
        Value<atUint32> soundId;
        Value<float> falloff;
        Value<float> maxDist;
    };
    std::vector<SoundPOINode> soundPOINodes;

    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        EVNT evnt;
        evnt.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        evnt.toYAMLStream(writer);
        return true;
    }

    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
    {
        EVNT evnt;
        athena::io::FileReader reader(inPath.getAbsolutePath());
        evnt.fromYAMLStream(reader);
        athena::io::FileWriter ws(outPath.getAbsolutePath());
        evnt.write(ws);
        return true;
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        for (const ParticlePOINode& node : particlePOINodes)
            g_curSpec->flattenDependencies(node.id, pathsOut);
    }
};

}
}

#endif // __DNAMP1_EVNT_HPP__
