#pragma once

#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/MLVL.hpp"
#include "DNAMP3.hpp"

namespace DataSpec::DNAMP3
{

struct MLVL : BigDNA
{
    AT_DECL_DNA_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    UniqueID64 worldNameId;
    Value<atUint32> unk;
    UniqueID64 saveWorldId;
    UniqueID64 worldSkyboxId;

    Value<atUint32> areaCount;
    struct Area : BigDNA
    {
        AT_DECL_DNA_YAML
        UniqueID64 areaNameId;
        Value<atVec4f> transformMtx[3];
        Value<atVec3f> aabb[2];
        UniqueID64 areaMREAId;
        Value<atUint64> areaId;

        Value<atUint32> attachedAreaCount;
        Vector<atUint16, AT_DNA_COUNT(attachedAreaCount)> attachedAreas;

        Value<atUint32> dockCount;
        struct Dock : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> endpointCount;
            struct Endpoint : BigDNA
            {
                AT_DECL_DNA_YAML
                Value<atUint32> areaIdx;
                Value<atUint32> dockIdx;
            };
            Vector<Endpoint, AT_DNA_COUNT(endpointCount)> endpoints;

            Value<atUint32> planeVertCount;
            Vector<atVec3f, AT_DNA_COUNT(planeVertCount)> planeVerts;
        };
        Vector<Dock, AT_DNA_COUNT(dockCount)> docks;

        String<-1> internalAreaName;
    };
    Vector<Area, AT_DNA_COUNT(areaCount)> areas;

    UniqueID64 worldMap;
    Value<atUint8> unknown2;
    Value<atUint32> unknown3;

    Value<atUint32> layerFlagCount;
    struct LayerFlags : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> layerCount;
        Value<atUint64> flags;
    };
    Vector<LayerFlags, AT_DNA_COUNT(layerFlagCount)> layerFlags;

    Value<atUint32> layerNameCount;
    Vector<String<-1>, AT_DNA_COUNT(layerNameCount)> layerNames;

    Value<atUint32> layerIDCount;
    struct LayerID : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint64> id[2];
    };
    Vector<LayerID, AT_DNA_COUNT(layerIDCount)> layerIDs;

    Value<atUint32> layerNameOffsetCount;
    Vector<atUint32, AT_DNA_COUNT(layerNameOffsetCount)> layerNameOffsets;


    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        MLVL mlvl;
        mlvl.read(rs);
        athena::io::FileWriter writer(outPath.getWithExtension(_SYS_STR(".yaml"), true).getAbsolutePath());
        athena::io::ToYAMLStream(mlvl, writer);
        hecl::blender::Connection& conn = btok.getBlenderConnection();
        return DNAMLVL::ReadMLVLToBlender(conn, mlvl, outPath, pakRouter,
                                          entry, force, fileChanged);
    }
};

}

