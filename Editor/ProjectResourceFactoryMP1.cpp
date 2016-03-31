#include "ProjectResourceFactoryMP1.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include "DataSpec/DNACommon/TXTR.hpp"

namespace DataSpec
{
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;
}

namespace urde
{

ProjectResourceFactoryMP1::ProjectResourceFactoryMP1(hecl::ClientProcess& clientProc)
: ProjectResourceFactoryBase(clientProc)
{
    m_factoryMgr.AddFactory(FOURCC('TXTR'), FFactoryFunc(FTextureFactory));
    m_factoryMgr.AddFactory(FOURCC('PART'), FFactoryFunc(FParticleFactory));
    m_factoryMgr.AddFactory(FOURCC('FRME'), FFactoryFunc(RGuiFrameFactoryInGame));
    m_factoryMgr.AddFactory(FOURCC('FONT'), FFactoryFunc(FRasterFontFactory));
    m_factoryMgr.AddFactory(FOURCC('CMDL'), FMemFactoryFunc(FModelFactory));
}

void ProjectResourceFactoryMP1::IndexMP1Resources(hecl::Database::Project& proj)
{
    BeginBackgroundIndex(proj, DataSpec::SpecEntMP1, DataSpec::SpecEntMP1PC);
}

SObjectTag ProjectResourceFactoryMP1::TagFromPath(const hecl::ProjectPath& path) const
{
    if (hecl::IsPathBlend(path))
    {
        hecl::BlenderConnection& conn = GetBackgroundBlender();
        if (!conn.openBlend(path))
            return {};

        switch (conn.getBlendType())
        {
        case hecl::BlenderConnection::BlendType::Mesh:
            return {SBIG('CMDL'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::Actor:
            return {SBIG('ANCS'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::Area:
            return {SBIG('MREA'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::World:
            return {SBIG('MLVL'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::MapArea:
            return {SBIG('MAPA'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::MapUniverse:
            return {SBIG('MAPU'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::Frame:
            return {SBIG('FRME'), path.hash().val32()};
        default:
            return {};
        }
    }
    else if (hecl::IsPathPNG(path))
    {
        return {SBIG('TXTR'), path.hash().val32()};
    }
    else if (hecl::IsPathYAML(path))
    {
        FILE* fp = hecl::Fopen(path.getAbsolutePath().c_str(), _S("r"));
        if (!fp)
            return {};

        athena::io::YAMLDocReader reader;
        yaml_parser_set_input_file(reader.getParser(), fp);

        SObjectTag resTag;
        if (reader.ClassTypeOperation([&](const char* className) -> bool
        {
            if (!strcmp(className, "GPSM"))
            {
                resTag.type = SBIG('PART');
                return true;
            }
            else if (!strcmp(className, "FONT"))
            {
                resTag.type = SBIG('FONT');
                return true;
            }
            return false;
        }))
        {
            resTag.id = path.hash().val32();
            fclose(fp);
            return resTag;
        }
        fclose(fp);
    }
    return {};
}

}
