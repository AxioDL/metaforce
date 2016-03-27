#if _WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#endif

#include "SpecBase.hpp"
#include "Blender/BlenderSupport.hpp"
#include "BlenderConnection.hpp"
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
SpecBase::SpecBase(hecl::Database::Project& project, bool pc)
: m_project(project), m_pc(pc),
  m_masterShader(project.getProjectWorkingPath(), ".hecl/RetroMasterShader.blend") {}

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
    static const hecl::SystemString regNONE = _S("");
    static const hecl::SystemString regE = _S("NTSC");
    static const hecl::SystemString regJ = _S("NTSC-J");
    static const hecl::SystemString regP = _S("PAL");
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
    DataSpec::g_curSpec = this;
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

bool SpecBase::canCook(const hecl::ProjectPath& path)
{
    if (!checkPathPrefix(path))
        return false;
    if (hecl::IsPathBlend(path))
    {
        hecl::BlenderConnection& conn = hecl::BlenderConnection::SharedConnection();
        if (!conn.openBlend(path))
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
        FILE* fp = hecl::Fopen(path.getAbsolutePath().c_str(), _S("r"));
        bool retval = validateYAMLDNAType(fp);
        fclose(fp);
        return retval;
    }
    return false;
}

const hecl::Database::DataSpecEntry* SpecBase::overrideDataSpec(const hecl::ProjectPath& path,
                                                                const hecl::Database::DataSpecEntry* oldEntry)
{
    if (!checkPathPrefix(path))
        return nullptr;
    if (hecl::IsPathBlend(path))
    {
        hecl::BlenderConnection& conn = hecl::BlenderConnection::SharedConnection();
        if (!conn.openBlend(path))
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
                      bool fast, FCookProgress progress)
{
    DataSpec::g_curSpec = this;
    if (hecl::IsPathBlend(path))
    {
        hecl::BlenderConnection& conn = hecl::BlenderConnection::SharedConnection();
        if (!conn.openBlend(path))
            return;
        switch (conn.getBlendType())
        {
        case hecl::BlenderConnection::BlendType::Mesh:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookMesh(cookedPath, path, ds, fast, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::Actor:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookActor(cookedPath, path, ds, fast, progress);
            break;
        }
        case hecl::BlenderConnection::BlendType::Area:
        {
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            cookArea(cookedPath, path, ds, fast, progress);
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
        FILE* fp = hecl::Fopen(path.getAbsolutePath().c_str(), _S("r"));
        cookYAML(cookedPath, path, fp, progress);
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
