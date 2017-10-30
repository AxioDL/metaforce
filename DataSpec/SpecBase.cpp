#if _WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#endif

#include "SpecBase.hpp"
#include "Blender/BlenderSupport.hpp"
#include "DNACommon/DNACommon.hpp"
#include "DNACommon/TXTR.hpp"
#include "AssetNameMap.hpp"
#include "hecl/ClientProcess.hpp"

#include <png.h>

#define DUMP_CACHE_FILL 1

namespace DataSpec
{

static logvisor::Module Log("urde::SpecBase");

static const hecl::SystemChar* MomErr[] =
{
    _S("Your metroid is in another castle"),
    _S("HECL is experiencing a PTSD attack"),
    _S("Unable to freeze metroids"),
    _S("Ridley ate your homework"),
    _S("Expected 0 maternal symbolisms, found 2147483647"),
    _S("Contradictive narratives unsupported"),
    _S("Wiimote profile \"NES + Zapper\" not recognized"),
    _S("Unable to find Waldo"),
    _S("Expected Ridley, found furby"),
    _S("Adam has not authorized this, please do not bug the developers"),
    _S("Lady returned objection"),
    _S("Unterminated plot thread 'Deleter' detected")
};

constexpr uint32_t MomErrCount = std::extent<decltype(MomErr)>::value;
SpecBase::SpecBase(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
: hecl::Database::IDataSpec(specEntry), m_project(project), m_pc(pc),
  m_masterShader(project.getProjectWorkingPath(), ".hecl/RetroMasterShader.blend")
{
    AssetNameMap::InitAssetNameMap();
    DataSpec::UniqueIDBridge::setThreadProject(m_project);
}

SpecBase::~SpecBase()
{
    cancelBackgroundIndex();
}

static const hecl::SystemString regNONE = _S("");
static const hecl::SystemString regE = _S("NTSC");
static const hecl::SystemString regJ = _S("NTSC-J");
static const hecl::SystemString regP = _S("PAL");

void SpecBase::setThreadProject()
{
    UniqueIDBridge::setThreadProject(m_project);
}

bool SpecBase::canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps)
{
    m_disc = nod::OpenDiscFromImage(info.srcpath.c_str(), m_isWii);
    if (!m_disc)
        return false;
    const char* gameID = m_disc->getHeader().m_gameID;

    if (!memcmp(gameID, "R3O", 3))
    {
        unsigned int t = time(nullptr);
#if _WIN32
        rand_s(&t);
        int r = t % MomErrCount;
#else
        int r = rand_r(&t) % MomErrCount;
#endif
        Log.report(logvisor::Fatal, MomErr[r]);
    }

    m_standalone = true;
    if (m_isWii && (!memcmp(gameID, "R3M", 3) || !memcmp(gameID, "R3I", 3) || !memcmp(gameID, "R32", 3)))
        m_standalone = false;

    if (m_standalone && !checkStandaloneID(gameID))
        return false;

    char region = m_disc->getHeader().m_gameID[3];
    const hecl::SystemString* regstr = &regNONE;
    switch (region)
    {
    case 'E':
        regstr = &regE;
        break;
    case 'J':
        regstr = &regJ;
        break;
    case 'P':
        regstr = &regP;
        break;
    }

    if (m_standalone)
        return checkFromStandaloneDisc(*m_disc, *regstr, info.extractArgs, reps);
    else
        return checkFromTrilogyDisc(*m_disc, *regstr, info.extractArgs, reps);
}

void SpecBase::doExtract(const ExtractPassInfo& info, FProgress progress)
{
    DataSpec::g_curSpec.reset(this);
    if (!Blender::BuildMasterShader(m_masterShader))
        Log.report(logvisor::Fatal, "Unable to build master shader blend");
    if (m_isWii)
    {
        /* Extract root files for repacking later */
        hecl::ProjectPath outDir(m_project.getProjectWorkingPath(), _S("out"));
        outDir.makeDirChain(true);
        nod::ExtractionContext ctx = {info.force, nullptr};

        if (!m_standalone)
        {
            progress(_S("Trilogy Files"), _S(""), 1, 0.0);
            nod::Partition* data = m_disc->getDataPartition();
            const nod::Node& root = data->getFSTRoot();
            for (const nod::Node& child : root)
                if (child.getKind() == nod::Node::Kind::File)
                    child.extractToDirectory(outDir.getAbsolutePath(), ctx);
            progress(_S("Trilogy Files"), _S(""), 1, 1.0);
        }
    }
    extractFromDisc(*m_disc, info.force, progress);
}

static bool IsPathAudioGroup(const hecl::ProjectPath& path)
{
    if (path.getPathType() != hecl::ProjectPath::Type::Glob ||
        !path.getWithExtension(_S(".pool"), true).isFile() ||
        !path.getWithExtension(_S(".proj"), true).isFile() ||
        !path.getWithExtension(_S(".sdir"), true).isFile() ||
        !path.getWithExtension(_S(".samp"), true).isFile())
    {
        if (path.isFile() &&
            !hecl::StrCmp(_S("proj"), path.getLastComponentExt()) &&
            path.getWithExtension(_S(".pool"), true).isFile() &&
            path.getWithExtension(_S(".sdir"), true).isFile() &&
            path.getWithExtension(_S(".samp"), true).isFile())
            return true;
        return false;
    }
    return true;
}

static bool IsPathSong(const hecl::ProjectPath& path)
{
    if (path.getPathType() != hecl::ProjectPath::Type::Glob ||
        !path.getWithExtension(_S(".mid"), true).isFile() ||
        !path.getWithExtension(_S(".yaml"), true).isFile())
    {
        if (path.isFile() &&
            !hecl::StrCmp(_S("mid"), path.getLastComponentExt()) &&
            path.getWithExtension(_S(".yaml"), true).isFile())
            return true;
        return false;
    }
    return true;
}

bool SpecBase::canCook(const hecl::ProjectPath& path, hecl::BlenderToken& btok)
{
    if (!checkPathPrefix(path))
        return false;

    hecl::ProjectPath asBlend;
    if (path.getPathType() == hecl::ProjectPath::Type::Glob)
        asBlend = path.getWithExtension(_S(".blend"), true);
    else
        asBlend = path;

    if (hecl::IsPathBlend(asBlend))
    {
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.openBlend(asBlend))
            return false;
        if (conn.getBlendType() != hecl::BlenderConnection::BlendType::None)
            return true;
    }
    else if (hecl::IsPathPNG(path))
    {
        return true;
    }
    else if (hecl::IsPathYAML(path))
    {
        athena::io::FileReader reader(path.getAbsolutePath());
        bool retval = validateYAMLDNAType(reader);
        return retval;
    }
    else if (IsPathAudioGroup(path))
    {
        return true;
    }
    else if (IsPathSong(path))
    {
        return true;
    }
    return false;
}

