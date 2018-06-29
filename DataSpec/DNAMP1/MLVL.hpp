#ifndef __DNAMP1_MLVL_HPP__
#define __DNAMP1_MLVL_HPP__

#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/MLVL.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

struct MLVL : BigDNA
{
    AT_DECL_DNA_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    UniqueID32 worldNameId;
    UniqueID32 saveWorldId;
    UniqueID32 worldSkyboxId;

    Value<atUint32> memRelayLinkCount;
    struct MemRelayLink : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> memRelayId;
        Value<atUint32> targetId;
        Value<atUint16> msg;
        Value<bool> active;
    };
    Vector<MemRelayLink, AT_DNA_COUNT(memRelayLinkCount)> memRelayLinks;

    Value<atUint32> areaCount;
    Value<atUint32> unknown1;
    struct Area : BigDNA
    {
        AT_DECL_DNA_YAML
        UniqueID32 areaNameId;
        Value<atVec4f> transformMtx[3];
        Value<atVec3f> aabb[2];
        UniqueID32 areaMREAId;
        Value<atUint32> areaId;

        Value<atUint32> attachedAreaCount;
        Vector<atUint16, AT_DNA_COUNT(attachedAreaCount)> attachedAreas;

        struct Dependency : BigDNA
        {
            AT_DECL_DNA_YAML
            UniqueID32 id;
            DNAFourCC type;

            Dependency() = default;
            Dependency(const UniqueID32& idin, const hecl::FourCC& fcc)
                : id(idin), type(fcc) {}
        };

        Value<atUint32> lazyDepCount;
        Vector<Dependency, AT_DNA_COUNT(lazyDepCount)> lazyDeps;

        Value<atUint32> depCount;
        Vector<Dependency, AT_DNA_COUNT(depCount)> deps;

        Value<atUint32> depLayerCount;
        Vector<atUint32, AT_DNA_COUNT(depLayerCount)> depLayers;

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
    };
    Vector<Area, AT_DNA_COUNT(areaCount)> areas;

    void finishLastArea()
    {
        if (areas.size())
        {
            MLVL::Area& areaLast = areas.back();
            areaLast.attachedAreaCount = areaLast.attachedAreas.size();
            areaLast.lazyDepCount = areaLast.lazyDeps.size();
            areaLast.depCount = areaLast.deps.size();
            areaLast.depLayerCount = areaLast.depLayers.size();
            areaLast.dockCount = areaLast.docks.size();
        }
    }

    UniqueID32 worldMap;
    Value<atUint8> unknown2;
    Value<atUint32> unknown3;

    Value<atUint32> audioGroupCount;
    struct AudioGroup : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> groupId;
        UniqueID32 agscId;
    };
    Vector<AudioGroup, AT_DNA_COUNT(audioGroupCount)> audioGroups;
    String<-1> unkString;

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

    Value<atUint32> layerNameOffsetCount;
    Vector<atUint32, AT_DNA_COUNT(layerNameOffsetCount)> layerNameOffsets;

    void readMeta(athena::io::YAMLDocReader& __dna_docin)
    {
        /* worldSkyboxId */
        __dna_docin.enumerate("worldSkyboxId", worldSkyboxId);
        /* audioGroupCount squelched */
        /* audioGroups */
        audioGroupCount = __dna_docin.enumerate("audioGroups", audioGroups);
    }

    void writeMeta(athena::io::YAMLDocWriter& __dna_docout) const
    {
        /* worldSkyboxId */
        __dna_docout.enumerate("worldSkyboxId", worldSkyboxId);
        /* audioGroupCount squelched */
        /* audioGroups */
        __dna_docout.enumerate("audioGroups", audioGroups);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged);

    using World = hecl::blender::World;

    static bool Cook(const hecl::ProjectPath& outPath,
                     const hecl::ProjectPath& inPath,
                     const World& wld,
                     hecl::blender::Token& btok);

    static bool CookMAPW(const hecl::ProjectPath& outPath,
                         const World& wld,
                         hecl::blender::Token& btok);

    static bool CookSAVW(const hecl::ProjectPath& outPath,
                         const World& wld);
};

}

#endif // __DNAMP1_MLVL_HPP__
