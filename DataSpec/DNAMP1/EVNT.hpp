#ifndef __DNAMP1_EVNT_HPP__
#define __DNAMP1_EVNT_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

struct EVNT : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    Value<atUint32> version;

    struct POINode : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint16> unk0;
        String<-1> name;
        Value<atUint16> type;
        struct CharAnimTime : BigDNA
        {
            enum class Type : atUint32
            {
                NonZero,
                ZeroIncreasing,
                ZeroSteady,
                ZeroDecreasing,
                Infinity
            };

            AT_DECL_DNA_YAML
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
        AT_DECL_DNA_YAML
        Value<atUint8> value;
    };
    Value<atUint32> boolPOICount;
    Vector<BoolPOINode, AT_DNA_COUNT(boolPOICount)> boolPOINodes;

    struct Int32POINode : POINode
    {
        AT_DECL_DNA_YAML
        Value<atUint32> value;
        String<-1> locator;
    };
    Value<atUint32> int32POICount;
    Vector<Int32POINode, AT_DNA_COUNT(int32POICount)> int32POINodes;

    struct ParticlePOINode : POINode
    {
        AT_DECL_DNA_YAML
        Value<atUint32> duration;
        DNAFourCC ptype;
        UniqueID32 id;
        String<-1> locator;
        Value<float> scale;
        Value<atUint32> parentMode;
    };
    Value<atUint32> particlePOICount;
    Vector<ParticlePOINode, AT_DNA_COUNT(particlePOICount)> particlePOINodes;

    struct SoundPOINode : POINode
    {
        AT_DECL_DNA_YAML
        Value<atUint32> soundId;
        Value<float> falloff;
        Value<float> maxDist;
    };
    Value<atUint32> soundPOICount;
    Vector<SoundPOINode, AT_DNA_COUNT(soundPOICount)> soundPOINodes;

    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        EVNT evnt;
        evnt.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        athena::io::ToYAMLStream(evnt, writer);
        return true;
    }

    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
    {
        EVNT evnt;
        athena::io::FileReader reader(inPath.getAbsolutePath());
        athena::io::FromYAMLStream(evnt, reader);
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

#endif // __DNAMP1_EVNT_HPP__