const hecl::Database::DataSpecEntry* SpecBase::overrideDataSpec(const hecl::ProjectPath& path,
                                                                const hecl::Database::DataSpecEntry* oldEntry,
                                                                hecl::BlenderToken& btok) const
{
    if (!checkPathPrefix(path))
        return nullptr;

    hecl::ProjectPath asBlend;
    if (path.getPathType() == hecl::ProjectPath::Type::Glob)
        asBlend = path.getWithExtension(_S(".blend"), true);
    else
        asBlend = path;

    if (hecl::IsPathBlend(asBlend))
    {
        if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")) ||
            hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
            return oldEntry;

        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.openBlend(asBlend))
        {
            Log.report(logvisor::Error, _S("unable to cook '%s'"),
                       path.getAbsolutePath().c_str());
            return nullptr;
        }
        hecl::BlenderConnection::BlendType type = conn.getBlendType();
        if (type == hecl::BlenderConnection::BlendType::Mesh ||
            type == hecl::BlenderConnection::BlendType::Area)
            return oldEntry;
    }
    else if (hecl::IsPathPNG(path))
    {
        return oldEntry;
    }
    return &getOriginalSpec();
}

void SpecBase::doCook(const hecl::ProjectPath& path, const hecl::ProjectPath& cookedPath,
                      bool fast, hecl::BlenderToken& btok, FCookProgress progress)
{
    cookedPath.makeDirChain(false);
    DataSpec::g_curSpec.reset(this);

    hecl::ProjectPath asBlend;
    if (path.getPathType() == hecl::ProjectPath::Type::Glob)
        asBlend = path.getWithExtension(_S(".blend"), true);
    else
        asBlend = path;

    if (hecl::IsPathBlend(asBlend))
    {
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.openBlend(asBlend))
            return;
        switch (conn.getBlendType())
        {
        case hecl::BlenderConnection::BlendType::Mesh:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookMesh(cookedPath, path, ds, fast, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::ColMesh:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookColMesh(cookedPath, path, ds, fast, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::Actor:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookActor(cookedPath, path, ds, fast, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::Area:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookArea(cookedPath, path, ds, fast, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::World:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookWorld(cookedPath, path, ds, fast, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::Frame:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookGuiFrame(cookedPath, path, ds, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::MapArea:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookMapArea(cookedPath, path, ds, btok, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::MapUniverse:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookMapUniverse(cookedPath, path, ds, btok, progress);
            break;
        }
        default: break;
        }
    }
    else if (hecl::IsPathPNG(path))
    {
        if (m_pc)
            TXTR::CookPC(path, cookedPath);
        else
            TXTR::Cook(path, cookedPath);
    }
    else if (hecl::IsPathYAML(path))
    {
        athena::io::FileReader reader(path.getAbsolutePath());
        cookYAML(cookedPath, path, reader, progress);
    }
    else if (IsPathAudioGroup(path))
    {
        cookAudioGroup(cookedPath, path, progress);
    }
    else if (IsPathSong(path))
    {
        cookSong(cookedPath, path, progress);
    }
}

void SpecBase::flattenDependencies(const hecl::ProjectPath& path,
                                   std::vector<hecl::ProjectPath>& pathsOut,
                                   hecl::BlenderToken& btok)
{
    DataSpec::g_curSpec.reset(this);
    g_ThreadBlenderToken.reset(&btok);

    hecl::ProjectPath asBlend;
    if (path.getPathType() == hecl::ProjectPath::Type::Glob)
        asBlend = path.getWithExtension(_S(".blend"), true);
    else
        asBlend = path;

    if (hecl::IsPathBlend(asBlend))
    {
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.openBlend(asBlend))
            return;
        switch (conn.getBlendType())
        {
        case hecl::BlenderConnection::BlendType::Mesh:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            std::vector<hecl::ProjectPath> texs = ds.getTextures();
            for (const hecl::ProjectPath& tex : texs)
                pathsOut.push_back(tex);
            break;
        }
        case hecl::BlenderConnection::BlendType::Actor:
        {
            hecl::ProjectPath asGlob = path.getWithExtension(_S(".*"), true);
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            hecl::BlenderConnection::DataStream::Actor actor = ds.compileActorCharacterOnly();
            for (auto& sub : actor.subtypes)
            {
                if (sub.armature >= 0)
                {
                    pathsOut.push_back(sub.mesh);

                    hecl::SystemStringView chSysName(sub.name);
                    pathsOut.push_back(asGlob.ensureAuxInfo(chSysName.sys_str() + _S(".CSKR")));

                    const auto& arm = actor.armatures[sub.armature];
                    hecl::SystemStringView armSysName(arm.name);
                    pathsOut.push_back(asGlob.ensureAuxInfo(armSysName.sys_str() + _S(".CINF")));
                    for (const auto& overlay : sub.overlayMeshes)
                    {
                        hecl::SystemStringView ovelaySys(overlay.first);
                        pathsOut.push_back(overlay.second);
                        pathsOut.push_back(asGlob.ensureAuxInfo(chSysName.sys_str() + _S('.') +
                                                                ovelaySys.sys_str() + _S(".CSKR")));
                    }
                }
            }
            auto actNames = ds.getActionNames();
            for (const auto& act : actNames)
            {
                hecl::SystemStringView actSysName(act);
                pathsOut.push_back(asGlob.ensureAuxInfo(actSysName.sys_str() + _S(".ANIM")));
                hecl::ProjectPath evntPath = asGlob.getWithExtension(
                    hecl::SysFormat(_S(".%s.evnt.yaml"), actSysName.c_str()).c_str(), true);
                if (evntPath.isFile())
                    pathsOut.push_back(evntPath);
            }

            pathsOut.push_back(asGlob);
            return;
        }
        case hecl::BlenderConnection::BlendType::Area:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            std::vector<hecl::ProjectPath> texs = ds.getTextures();
            for (const hecl::ProjectPath& tex : texs)
                pathsOut.push_back(tex);
            break;
        }
        default: break;
        }
    }
    else if (hecl::IsPathYAML(path))
    {
        athena::io::FileReader reader(path.getAbsolutePath());
        flattenDependenciesYAML(reader, pathsOut);
    }

    pathsOut.push_back(path);
}

void SpecBase::flattenDependencies(const UniqueID32& id, std::vector<hecl::ProjectPath>& pathsOut)
{
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id);
    if (path)
        flattenDependencies(path, pathsOut, *g_ThreadBlenderToken.get());
}

void SpecBase::flattenDependencies(const UniqueID64& id, std::vector<hecl::ProjectPath>& pathsOut)
{
    hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id);
    if (path)
        flattenDependencies(path, pathsOut, *g_ThreadBlenderToken.get());
}

