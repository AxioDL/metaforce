#include "SpecBase.hpp"
#include "Blender/BlenderSupport.hpp"
#include "BlenderConnection.hpp"
#include "DNACommon/DNACommon.hpp"

#include <time.h>

namespace Retro
{

static LogVisor::LogModule Log("Retro::SpecBase");

static const HECL::SystemChar* MomErr[] =
{
    _S("Your metroid is in another castle"),
    _S("HECL is experiencing a PTSD attack"),
    _S("Unable to freeze metroids"),
    _S("Ridley ate your homework"),
    _S("Expected 0 maternal symbolisms, found ∞"),
    _S("Contradictive narratives unsupported"),
    _S("Wiimote profile \"NES + Zapper\" not recognized"),
    _S("Unable to find Waldo"),
    _S("Expected Ridley, found furby")
};

SpecBase::SpecBase(HECL::Database::Project& project)
: m_project(project),
  m_masterShader(project.getProjectWorkingPath(), ".hecl/RetroMasterShader.blend") {}

bool SpecBase::canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps)
{
    m_disc = NOD::OpenDiscFromImage(info.srcpath.c_str(), m_isWii);
    if (!m_disc)
        return false;
    const char* gameID = m_disc->getHeader().gameID;

    if (!memcmp(gameID, "R3O", 3))
    {
        unsigned int t = time(nullptr);
        int r = rand_r(&t) % 9;
        Log.report(LogVisor::FatalError, MomErr[r]);
    }

    m_standalone = true;
    if (m_isWii && !memcmp(gameID, "R3M", 3))
        m_standalone = false;

    if (m_standalone && !checkStandaloneID(gameID))
        return false;

    char region = m_disc->getHeader().gameID[3];
    static const HECL::SystemString regNONE = _S("");
    static const HECL::SystemString regE = _S("NTSC");
    static const HECL::SystemString regJ = _S("NTSC-J");
    static const HECL::SystemString regP = _S("PAL");
    const HECL::SystemString* regstr = &regNONE;
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
    Retro::g_curSpec = this;
    if (!Blender::BuildMasterShader(m_masterShader))
        Log.report(LogVisor::FatalError, "Unable to build master shader blend");
    if (m_isWii)
    {
        /* Extract update partition for repacking later */
        const HECL::SystemString& target = m_project.getProjectWorkingPath().getAbsolutePath();
        NOD::Partition* update = m_disc->getUpdatePartition();
        NOD::ExtractionContext ctx = {true, info.force, nullptr};

        if (update)
        {
            atUint64 idx = 0;
            progress(_S("Update Partition"), _S(""), 0, 0.0);
            const atUint64 nodeCount = update->getFSTRoot().rawEnd() - update->getFSTRoot().rawBegin();
            ctx.progressCB = [&](const std::string& name) {
                HECL::SystemStringView nameView(name);
                progress(_S("Update Partition"), nameView.sys_str().c_str(), 0, idx / (float)nodeCount);
                idx++;
            };

            update->getFSTRoot().extractToDirectory(target, ctx);
            progress(_S("Update Partition"), _S(""), 0, 1.0);
        }

        if (!m_standalone)
        {
            progress(_S("Trilogy Files"), _S(""), 1, 0.0);
            NOD::Partition* data = m_disc->getDataPartition();
            const NOD::Node& root = data->getFSTRoot();
            for (const NOD::Node& child : root)
                if (child.getKind() == NOD::Node::Kind::File)
                    child.extractToDirectory(target, ctx);
            progress(_S("Trilogy Files"), _S(""), 1, 1.0);
        }
    }
    extractFromDisc(*m_disc, info.force, progress);
}

bool SpecBase::canCook(const HECL::ProjectPath& path)
{
    if (!checkPathPrefix(path))
        return false;
    if (HECL::IsPathBlend(path))
    {
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.openBlend(path))
            return false;
        if (conn.getBlendType() != HECL::BlenderConnection::BlendType::None)
            return true;
    }
    else if (HECL::IsPathPNG(path))
    {
        return true;
    }
    else if (HECL::IsPathYAML(path))
    {
        FILE* fp = HECL::Fopen(path.getAbsolutePath().c_str(), _S("r"));
        bool retval = validateYAMLDNAType(fp);
        fclose(fp);
        return retval;
    }
    return false;
}

void SpecBase::doCook(const HECL::ProjectPath& path, const HECL::ProjectPath& cookedPath,
                      bool fast, FCookProgress progress)
{
    Retro::g_curSpec = this;
    if (HECL::IsPathBlend(path))
    {
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.openBlend(path))
            return;
        switch (conn.getBlendType())
        {
        case HECL::BlenderConnection::BlendType::Mesh:
        {
            HECL::BlenderConnection::DataStream ds = conn.beginData();
            cookMesh(cookedPath, path, ds, fast, progress);
            break;
        }
        case HECL::BlenderConnection::BlendType::Actor:
        {
            HECL::BlenderConnection::DataStream ds = conn.beginData();
            cookActor(cookedPath, path, ds, fast, progress);
            break;
        }
        case HECL::BlenderConnection::BlendType::Area:
        {
            HECL::BlenderConnection::DataStream ds = conn.beginData();
            cookArea(cookedPath, path, ds, fast, progress);
            break;
        }
        default: break;
        }
    }
    else if (HECL::IsPathYAML(path))
    {
        FILE* fp = HECL::Fopen(path.getAbsolutePath().c_str(), _S("r"));
        cookYAML(cookedPath, path, fp, progress);
    }
}

bool SpecBase::canPackage(const PackagePassInfo& info)
{
    return false;
}

void SpecBase::gatherDependencies(const PackagePassInfo& info,
                                  std::unordered_set<HECL::ProjectPath>& implicitsOut)
{
}

void SpecBase::doPackage(const PackagePassInfo& info)
{
}

}
