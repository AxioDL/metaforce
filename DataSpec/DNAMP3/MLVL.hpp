#ifndef __DNAMP3_MLVL_HPP__
#define __DNAMP3_MLVL_HPP__

#include "../DNACommon/PAK.hpp"
#include "../DNACommon/MLVL.hpp"
#include "DNAMP3.hpp"

namespace DataSpec
{
namespace DNAMP3
{

struct MLVL : BigYAML
{
    DECL_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    UniqueID64 worldNameId;
    Value<atUint32> unk;
    UniqueID64 saveWorldId;
    UniqueID64 worldSkyboxId;

    Value<atUint32> areaCount;
    struct Area : BigYAML
    {
        DECL_YAML
        UniqueID64 areaNameId;
        Value<atVec4f> transformMtx[3];
        Value<atVec3f> aabb[2];
        UniqueID64 areaMREAId;
        Value<atUint64> areaId;

        Value<atUint32> attachedAreaCount;
        Vector<atUint16, DNA_COUNT(attachedAreaCount)> attachedAreas;

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

        String<-1> internalAreaName;
    };
    Vector<Area, DNA_COUNT(areaCount)> areas;

    UniqueID64 worldMap;
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

    Value<atUint32> layerIDCount;
    Vector<UniqueID128, DNA_COUNT(layerIDCount)> layerIDs;

    Value<atUint32> layerNameOffsetCount;
    Vector<atUint32, DNA_COUNT(layerNameOffsetCount)> layerNameOffsets;


    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        std::function<void(const HECL::SystemChar*)> fileChanged)
    {
        MLVL mlvl;
        mlvl.read(rs);
        FILE* fp = HECL::Fopen(outPath.getWithExtension(_S(".yaml"), true).getAbsolutePath().c_str(), _S("wb"));
        mlvl.toYAMLFile(fp);
        fclose(fp);
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        return DNAMLVL::ReadMLVLToBlender(conn, mlvl, outPath, pakRouter,
                                          entry, force, fileChanged);
    }
};

}
}

#endif // __DNAMP3_MLVL_HPP__