bool SpecBase::canPackage(const hecl::ProjectPath& path)
{
    auto components = path.getPathComponents();
    if (components.size() <= 1)
        return false;
    return path.isFile() || path.isDirectory();
}

void SpecBase::recursiveBuildResourceList(std::vector<urde::SObjectTag>& listOut,
                                          std::unordered_set<urde::SObjectTag>& addedTags,
                                          const hecl::ProjectPath& path,
                                          hecl::BlenderToken& btok)
{
    hecl::DirectoryEnumerator dEnum(path.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
    for (const auto& ent : dEnum)
    {
        hecl::ProjectPath childPath(path, ent.m_name);
        if (ent.m_isDir)
        {
            recursiveBuildResourceList(listOut, addedTags, childPath, btok);
        }
        else
        {
            std::vector<hecl::ProjectPath> subPaths;
            flattenDependencies(childPath, subPaths, btok);
            for (const auto& subPath : subPaths)
            {
                if (urde::SObjectTag tag = tagFromPath(subPath, btok))
                {
                    if (addedTags.find(tag) != addedTags.end())
                        continue;
                    addedTags.insert(tag);
                    listOut.push_back(tag);
                }
            }
        }
    }
}

void SpecBase::copyBuildListData(std::vector<std::tuple<size_t, size_t, bool>>& fileIndex,
                                 const std::vector<urde::SObjectTag>& buildList,
                                 const hecl::Database::DataSpecEntry* entry,
                                 bool fast, FProgress progress, athena::io::FileWriter& pakOut)
{
    fileIndex.reserve(buildList.size());
    size_t loadIdx = 0;
    for (const auto& tag : buildList)
    {
        fprintf(stderr, "\r %" PRISize " / %" PRISize "  %.4s %08X", ++loadIdx, buildList.size(),
                tag.type.getChars(), (unsigned int)tag.id.Value());

        fileIndex.emplace_back();
        auto& thisIdx = fileIndex.back();
        hecl::ProjectPath path = pathFromTag(tag);
        hecl::ProjectPath cooked = getCookedPath(path, true);
        athena::io::FileReader r(cooked.getAbsolutePath());
        if (r.hasError())
            Log.report(logvisor::Fatal, _S("Unable to open resource %s"), cooked.getRelativePath().c_str());
        atUint64 size = r.length();
        auto data = r.readUBytes(size);
        auto compData = compressPakData(tag, data.get(), size);
        if (compData.first)
        {
            std::get<0>(thisIdx) = pakOut.position();
            std::get<1>(thisIdx) = ROUND_UP_32(compData.second + 4);
            std::get<2>(thisIdx) = true;
            pakOut.writeUint32Big(atUint32(size));
            pakOut.writeUBytes(compData.first.get(), compData.second);
            for (atUint64 i = compData.second + 4 ; i < std::get<1>(thisIdx) ; ++i)
                pakOut.writeUByte(0xff);
        }
        else
        {
            std::get<0>(thisIdx) = pakOut.position();
            std::get<1>(thisIdx) = ROUND_UP_32(size);
            std::get<2>(thisIdx) = false;
            pakOut.writeUBytes(data.get(), size);
            for (atUint64 i = size ; i < std::get<1>(thisIdx) ; ++i)
                pakOut.writeUByte(0xff);
        }
    }
    fprintf(stderr, "\n");
}

void SpecBase::doPackage(const hecl::ProjectPath& path, const hecl::Database::DataSpecEntry* entry,
                         bool fast, hecl::BlenderToken& btok, FProgress progress, hecl::ClientProcess* cp)
{
    /* Prepare complete resource index */
    if (!m_backgroundRunning && m_tagToPath.empty())
        beginBackgroundIndex();
    waitForIndexComplete();

    /* Name pak based on root-relative components */
    auto components = path.getWithExtension(_S(""), true).getPathComponents();
    if (components.size() <= 1)
        return;
    hecl::ProjectPath outPath(m_project.getProjectWorkingPath(),
                             _S("out/") + components[0] + _S("/") + components[1] + _S(".upak"));
    outPath.makeDirChain(false);
    hecl::SystemString tmpPath = outPath.getAbsolutePath() + _S("~");

    /* Output file */
    athena::io::FileWriter pakOut(tmpPath);
    std::vector<urde::SObjectTag> buildList;
    atUint64 resTableOffset = 0;

    if (path.getPathType() == hecl::ProjectPath::Type::File &&
        !hecl::StrCmp(path.getLastComponent(), _S("!world.blend"))) /* World PAK */
    {
        /* Force-cook MLVL and write resource list structure */
        m_project.cookPath(path, progress, false, true, fast);
        hecl::ProjectPath cooked = getCookedPath(path, true);
        buildWorldPakList(path, cooked, btok, pakOut, buildList, resTableOffset);
        if (int64_t rem = pakOut.position() % 32)
            for (int64_t i=0 ; i<32-rem ; ++i)
                pakOut.writeUByte(0xff);
    }
    else if (path.getPathType() == hecl::ProjectPath::Type::Directory) /* General PAK */
    {
        /* Build resource list */
        std::unordered_set<urde::SObjectTag> addedTags;
        recursiveBuildResourceList(buildList, addedTags, path, btok);
        std::vector<std::pair<urde::SObjectTag, std::string>> nameList;

        /* Build name list */
        for (const auto& item : buildList)
        {
            auto search = m_catalogTagToName.find(item);
            if (search != m_catalogTagToName.end())
                nameList.emplace_back(item, search->second);
        }

        /* Write resource list structure */
        buildPakList(btok, pakOut, buildList, nameList, resTableOffset);
        if (int64_t rem = pakOut.position() % 32)
            for (int64_t i=0 ; i<32-rem ; ++i)
                pakOut.writeUByte(0xff);
    }
    else if (path.getPathType() == hecl::ProjectPath::Type::File) /* One-file General PAK */
    {
        /* Build resource list */
        std::vector<hecl::ProjectPath> subPaths;
        flattenDependencies(path, subPaths, btok);
        std::unordered_set<urde::SObjectTag> addedTags;
        std::vector<std::pair<urde::SObjectTag, std::string>> nameList;
        for (const auto& subPath : subPaths)
        {
            if (urde::SObjectTag tag = tagFromPath(subPath, btok))
            {
                if (addedTags.find(tag) != addedTags.end())
                    continue;
                addedTags.insert(tag);
                buildList.push_back(tag);
            }
        }

        /* Build name list */
        for (const auto& item : buildList)
        {
            auto search = m_catalogTagToName.find(item);
            if (search != m_catalogTagToName.end())
                nameList.emplace_back(item, search->second);
        }

        /* Write resource list structure */
        buildPakList(btok, pakOut, buildList, nameList, resTableOffset);
        if (int64_t rem = pakOut.position() % 32)
            for (int64_t i=0 ; i<32-rem ; ++i)
                pakOut.writeUByte(0xff);
    }

    /* Async cook resource list if using ClientProcess */
    if (cp)
    {
        std::unordered_set<urde::SObjectTag> addedTags;
        addedTags.reserve(buildList.size());

        Log.report(logvisor::Info, _S("Validating resources"));
        size_t loadIdx = 0;
        for (auto& tag : buildList)
        {
            fprintf(stderr, "\r %" PRISize " / %" PRISize "  %.4s %08X", ++loadIdx, buildList.size(),
                    tag.type.getChars(), (unsigned int)tag.id.Value());
            if (addedTags.find(tag) != addedTags.end())
                continue;
            addedTags.insert(tag);

            hecl::ProjectPath depPath = pathFromTag(tag);
            if (!depPath)
            {
                fprintf(stderr, "\n");
                Log.report(logvisor::Fatal, _S("Unable to resolve %.4s %08X"),
                           tag.type.getChars(), tag.id.Value());
            }
            m_project.cookPath(depPath, progress, false, false, fast, cp);
        }
        fprintf(stderr, "\n");
        Log.report(logvisor::Info, _S("Waiting for remaining cook transactions"));
        cp->waitUntilComplete();
    }

    /* Write resource data and build file index */
    std::vector<std::tuple<size_t, size_t, bool>> fileIndex;
    Log.report(logvisor::Info, _S("Copying data into %s"), outPath.getRelativePath().c_str());
    copyBuildListData(fileIndex, buildList, entry, fast, progress, pakOut);

    /* Write file index */
    writePakFileIndex(pakOut, buildList, fileIndex, resTableOffset);
    pakOut.close();

    /* Atomic rename */
    hecl::Rename(tmpPath.c_str(), outPath.getAbsolutePath().c_str());
}

hecl::ProjectPath SpecBase::getCookedPath(const hecl::ProjectPath& working, bool pcTarget) const
{
    const hecl::Database::DataSpecEntry* spec = &getOriginalSpec();
    if (pcTarget)
        spec = overrideDataSpec(working, getDataSpecEntry(), hecl::SharedBlenderToken);
    if (!spec)
        return {};
    return working.getCookedPath(*spec);
}

static void PNGErr(png_structp png, png_const_charp msg)
{
    Log.report(logvisor::Error, msg);
}

static void PNGWarn(png_structp png, png_const_charp msg)
{
    Log.report(logvisor::Warning, msg);
}

static inline uint8_t Convert4To8(uint8_t v)
{
    /* Swizzle bits: 00001234 -> 12341234 */
    return (v << 4) | v;
}

void SpecBase::extractRandomStaticEntropy(const uint8_t* buf, const hecl::ProjectPath& noAramPath)
{
    hecl::ProjectPath entropyPath(noAramPath, _S("RandomStaticEntropy.png"));
    hecl::ProjectPath catalogPath(noAramPath, _S("!catalog.yaml"));

    if (FILE* fp = hecl::Fopen(catalogPath.getAbsolutePath().c_str(), _S("a")))
    {
        fprintf(fp, "RandomStaticEntropy: %s\n", entropyPath.getRelativePathUTF8().c_str());
        fclose(fp);
    }

    FILE* fp = hecl::Fopen(entropyPath.getAbsolutePath().c_str(), _S("wb"));
    if (!fp)
    {
        Log.report(logvisor::Error,
                   _S("Unable to open '%s' for writing"),
                   entropyPath.getAbsolutePath().c_str());
        return;
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, PNGErr, PNGWarn);
    png_init_io(png, fp);
    png_infop info = png_create_info_struct(png);

    png_text textStruct = {};
    textStruct.key = png_charp("urde_nomip");
    png_set_text(png, info, &textStruct, 1);

    png_set_IHDR(png, info, 1024, 512, 8,
                 PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    std::unique_ptr<uint8_t[]> rowbuf(new uint8_t[1024*2]);
    for (int y=0 ; y<512 ; ++y)
    {
        for (int x=0 ; x<1024 ; ++x)
        {
            uint8_t texel = buf[y*1024+x];
            rowbuf[x*2] = Convert4To8(texel >> 4 & 0xf);
            rowbuf[x*2+1] = Convert4To8(texel & 0xf);
        }
        png_write_row(png, rowbuf.get());
    }

    png_write_end(png, info);
    png_write_flush(png);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

void SpecBase::clearTagCache()
{
    m_tagToPath.clear();
    m_pathToTag.clear();
    m_catalogNameToTag.clear();
    m_catalogTagToName.clear();
}

hecl::ProjectPath SpecBase::pathFromTag(const urde::SObjectTag& tag) const
{
    std::unique_lock<std::mutex> lk(const_cast<SpecBase&>(*this).m_backgroundIndexMutex);
    auto search = m_tagToPath.find(tag);
    if (search != m_tagToPath.cend())
        return search->second;
    return {};
}

urde::SObjectTag SpecBase::tagFromPath(const hecl::ProjectPath& path,
                                       hecl::BlenderToken& btok) const
{
    auto search = m_pathToTag.find(path.hash());
    if (search != m_pathToTag.cend())
        return search->second;
    return buildTagFromPath(path, btok);
}

bool SpecBase::waitForTagReady(const urde::SObjectTag& tag, const hecl::ProjectPath*& pathOut)
{
    std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
    auto search = m_tagToPath.find(tag);
    if (search == m_tagToPath.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                lk.lock();
                search = m_tagToPath.find(tag);
                if (search != m_tagToPath.end())
                    break;
            }
            if (search == m_tagToPath.end())
                return false;
        }
        else
            return false;
    }
    lk.unlock();
    pathOut = &search->second;
    return true;
}

const urde::SObjectTag* SpecBase::getResourceIdByName(const char* name) const
{
    std::string lower = name;
    std::transform(lower.cbegin(), lower.cend(), lower.begin(), tolower);

    std::unique_lock<std::mutex> lk(const_cast<SpecBase&>(*this).m_backgroundIndexMutex);
    auto search = m_catalogNameToTag.find(lower);
    if (search == m_catalogNameToTag.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                lk.lock();
                search = m_catalogNameToTag.find(lower);
                if (search != m_catalogNameToTag.end())
                    break;
            }
            if (search == m_catalogNameToTag.end())
                return nullptr;
        }
        else
            return nullptr;
    }
    return &search->second;
}

