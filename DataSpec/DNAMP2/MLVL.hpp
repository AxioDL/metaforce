#ifndef __DNAMP2_MLVL_HPP__
#define __DNAMP2_MLVL_HPP__

#include "../DNACommon/PAK.hpp"

namespace Retro
{
namespace DNAMP2
{

struct MLVL : BigYAML
{
    DECL_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    UniqueID32 worldNameId;
    UniqueID32 darkWorldNameId;
    Value<atUint32> unk;
    UniqueID32 saveWorldId;
    UniqueID32 worldSkyboxId;

    Value<atUint32> areaCount;
    struct Area : BigYAML
    {
        DECL_YAML
        UniqueID32 areaNameId;
        Value<atVec4f> transformMtx[3];
        Value<atVec3f> aabb[2];
        UniqueID32 areaMREAId;
        UniqueID32 areaId;

        Value<atUint32> attachedAreaCount;
        Vector<atUint16, DNA_COUNT(attachedAreaCount)> attachedAreas;
        Value<atUint32> padding;

        Value<atUint32> depCount;
        struct Dependency : BigYAML
        {
            DECL_YAML
            UniqueID32 id;
            DNAFourCC type;
        };
        Vector<Dependency, DNA_COUNT(depCount)> deps;

        Value<atUint32> depLayerCount;
        Vector<atUint32, DNA_COUNT(depLayerCount)> depLayers;

        Value<atUint32> dockCount;
        struct Dock : BigYAML
        {
            DECL_YAML
            Value<atUint32> endpointCount;
            struct Endpoint : BigYAML
            {
                DECL_YAML
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
    Vector<Area, DNA_COUNT(areaCount)> areas;

    UniqueID32 worldMap;
    Value<atUint8> unknown2;
    Value<atUint32> unknown3;

    Value<atUint32> layerFlagCount;
    struct LayerFlags : BigYAML
    {
        DECL_YAML
        Value<atUint32> layerCount;
        Value<atUint64> flags;
    };
    Vector<LayerFlags, DNA_COUNT(layerFlagCount)> layerFlags;

    Value<atUint32> layerNameCount;
    Vector<String<-1>, DNA_COUNT(layerNameCount)> layerNames;

    Value<atUint32> layerNameOffsetCount;
    Vector<atUint32, DNA_COUNT(layerNameOffsetCount)> layerNameOffsets;

    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        MLVL mlvl;
        mlvl.read(rs);
        FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
        mlvl.toYAMLFile(fp);
        fclose(fp);
        return true;
    }
};

}
}

#endif // __DNAMP2_MLVL_HPP__
