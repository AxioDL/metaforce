#if _WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#endif

#include "SpecBase.hpp"
#include "Blender/BlenderSupport.hpp"
#include "BlenderConnection.hpp"
#include "DNACommon/DNACommon.hpp"

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
    _S("Error: Lady returned objection")
};

constexpr uint32_t MomErrCount = 11;
SpecBase::SpecBase(hecl::Database::Project& project)
: m_project(project),
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
        /* Extract update partition for repacking later */
        const hecl::SystemString& target = m_project.getProjectWorkingPath().getAbsolutePath();
        nod::Partition* update = m_disc->getUpdatePartition();
        nod::ExtractionContext ctx = {true, info.force, nullptr};

        if (update)
        {
            atUint64 idx = 0;
            progress(_S("Update Partition"), _S(""), 0, 0.0);
            const atUint64 nodeCount = update->getFSTRoot().rawEnd() - update->getFSTRoot().rawBegin();
            ctx.progressCB = [&](const std::string& name) {
                hecl::SystemStringView nameView(name);
                progress(_S("Update Partition"), nameView.sys_str().c_str(), 0, idx / (float)nodeCount);
                idx++;
            };

            update->getFSTRoot().extractToDirectory(target, ctx);
            progress(_S("Update Partition"), _S(""), 0, 1.0);
        }

        if (!m_standalone)
        {
            progress(_S("Trilogy Files"), _S(""), 1, 0.0);
            nod::Partition* data = m_disc->getDataPartition();
            const nod::Node& root = data->getFSTRoot();
            for (const nod::Node& child : root)
                if (child.getKind() == nod::Node::Kind::File)
                    child.extractToDirectory(target, ctx);
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