FourCC SpecBase::getResourceTypeById(urde::CAssetId id) const
{
    if (!id.IsValid())
        return {};

    std::unique_lock<std::mutex> lk(const_cast<SpecBase&>(*this).m_backgroundIndexMutex);
    urde::SObjectTag searchTag = {FourCC(), id};
    auto search = m_tagToPath.find(searchTag);
    if (search == m_tagToPath.end())
    {
        if (m_backgroundRunning)
        {
            while (m_backgroundRunning)
            {
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                lk.lock();
                search = m_tagToPath.find(searchTag);
                if (search != m_tagToPath.end())
                    break;
            }
            if (search == m_tagToPath.end())
                return {};
        }
        else
            return {};
    }

    return search->first.type;
}

void SpecBase::enumerateResources(const std::function<bool(const urde::SObjectTag&)>& lambda) const
{
    waitForIndexComplete();
    for (const auto& pair : m_tagToPath)
    {
        if (!lambda(pair.first))
        break;
    }
}

void SpecBase::enumerateNamedResources(
    const std::function<bool(const std::string&, const urde::SObjectTag&)>& lambda) const
{
    waitForIndexComplete();
    for (const auto& pair : m_catalogNameToTag)
    {
        if (!lambda(pair.first, pair.second))
            break;
    }
}

