#include "ProjectResourceFactoryMP1.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Character/CAllFormatsAnimSource.hpp"
#include "Runtime/Character/CAnimPOIData.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "Runtime/CDependencyGroup.hpp"
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
    m_factoryMgr.AddFactory(FOURCC('TXTR'), FMemFactoryFunc(FTextureFactory));
    m_factoryMgr.AddFactory(FOURCC('PART'), FFactoryFunc(FParticleFactory));
    m_factoryMgr.AddFactory(FOURCC('FRME'), FFactoryFunc(RGuiFrameFactoryInGame));
    m_factoryMgr.AddFactory(FOURCC('FONT'), FFactoryFunc(FRasterFontFactory));
    m_factoryMgr.AddFactory(FOURCC('CMDL'), FMemFactoryFunc(FModelFactory));
    m_factoryMgr.AddFactory(FOURCC('CINF'), FFactoryFunc(FCharLayoutInfo));
    m_factoryMgr.AddFactory(FOURCC('CSKR'), FFactoryFunc(FSkinRulesFactory));
    m_factoryMgr.AddFactory(FOURCC('ANCS'), FFactoryFunc(FAnimCharacterSet));
    m_factoryMgr.AddFactory(FOURCC('ANIM'), FFactoryFunc(AnimSourceFactory));
    m_factoryMgr.AddFactory(FOURCC('EVNT'), FFactoryFunc(AnimPOIDataFactory));
    m_factoryMgr.AddFactory(FOURCC('DCLN'), FFactoryFunc(FCollidableOBBTreeGroupFactory));
    m_factoryMgr.AddFactory(FOURCC('DGRP'), FFactoryFunc(FDependencyGroupFactory));
    m_factoryMgr.AddFactory(FOURCC('AGSC'), FMemFactoryFunc(FAudioGroupSetDataFactory));
    m_factoryMgr.AddFactory(FOURCC('STRG'), FFactoryFunc(FStringTableFactory));
    m_factoryMgr.AddFactory(FOURCC('HINT'), FFactoryFunc(FHintFactory));
}

void ProjectResourceFactoryMP1::IndexMP1Resources(hecl::Database::Project& proj)
{
    BeginBackgroundIndex(proj, DataSpec::SpecEntMP1, DataSpec::SpecEntMP1PC);
}

SObjectTag ProjectResourceFactoryMP1::TagFromPath(const hecl::ProjectPath& path, hecl::BlenderToken& btok) const
{
    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CINF")))
        return SObjectTag(SBIG('CINF'), path.hash().val32());
    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")))
        return SObjectTag(SBIG('CSKR'), path.hash().val32());
    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
        return SObjectTag(SBIG('ANIM'), path.hash().val32());

    if (hecl::IsPathBlend(path))
    {
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.openBlend(path))
            return {};

        switch (conn.getBlendType())
        {
        case hecl::BlenderConnection::BlendType::Mesh:
            return {SBIG('CMDL'), path.hash().val32()};
        case hecl::BlenderConnection::BlendType::Actor:
            if (path.getAuxInfo().size())
            {
                if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CINF")))
                    return {SBIG('CINF'), path.hash().val32()};
                else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")))
                    return {SBIG('CSKR'), path.hash().val32()};
                else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
                    return {SBIG('ANIM'), path.hash().val32()};
            }
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
        if (reader.ClassTypeOperation([&](const char* className) -> bool {
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
                else if (!strcmp(className, "urde::DNAMP1::EVNT"))
                {
                    resTag.type = SBIG('EVNT');
                    return true;
                }
                else if (!strcmp(className, "urde::DGRP"))
                {
                    resTag.type = SBIG('DGRP');
                    return true;
                }
                else if (!strcmp(className, "urde::DNAMP1::STRG"))
                {
                    resTag.type = SBIG('STRG');
                    return true;
                }
                else if (!strcmp(className, "DataSpec::DNAMP1::CTweakPlayerRes") ||
                         !strcmp(className, "DataSpec::DNAMP1::CTweakGunRes"))
                {
                    resTag.type = SBIG('CTWK');
                    return true;
                }
                else if (!strcmp(className, "DataSpec::DNAMP1::HINT"))
                {
                    resTag.type = SBIG('HINT');
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
