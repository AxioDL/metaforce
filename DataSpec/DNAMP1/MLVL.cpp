#include "MLVL.hpp"
#include "SCLY.hpp"
#include "SAVW.hpp"
#include "SCAN.hpp"
#include "ScriptObjects/MemoryRelay.hpp"
#include "ScriptObjects/SpecialFunction.hpp"
#include "ScriptObjects/DoorArea.hpp"
#include "Runtime/RetroTypes.hpp"

namespace DataSpec
{
namespace DNAMP1
{

bool MLVL::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool force,
                   hecl::BlenderToken& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged)
{
    MLVL mlvl;
    mlvl.read(rs);

    for (const MLVL::Area& area : mlvl.areas)
    {
        hecl::ProjectPath areaDir = pakRouter.getWorking(area.areaMREAId).getParentPath();
        athena::io::FileWriter fw(hecl::ProjectPath(areaDir, _S("!memoryid.yaml")).getAbsolutePath());
        athena::io::YAMLDocWriter w(nullptr);
        w.writeUint32("memoryid", area.areaId);
        w.finish(&fw);
    }

    athena::io::FileWriter writer(outPath.getWithExtension(_S(".yaml"), true).getAbsolutePath());
    mlvl.toYAMLStream(writer, static_cast<YAMLWriteMemberFn>(&MLVL::writeMeta));
    hecl::BlenderConnection& conn = btok.getBlenderConnection();
    return DNAMLVL::ReadMLVLToBlender(conn, mlvl, outPath, pakRouter,
                                      entry, force, fileChanged);
}

bool MLVL::Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const World& wld)
{
    MLVL mlvl = {};
    athena::io::FileReader reader(inPath.getWithExtension(_S(".yaml"), true).getAbsolutePath());
    mlvl.fromYAMLStream(reader, static_cast<YAMLReadMemberFn>(&MLVL::readMeta));

    mlvl.magic = 0xDEAFBABE;
    mlvl.version = 0x11;
    hecl::ProjectPath savwPath = inPath.ensureAuxInfo(_S(".SAVW"));
    mlvl.saveWorldId = savwPath;
    hecl::ProjectPath mapwPath = inPath.ensureAuxInfo(_S(".MAPW"));
    mlvl.worldMap = mapwPath;

    std::vector<urde::SObjectTag> mapaTags;
    mapaTags.reserve(wld.areas.size());

    SAVW savw = {};
    savw.header.magic = 0xC001D00D;
    savw.header.version = 0x3;

    size_t areaIdx = 0;
    size_t nameOffset = 0;
    for (const World::Area& area : wld.areas)
    {
        if (area.path.getPathType() != hecl::ProjectPath::Type::Directory)
            continue;

        hecl::ProjectPath areaPath(area.path, _S("/!area.blend"));
        if (!areaPath.isFile())
            continue;

        hecl::DirectoryEnumerator dEnum(area.path.getAbsolutePath(),
                                        hecl::DirectoryEnumerator::Mode::DirsSorted);
        bool areaInit = false;

        size_t layerIdx = 0;
        for (const hecl::DirectoryEnumerator::Entry& e : dEnum)
        {
            hecl::SystemString layerName;
            hecl::SystemChar* endCh = nullptr;
            hecl::StrToUl(e.m_name.c_str(), &endCh, 0);
            if (!endCh)
                layerName = hecl::StringUtils::TrimWhitespace(e.m_name);
            else
                layerName = hecl::StringUtils::TrimWhitespace(hecl::SystemString(endCh));

            hecl::ProjectPath objectsPath(area.path, e.m_name + _S("/!objects.yaml"));
            if (objectsPath.isNone())
                continue;

            SCLY::ScriptLayer layer;
            {
                athena::io::FileReader freader(objectsPath.getAbsolutePath());
                if (!freader.isOpen())
                    continue;
                if (!BigYAML::ValidateFromYAMLStream<DNAMP1::SCLY::ScriptLayer>(freader))
                    continue;

                athena::io::YAMLDocReader reader;
                if (!reader.parse(&freader))
                    continue;

                layer.read(reader);
            }

            /* Set active flag state */
            hecl::ProjectPath defActivePath(area.path, e.m_name + _S("/!defaultactive"));
            bool active = defActivePath.isNone() ? false : true;

            if (!areaInit)
            {
                /* Finish last area */
                if (mlvl.areas.size())
                {
                    MLVL::Area& areaLast = mlvl.areas.back();
                    areaLast.attachedAreaCount = areaLast.attachedAreas.size();
                    areaLast.depCount = areaLast.deps.size();
                    areaLast.depLayerCount = areaLast.depLayers.size();
                    areaLast.dockCount = areaLast.docks.size();
                }

                /* Area map */
                hecl::ProjectPath mapPath(area.path, _S("/!map.blend"));
                if (mapPath.isFile())
                    mapaTags.push_back(g_curSpec->BuildTagFromPath(mapPath, hecl::SharedBlenderToken));

                /* Populate area record */
                mlvl.areas.emplace_back();
                MLVL::Area& areaOut = mlvl.areas.back();

                hecl::ProjectPath namePath(area.path, _S("/!name.yaml"));
                if (namePath.isFile())
                    areaOut.areaNameId = namePath;

                areaOut.transformMtx[0] = area.transform[0];
                areaOut.transformMtx[1] = area.transform[1];
                areaOut.transformMtx[2] = area.transform[2];
                areaOut.aabb[0] = area.aabb[0];
                areaOut.aabb[1] = area.aabb[1];
                areaOut.areaMREAId = areaPath;
                areaOut.areaId = 0xffffffff;

                hecl::ProjectPath memIdPath(area.path, _S("/!memoryid.yaml"));
                if (namePath.isFile())
                {
                    athena::io::FileReader fr(memIdPath.getAbsolutePath());
                    athena::io::YAMLDocReader r;
                    if (r.parse(&fr))
                        areaOut.areaId = r.readUint32("memoryid");
                }

                /* Attached Areas and Docks */
                {
                    std::unordered_set<uint32_t> addedAreas;
                    areaOut.dockCount = area.docks.size();
                    for (const World::Area::Dock& dock : area.docks)
                    {
                        areaOut.docks.emplace_back();
                        MLVL::Area::Dock& dockOut = areaOut.docks.back();
                        dockOut.endpointCount = 1;
                        dockOut.endpoints.emplace_back();
                        MLVL::Area::Dock::Endpoint& ep = dockOut.endpoints.back();
                        ep.areaIdx = dock.targetArea;
                        ep.dockIdx = dock.targetDock;
                        dockOut.planeVertCount = 4;
                        dockOut.planeVerts.push_back(dock.verts[0]);
                        dockOut.planeVerts.push_back(dock.verts[1]);
                        dockOut.planeVerts.push_back(dock.verts[2]);
                        dockOut.planeVerts.push_back(dock.verts[3]);

                        if (addedAreas.find(dock.targetArea) == addedAreas.cend())
                        {
                            addedAreas.insert(dock.targetArea);
                            areaOut.attachedAreas.push_back(dock.targetArea);
                        }
                    }
                    areaOut.attachedAreaCount = areaOut.attachedAreas.size();
                }

                /* Layer flags */
                mlvl.layerFlags.emplace_back();
                mlvl.layerFlags.back().layerCount = 0;
                mlvl.layerFlags.back().flags = ~0;

                /* Layer name offset */
                mlvl.layerNameOffsets.push_back(nameOffset);

                areaInit = true;
            }

            MLVL::Area& areaOut = mlvl.areas.back();
            areaOut.depLayers.push_back(areaOut.deps.size());

            /* Gather memory relays, scans, and dependencies */
            {
                std::vector<hecl::ProjectPath> depPaths;
                std::vector<Scan> scans;
                for (std::unique_ptr<IScriptObject>& obj : layer.objects)
                {
                    if (obj->type == 0x13)
                    {
                        MemoryRelay& memRelay = static_cast<MemoryRelay&>(*obj);
                        for (IScriptObject::Connection& conn : memRelay.connections)
                        {
                            mlvl.memRelayLinks.emplace_back();
                            MemRelayLink& linkOut = mlvl.memRelayLinks.back();
                            linkOut.memRelayId = memRelay.id;
                            linkOut.targetId = conn.target;
                            linkOut.msg = conn.msg;
                            linkOut.active = memRelay.active;
                        }
                        savw.relays.push_back(memRelay.id);
                    }
                    else if (obj->type == 0x3A)
                    {
                        SpecialFunction& specialFunc = static_cast<SpecialFunction&>(*obj);
                        if (specialFunc.type == ESpecialFunctionType::CinematicSkip)
                            savw.skippableCutscenes.push_back(specialFunc.id);
                        else if (specialFunc.type == ESpecialFunctionType::ScriptLayerController)
                        {
                            savw.layers.emplace_back();
                            SAVWCommon::Layer& layer = savw.layers.back();
                            layer.areaId = specialFunc.layerSwitch.area;
                            layer.layer = specialFunc.layerSwitch.layerIdx;
                        }
                    }
                    else if (obj->type == 0x3)
                    {
                        DoorArea& doorArea = static_cast<DoorArea&>(*obj);
                        savw.doors.push_back(doorArea.id);
                    }

                    obj->gatherDependencies(depPaths);
                    obj->gatherScans(scans);
                }

                /* Cull duplicate paths and add typed hash to list */
                std::unordered_set<hecl::Hash> addedPaths;
                for (const hecl::ProjectPath& path : depPaths)
                {
                    if (addedPaths.find(path.hash()) == addedPaths.cend())
                    {
                        addedPaths.insert(path.hash());
                        urde::SObjectTag tag = g_curSpec->BuildTagFromPath(path, hecl::SharedBlenderToken);
                        areaOut.deps.emplace_back(tag.id, tag.type);
                    }
                }

                /* Cull duplicate scans and add to list */
                std::unordered_set<UniqueID32> addedScans;
                for (const Scan& scan : scans)
                {
                    if (addedScans.find(scan.scanId) == addedScans.cend())
                    {
                        addedScans.insert(scan.scanId);
                        hecl::ProjectPath scanPath = UniqueIDBridge::TranslatePakIdToPath(scan.scanId);
                        savw.scans.emplace_back();
                        Scan& scanOut = savw.scans.back();
                        scanOut.scanId = scan.scanId;
                        scanOut.category = SAVWCommon::EScanCategory(SCAN::GetCategory(scanPath));
                    }
                }
            }

            hecl::SystemUTF8View layerU8(layerName);
            mlvl.layerNames.push_back(layerU8.str());
            nameOffset += layerName.size() + 1;

            MLVL::LayerFlags& thisLayFlags = mlvl.layerFlags.back();
            ++thisLayFlags.layerCount;
            if (!active)
                thisLayFlags.flags &= ~(1 << layerIdx);

            ++layerIdx;
        }
        ++areaIdx;
    }

    mlvl.memRelayLinkCount = mlvl.memRelayLinks.size();
    mlvl.areaCount = mlvl.areas.size();
    mlvl.layerFlagCount = mlvl.layerFlags.size();
    mlvl.layerNameCount = mlvl.layerNames.size();
    mlvl.layerNameOffsetCount = mlvl.layerNameOffsets.size();

    /* Write out */
    {
        athena::io::FileWriter fo(outPath.getAbsolutePath());
        mlvl.write(fo);
    }

    /* Write out MAPW */
    {
        hecl::ProjectPath mapwCooked =
            mapwPath.getCookedPath(*g_curSpec->overrideDataSpec(mapwPath, nullptr, hecl::SharedBlenderToken));
        athena::io::FileWriter fo(mapwCooked.getAbsolutePath());
        fo.writeUint32Big(0xDEADF00D);
        fo.writeUint32Big(1);
        fo.writeUint32Big(mapaTags.size());
        for (const urde::SObjectTag& mapa : mapaTags)
            fo.writeUint32Big(mapa.id);
    }

    /* Write out SAVW */
    {
        savw.header.areaCount = mlvl.areaCount;
        savw.skippableCutsceneCount = savw.skippableCutscenes.size();
        savw.relayCount = savw.relays.size();
        savw.layerCount = savw.layers.size();
        savw.doorCount = savw.doors.size();
        savw.scanCount = savw.scans.size();

        hecl::ProjectPath savwCooked =
            savwPath.getCookedPath(*g_curSpec->overrideDataSpec(savwPath, nullptr, hecl::SharedBlenderToken));
        athena::io::FileWriter fo(savwCooked.getAbsolutePath());
        savw.write(fo);
    }

    return true;
}

}
}