static void WriteTag(athena::io::YAMLDocWriter& cacheWriter,
                     const urde::SObjectTag& pathTag, const hecl::ProjectPath& path)
{
    char idStr[9];
    snprintf(idStr, 9, "%08X", uint32_t(pathTag.id.Value()));
    if (auto v = cacheWriter.enterSubVector(idStr))
    {
        cacheWriter.writeString(nullptr, pathTag.type.toString().c_str());
        cacheWriter.writeString(nullptr, path.getAuxInfo().size() ?
                                         (path.getRelativePathUTF8() + '|' + path.getAuxInfoUTF8()) :
                                         path.getRelativePathUTF8());
    }
}

static void WriteNameTag(athena::io::YAMLDocWriter& nameWriter,
                         const urde::SObjectTag& pathTag,
                         const std::string& name)
{
    char idStr[9];
    snprintf(idStr, 9, "%08X", uint32_t(pathTag.id.Value()));
    nameWriter.writeString(name.c_str(), idStr);
}

void SpecBase::readCatalog(const hecl::ProjectPath& catalogPath,
                           athena::io::YAMLDocWriter& nameWriter)
{
    athena::io::FileReader freader(catalogPath.getAbsolutePath());
    if (!freader.isOpen())
        return;

    athena::io::YAMLDocReader reader;
    bool res = reader.parse(&freader);
    if (!res)
        return;

    const athena::io::YAMLNode* root = reader.getRootNode();
    for (const auto& p : root->m_mapChildren)
    {
        /* Hash as lowercase since lookup is case-insensitive */
        std::string pLower = p.first;
        std::transform(pLower.cbegin(), pLower.cend(), pLower.begin(), tolower);

        /* Avoid redundant filesystem access for re-caches */
        if (m_catalogNameToTag.find(pLower) != m_catalogNameToTag.cend())
            continue;

        athena::io::YAMLNode& node = *p.second;
        hecl::ProjectPath path;
        if (node.m_type == YAML_SCALAR_NODE)
        {
            path = hecl::ProjectPath(m_project.getProjectWorkingPath(), node.m_scalarString);
        }
        else if (node.m_type == YAML_SEQUENCE_NODE)
        {
            if (node.m_seqChildren.size() >= 2)
                path = hecl::ProjectPath(m_project.getProjectWorkingPath(), node.m_seqChildren[0]->m_scalarString).
                    ensureAuxInfo(node.m_seqChildren[1]->m_scalarString);
            else if (node.m_seqChildren.size() == 1)
                path = hecl::ProjectPath(m_project.getProjectWorkingPath(), node.m_seqChildren[0]->m_scalarString);
        }
        if (!path.isFileOrGlob())
            continue;
        urde::SObjectTag pathTag = tagFromPath(path, m_backgroundBlender);
        if (pathTag)
        {
            std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
            m_catalogNameToTag[pLower] = pathTag;
            m_catalogTagToName[pathTag] = p.first;
            WriteNameTag(nameWriter, pathTag, p.first);
#if 0
            fprintf(stderr, "%s %s %08X\n",
                    p.first.c_str(),
                    pathTag.type.toString().c_str(), uint32_t(pathTag.id));
#endif
        }
    }
}

