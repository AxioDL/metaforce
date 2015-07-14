#ifndef __DNAMP2_MLVL_HPP__
#define __DNAMP2_MLVL_HPP__

#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP2
{

struct MLVL : BigDNA
{
    DECL_DNA
    Value<atUint32> magic;
    Value<atUint32> version;
    UniqueID32 worldNameId;
    UniqueID32 darkWorldNameId;
    Value<atUint32> unk;
    UniqueID32 saveWorldId;
    UniqueID32 worldSkyboxId;

    Value<atUint32> areaCount;
    struct Area : BigDNA
    {
        DECL_DNA
        UniqueID32 areaNameId;
        Value<atVec4f> transformMtx[3];
        Value<atVec3f> aabb[2];
        UniqueID32 areaMREAId;
        UniqueID32 areaId;

        Value<atUint32> attachedAreaCount;
        Vector<atUint16, DNA_COUNT(attachedAreaCount)> attachedAreas;
        Value<atUint32> padding;

        Value<atUint32> depCount;
        struct Dependency : BigDNA
        {
            DECL_DNA
            UniqueID32 id;
            FourCC type;
        };
        Vector<Dependency, DNA_COUNT(depCount)> deps;

        Value<atUint32> depLayerCount;
        Vector<atUint32, DNA_COUNT(depLayerCount)> depLayers;

        Value<atUint32> dockCount;
        struct Dock : BigDNA
        {
            DECL_DNA
            Value<atUint32> endpointCount;
            struct Endpoint : BigDNA
            {
                DECL_DNA
                Value<atUint32> areaIdx;
                Value<atUint32> dockIdx;
            };
            Vector<Endpoint, DNA_COUNT(endpointCount)> endpoints;

            Value<atUint32> planeVertCount;
            Vector<atVec3f, DNA_COUNT(planeVertCount)> planeVerts;
        };
        Vector<Dock, DNA_COUNT(dockCount)> docks;

        Value<atUint32> relCount;
        Vector<String<-1>, DNA_COUNT(relCount)> relFilenames;
        Value<atUint32> relOffsetCount;
        Vector<atUint32, DNA_COUNT(relOffsetCount)> relOffsets;

        String<-1> internalAreaName;
    };

    UniqueID32 worldMap;
    Value<atUint8> unknown2;
    Value<atUint32> unknown3;

    Value<atUint32> layerFlagCount;
    struct LayerFlags : BigDNA
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

#endif // __DNAMP2_MLVL_HPP__
