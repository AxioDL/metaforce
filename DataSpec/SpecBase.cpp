#if _WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#endif

#include "SpecBase.hpp"
#include "Blender/BlenderSupport.hpp"
#include "hecl/Blender/BlenderConnection.hpp"
#include "DNACommon/DNACommon.hpp"
#include "DNACommon/TXTR.hpp"

#include <time.h>

namespace DataSpec
{

static logvisor::Module Log("urde::SpecBase");

static const hecl::SystemChar* MomErr[] =
{
    _S("Your metroid is in another castle"),
    _S("HECL is experiencing a PTSD attack"),
    _S("Unable to freeze metroids"),
    _S("Ridley ate your homework"),
    _S("Expected 0 maternal symbolisms, found ∞"),
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
    DataSpec::UniqueIDBridge::setThreadProject(m_project);
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
    if (m_isWii && !memcmp(gameID, "R3M", 3))
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
        outDir.makeDir();
        nod::ExtractionContext ctx = {true, info.force, nullptr};

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
        return false;
    return true;
}

static bool IsPathSong(const hecl::ProjectPath& path)
{
    if (path.getPathType() != hecl::ProjectPath::Type::Glob ||
        !path.getWithExtension(_S(".mid"), true).isFile() ||
        !path.getWithExtension(_S(".yaml"), true).isFile())
        return false;
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
                                                                hecl::BlenderToken& btok)
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
    return getOriginalSpec();
}

void SpecBase::doCook(const hecl::ProjectPath& path, const hecl::ProjectPath& cookedPath,
                      bool fast, hecl::BlenderToken& btok, FCookProgress progress)
{
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

bool SpecBase::canPackage(const PackagePassInfo& info)
{
    return false;
}

void SpecBase::gatherDependencies(const PackagePassInfo& info,
                                  std::unordered_set<hecl::ProjectPath>& implicitsOut)
{
}

void SpecBase::doPackage(const PackagePassInfo& info)
{
}

}