void SpecBase::backgroundIndexRecursiveCatalogs(const hecl::ProjectPath& dir,
                                                athena::io::YAMLDocWriter& nameWriter,
                                                int level)
{
    hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    false, false, true);

    /* Enumerate all items */
    for (const hecl::DirectoryEnumerator::Entry& ent : dEnum)
    {
        hecl::ProjectPath path(dir, ent.m_name);
        if (ent.m_isDir && level < 1)
            backgroundIndexRecursiveCatalogs(path, nameWriter, level + 1);
        else
        {
            if (!path.isFile())
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("!catalog.yaml")))
            {
                readCatalog(path, nameWriter);
                continue;
            }
        }

        /* bail if cancelled by client */
        if (!m_backgroundRunning)
            break;
    }
}

#if DUMP_CACHE_FILL
static void DumpCacheAdd(const urde::SObjectTag& pathTag, const hecl::ProjectPath& path)
{
    fprintf(stderr, "%s %08X %s\n",
            pathTag.type.toString().c_str(), uint32_t(pathTag.id.Value()),
            path.getRelativePathUTF8().c_str());
}
#endif

bool SpecBase::addFileToIndex(const hecl::ProjectPath& path,
                              athena::io::YAMLDocWriter& cacheWriter)
{
    /* Avoid redundant filesystem access for re-caches */
    if (m_pathToTag.find(path.hash()) != m_pathToTag.cend())
        return true;

    /* Try as glob */
    hecl::ProjectPath asGlob = path.getWithExtension(_S(".*"), true);
    if (m_pathToTag.find(asGlob.hash()) != m_pathToTag.cend())
        return true;

    /* Classify intermediate into tag */
    urde::SObjectTag pathTag = buildTagFromPath(path, m_backgroundBlender);
    if (pathTag)
    {
        std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
        bool useGlob = false;

        /* Special multi-resource intermediates */
        if (pathTag.type == SBIG('ANCS'))
        {
            hecl::BlenderConnection& conn = m_backgroundBlender.getBlenderConnection();
            if (!conn.openBlend(path) || conn.getBlendType() != hecl::BlenderConnection::BlendType::Actor)
                return false;

            /* Transform tag to glob */
            pathTag = {SBIG('ANCS'), asGlob.hash().val32()};
            useGlob = true;

            hecl::BlenderConnection::DataStream ds = conn.beginData();
            std::vector<std::string> armatureNames = ds.getArmatureNames();
            std::vector<std::string> subtypeNames = ds.getSubtypeNames();
            std::vector<std::string> actionNames = ds.getActionNames();

            for (const std::string& arm : armatureNames)
            {
                hecl::SystemStringView sysStr(arm);
                hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S(".CINF"));
                urde::SObjectTag pathTag = buildTagFromPath(subPath, m_backgroundBlender);
                m_tagToPath[pathTag] = subPath;
                m_pathToTag[subPath.hash()] = pathTag;
                WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                DumpCacheAdd(pathTag, subPath);
#endif
            }

            for (const std::string& sub : subtypeNames)
            {
                hecl::SystemStringView sysStr(sub);
                hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S(".CSKR"));
                urde::SObjectTag pathTag = buildTagFromPath(subPath, m_backgroundBlender);
                m_tagToPath[pathTag] = subPath;
                m_pathToTag[subPath.hash()] = pathTag;
                WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                DumpCacheAdd(pathTag, subPath);
#endif

                std::vector<std::string> overlayNames = ds.getSubtypeOverlayNames(sub);
                for (const auto& overlay : overlayNames)
                {
                    hecl::SystemStringView overlaySys(overlay);
                    hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S('.') +
                                                                     overlaySys.sys_str() + _S(".CSKR"));
                    urde::SObjectTag pathTag = buildTagFromPath(subPath, m_backgroundBlender);
                    m_tagToPath[pathTag] = subPath;
                    m_pathToTag[subPath.hash()] = pathTag;
                    WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                    DumpCacheAdd(pathTag, subPath);
#endif
                }
            }

            for (const std::string& act : actionNames)
            {
                hecl::SystemStringView sysStr(act);
                hecl::ProjectPath subPath = asGlob.ensureAuxInfo(sysStr.sys_str() + _S(".ANIM"));
                urde::SObjectTag pathTag = buildTagFromPath(subPath, m_backgroundBlender);
                m_tagToPath[pathTag] = subPath;
                m_pathToTag[subPath.hash()] = pathTag;
                WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
                DumpCacheAdd(pathTag, subPath);
#endif
            }
        }
        else if (pathTag.type == SBIG('MLVL'))
        {
            /* Transform tag to glob */
            pathTag = {SBIG('MLVL'), asGlob.hash().val32()};
            useGlob = true;

            hecl::ProjectPath subPath = asGlob.ensureAuxInfo(_S("MAPW"));
            urde::SObjectTag pathTag = buildTagFromPath(subPath, m_backgroundBlender);
            m_tagToPath[pathTag] = subPath;
            m_pathToTag[subPath.hash()] = pathTag;
            WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
            DumpCacheAdd(pathTag, subPath);
#endif

            subPath = asGlob.ensureAuxInfo(_S("SAVW"));
            pathTag = buildTagFromPath(subPath, m_backgroundBlender);
            m_tagToPath[pathTag] = subPath;
            m_pathToTag[subPath.hash()] = pathTag;
            WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
            DumpCacheAdd(pathTag, subPath);
#endif
        }
        else if (pathTag.type == SBIG('AGSC'))
        {
            /* Transform tag to glob */
            pathTag = {SBIG('AGSC'), asGlob.hash().val32()};
            useGlob = true;
        }
        else if (pathTag.type == SBIG('MREA'))
        {
            hecl::ProjectPath subPath = path.ensureAuxInfo(_S("PATH"));
            urde::SObjectTag pathTag = buildTagFromPath(subPath, m_backgroundBlender);
            m_tagToPath[pathTag] = subPath;
            m_pathToTag[subPath.hash()] = pathTag;
            WriteTag(cacheWriter, pathTag, subPath);
#if DUMP_CACHE_FILL
            DumpCacheAdd(pathTag, subPath);
#endif
        }

        /* Cache in-memory */
        const hecl::ProjectPath& usePath = useGlob ? asGlob : path;
        m_tagToPath[pathTag] = usePath;
        m_pathToTag[usePath.hash()] = pathTag;
        WriteTag(cacheWriter, pathTag, usePath);
