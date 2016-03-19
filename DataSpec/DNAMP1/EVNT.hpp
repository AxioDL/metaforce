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
            DECL_YAML
            Value<float> time;
            Value<atUint32> unk1;
        };

        CharAnimTime animTime;
        Value<atUint32> idx;
        Value<bool> unk2;
        Value<float> unk3;
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
        String<-1> locatorName;
    };
    std::vector<Int32POINode> int32POINodes;

    struct ParticlePOINode : POINode
    {
        DECL_YAML
        Value<atUint32> duration;
        DNAFourCC effectType;
        UniqueID32 effectId;
        String<-1> locatorName;
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
        FILE* fp = hecl::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
        evnt.toYAMLFile(fp);
        fclose(fp);
        return true;
    }
};

}
}

#endif // __DNAMP1_EVNT_HPP__
