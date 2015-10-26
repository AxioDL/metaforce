#ifndef __DNAMP1_MLVL_HPP__
#define __DNAMP1_MLVL_HPP__

#include "../DNACommon/PAK.hpp"
#include "../DNACommon/MLVL.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{

struct MLVL : BigYAML
{
    DECL_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    UniqueID32 worldNameId;
    UniqueID32 saveWorldId;
    UniqueID32 worldSkyboxId;

    Value<atUint32> memRelayLinkCount;
    struct MemRelayLink : BigYAML
    {
        DECL_YAML
        Value<atUint32> memRelayId;
        Value<atUint32> targetId;
        Value<atUint16> msg;
        Value<atUint8> unk;
    };
    Vector<MemRelayLink, DNA_COUNT(memRelayLinkCount)> memRelayLinks;

    Value<atUint32> areaCount;
    Value<atUint32> unknown1;
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
    };
    Vector<Area, DNA_COUNT(areaCount)> areas;

    UniqueID32 worldMap;
    Value<atUint8> unknown2;
    Value<atUint32> unknown3;

    Value<atUint32> audioGroupCount;
    struct AudioGroup : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown;
        UniqueID32 agscId;
    };
    Vector<AudioGroup, DNA_COUNT(audioGroupCount)> audioGroups;
    String<-1> unkString;

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
        FILE* fp = HECL::Fopen(outPath.getWithExtension(_S(".yaml"), true).getAbsolutePath().c_str(), _S("w"));
        mlvl.toYAMLFile(fp);
        fclose(fp);
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        return DNAMLVL::ReadMLVLToBlender(conn, mlvl, outPath, pakRouter,
                                          entry, force, fileChanged);
    }
};

}
}

#endif // __DNAMP1_MLVL_HPP__