#if DUMP_CACHE_FILL
        DumpCacheAdd(pathTag, usePath);
#endif
    }

    return true;
}

void SpecBase::backgroundIndexRecursiveProc(const hecl::ProjectPath& dir,
                                            athena::io::YAMLDocWriter& cacheWriter,
                                            athena::io::YAMLDocWriter& nameWriter,
                                            int level)
{
    hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(),
                                    hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted,
                                    false, false, true);

    /* Enumerate all items */
    for (const hecl::DirectoryEnumerator::Entry& ent : dEnum)
    {
        hecl::ProjectPath path(dir, ent.m_name);
        if (ent.m_isDir)
            backgroundIndexRecursiveProc(path, cacheWriter, nameWriter, level + 1);
        else
        {
            if (!path.isFile())
                continue;

            /* Read catalog.yaml for .pak directory if exists */
            if (level == 1 && !ent.m_name.compare(_S("!catalog.yaml")))
            {
                readCatalog(path, nameWriter);
                continue;
            }

            /* Index the regular file */
            addFileToIndex(path, cacheWriter);
        }

        /* bail if cancelled by client */
        if (!m_backgroundRunning)
            break;
    }
}

void SpecBase::backgroundIndexProc()
{
    logvisor::RegisterThreadName("Resource Index Thread");

    hecl::ProjectPath tagCachePath(m_project.getProjectCookedPath(getOriginalSpec()), _S("tag_cache.yaml"));
    hecl::ProjectPath nameCachePath(m_project.getProjectCookedPath(getOriginalSpec()), _S("name_cache.yaml"));
    hecl::ProjectPath specRoot(m_project.getProjectWorkingPath(), getOriginalSpec().m_name);

    /* Cache will be overwritten with validated entries afterwards */
    athena::io::YAMLDocWriter cacheWriter(nullptr);
    athena::io::YAMLDocWriter nameWriter(nullptr);

    /* Read in tag cache */
    if (tagCachePath.isFile())
    {
        athena::io::FileReader reader(tagCachePath.getAbsolutePath());
        if (reader.isOpen())
        {
            Log.report(logvisor::Info, _S("Cache index of '%s' loading"), getOriginalSpec().m_name);
            athena::io::YAMLDocReader cacheReader;
            if (cacheReader.parse(&reader))
            {
                std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                size_t tagCount = cacheReader.getRootNode()->m_mapChildren.size();
                m_tagToPath.reserve(tagCount);
                m_pathToTag.reserve(tagCount);
                size_t loadIdx = 0;
                for (const auto& child : cacheReader.getRootNode()->m_mapChildren)
                {
                    const athena::io::YAMLNode& node = *child.second;
                    unsigned long id = strtoul(child.first.c_str(), nullptr, 16);
                    hecl::FourCC type(node.m_seqChildren.at(0)->m_scalarString.c_str());
                    hecl::ProjectPath path(m_project.getProjectWorkingPath(),
                                           node.m_seqChildren.at(1)->m_scalarString);

                    if (path.isFileOrGlob())
                    {
                        urde::SObjectTag pathTag(type, id);
                        m_tagToPath[pathTag] = path;
                        m_pathToTag[path.hash()] = pathTag;
                        WriteTag(cacheWriter, pathTag, path);
                    }
                    fprintf(stderr, "\r %" PRISize " / %" PRISize, ++loadIdx, tagCount);
                }
                fprintf(stderr, "\n");
            }
            Log.report(logvisor::Info, _S("Cache index of '%s' loaded; %d tags"),
                       getOriginalSpec().m_name, m_tagToPath.size());

            if (nameCachePath.isFile())
            {
                /* Read in name cache */
                Log.report(logvisor::Info, _S("Name index of '%s' loading"), getOriginalSpec().m_name);
                athena::io::FileReader nreader(nameCachePath.getAbsolutePath());
                athena::io::YAMLDocReader nameReader;
                if (nameReader.parse(&nreader))
                {
                    std::unique_lock<std::mutex> lk(m_backgroundIndexMutex);
                    m_catalogNameToTag.reserve(nameReader.getRootNode()->m_mapChildren.size());
                    m_catalogTagToName.reserve(nameReader.getRootNode()->m_mapChildren.size());
                    for (const auto& child : nameReader.getRootNode()->m_mapChildren)
                    {
                        unsigned long id = strtoul(child.second->m_scalarString.c_str(), nullptr, 16);
                        auto search = m_tagToPath.find(urde::SObjectTag(FourCC(), uint32_t(id)));
                        if (search != m_tagToPath.cend())
                        {
                            std::string chLower = child.first;
                            std::transform(chLower.cbegin(), chLower.cend(), chLower.begin(), tolower);
                            m_catalogNameToTag[chLower] = search->first;
                            m_catalogTagToName[search->first] = child.first;
                            WriteNameTag(nameWriter, search->first, child.first);
                        }
                    }
                }
                Log.report(logvisor::Info, _S("Name index of '%s' loaded; %d names"),
                           getOriginalSpec().m_name, m_catalogNameToTag.size());
            }
        }
    }

    /* Add special original IDs resource if exists (not name-cached to disk) */
    hecl::ProjectPath oidsPath(specRoot, "!original_ids.yaml");
    urde::SObjectTag oidsTag = buildTagFromPath(oidsPath, m_backgroundBlender);
    if (oidsTag)
    {
        m_catalogNameToTag["mp1originalids"] = oidsTag;
        m_catalogTagToName[oidsTag] = "MP1OriginalIDs";
    }

    Log.report(logvisor::Info, _S("Background index of '%s' started"), getOriginalSpec().m_name);
    backgroundIndexRecursiveProc(specRoot, cacheWriter, nameWriter, 0);

    tagCachePath.makeDirChain(false);
    athena::io::FileWriter twriter(tagCachePath.getAbsolutePath());
    cacheWriter.finish(&twriter);

    athena::io::FileWriter nwriter(nameCachePath.getAbsolutePath());
    nameWriter.finish(&nwriter);

    m_backgroundBlender.shutdown();
    Log.report(logvisor::Info, _S("Background index of '%s' complete; %d tags, %d names"),
               getOriginalSpec().m_name, m_tagToPath.size(), m_catalogNameToTag.size());
    m_backgroundRunning = false;
}

void SpecBase::cancelBackgroundIndex()
{
    m_backgroundRunning = false;
    if (m_backgroundIndexTh.joinable())
        m_backgroundIndexTh.join();
}

void SpecBase::beginBackgroundIndex()
{
    cancelBackgroundIndex();
    clearTagCache();
    m_backgroundRunning = true;
    m_backgroundIndexTh = std::thread(std::bind(&SpecBase::backgroundIndexProc, this));
}

void SpecBase::waitForIndexComplete() const
{
    std::unique_lock<std::mutex> lk(const_cast<SpecBase&>(*this).m_backgroundIndexMutex);
    while (m_backgroundRunning)
    {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        lk.lock();
    }
}

}
