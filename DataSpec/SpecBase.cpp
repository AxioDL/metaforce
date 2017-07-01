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
#include <png.h>

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
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            hecl::BlenderConnection::DataStream::Actor actor = ds.compileActorCharacterOnly();
            for (auto& sub : actor.subtypes)
            {
                if (sub.armature >= 0)
                {
                    pathsOut.push_back(sub.mesh);

                    hecl::SystemStringView chSysName(sub.name);
                    pathsOut.push_back(path.ensureAuxInfo(chSysName.sys_str() + _S(".CSKR")));

                    const auto& arm = actor.armatures[sub.armature];
                    hecl::SystemStringView armSysName(arm.name);
                    pathsOut.push_back(path.ensureAuxInfo(armSysName.sys_str() + _S(".CINF")));
                    if (sub.overlayMeshes.size())
                    {
                        pathsOut.push_back(sub.overlayMeshes[0].second);
                        pathsOut.push_back(path.ensureAuxInfo(chSysName.sys_str() + _S(".over.CSKR")));
                    }
                }
            }
            break;
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

bool SpecBase::canPackage(const PackagePassInfo& info)
{
    return false;
}

void SpecBase::doPackage(const PackagePassInfo& info)
{
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

void SpecBase::ExtractRandomStaticEntropy(const uint8_t* buf, const hecl::ProjectPath& noAramPath)
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

}
