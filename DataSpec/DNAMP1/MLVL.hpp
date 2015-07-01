#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

struct MLVL : public BigDNA
{
    DECL_DNA
    DNAFourCC magic;
    Value<atUint32> version;
    DNAUniqueID32 worldNameId;
    DNAUniqueID32 saveWorldId;
    DNAUniqueID32 worldSkyboxId;

    Value<atUint32> memRelayLinkCount;
    struct MemRelayLink : public BigDNA
    {
        DECL_DNA
        Value<atUint32> memRelayId;
        Value<atUint32> targetId;
        Value<atUint16> msg;
        Value<atUint8> unk;
    };
    Vector<MemRelayLink, DNA_COUNT(memRelayLinkCount)> memRelayLinks;

    Value<atUint32> areaCount;
    Value<atUint32> unknown1;
    struct Area : public BigDNA
    {
        DECL_DNA
        DNAUniqueID32 areaNameId;
        Value<atVec4f> transformMtx[3];
        Value<atVec3f> aabb[2];
        DNAUniqueID32 areaMREAId;
        Value<atUint32> areaId;

        Value<atUint32> attachedAreaCount;
        Vector<atUint16, DNA_COUNT(attachedAreaCount)> attachedAreas;
        Value<atUint32> padding;

        Value<atUint32> depCount;
        struct Dependency : public BigDNA
        {
            DECL_DNA
            DNAUniqueID32 id;
            DNAFourCC type;
        };
        Vector<Dependency, DNA_COUNT(depCount)> deps;

        Value<atUint32> depLayerCount;
        Vector<atUint32, DNA_COUNT(depLayerCount)> depLayers;

        Value<atUint32> dockCount;
        struct Dock : public BigDNA
        {
            DECL_DNA
            Value<atUint32> endpointCount;
            struct Endpoint : public BigDNA
            {
                DECL_DNA
                Value<atUint32> areaIdx;
                Value<atUint32> dockIdx;
            };
            DNAFourCC type;
            Vector<Endpoint, DNA_COUNT(endpointCount)> endpoints;

            Value<atUint32> planeVertCount;
            Vector<atVec3f, DNA_COUNT(planeVertCount)> planeVerts;
        };
        Vector<Dock, DNA_COUNT(dockCount)> docks;
    };

    DNAUniqueID32 worldMap;
    Value<atUint8> unknown2;
    Value<atUint32> unknown3;

    Value<atUint32> audioGroupCount;
    struct AudioGroup : public BigDNA
    {
        DECL_DNA
        Value<atUint32> unknown;
        DNAUniqueID32 agscId;
    };
    Vector<AudioGroup, DNA_COUNT(audioGroupCount)> audioGroups;
    String<-1> unkString;

    Value<atUint32> layerFlagCount;
    struct LayerFlags : public BigDNA
    {
        DECL_DNA
        Value<atUint32> layerCount;
        Value<atUint64> flags;
    };
    Vector<LayerFlags, DNA_COUNT(layerFlagCount)> layerFlags;

    Value<atUint32> layerNameCount;
    Vector<String<-1>, DNA_COUNT(layerNameCount)> layerNames;

    Value<atUint32> layerNameOffsetCount;
    Vector<atUint32, DNA_COUNT(layerNameOffsetCount)> layerNameOffsets;
};

}
}
