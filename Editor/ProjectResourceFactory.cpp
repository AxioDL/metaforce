#include "ProjectResourceFactory.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CTexture.hpp"

namespace urde
{

ProjectResourceFactory::ProjectResourceFactory()
{
    m_factoryMgr.AddFactory(FOURCC('TXTR'), urde::FTextureFactory);
    m_factoryMgr.AddFactory(FOURCC('PART'), urde::FParticleFactory);
    m_factoryMgr.AddFactory(FOURCC('FRME'), urde::RGuiFrameFactoryInGame);
    m_factoryMgr.AddFactory(FOURCC('FONT'), urde::FRasterFontFactory);
}

void ProjectResourceFactory::BuildObjectMap(const hecl::Database::Project::ProjectDataSpec &spec)
{
#if 0
    m_tagToPath.clear();
    m_catalogNameToTag.clear();
    hecl::SystemString catalogPath = hecl::ProjectPath(spec.cookedPath, hecl::SystemString(spec.spec.m_name) + _S("/catalog.yaml")).getAbsolutePath();
    FILE* catalogFile = hecl::Fopen(catalogPath.c_str(), _S("r"));
    athena::io::YAMLDocReader reader;
    yaml_parser_set_input_file(reader.getParser(), catalogFile);
    reader.parse();
    const athena::io::YAMLNode* catalogRoot = reader.getRootNode();
    if (catalogRoot)
    {
        m_catalogNameToPath.reserve(catalogRoot->m_mapChildren.size());
        for (const std::pair<std::string, std::unique_ptr<athena::io::YAMLNode>>& ch : catalogRoot->m_mapChildren)
        {
            if (ch.second->m_type == YAML_SCALAR_NODE)
                m_catalogNameToPath[ch.first] = hecl::ProjectPath(spec.cookedPath, hecl::SystemString(ch.second->m_scalarString));
        }
    }

    if (!hecl::StrCmp(spec.spec.m_name, _S("MP3")))
    {
        RecursiveAddDirObjects(spec.cookedPath);
    }
    else
    {
        RecursiveAddDirObjects(spec.cookedPath);
    }
#endif
}

std::unique_ptr<urde::IObj> ProjectResourceFactory::Build(const urde::SObjectTag& tag,
                                                          const urde::CVParamTransfer& paramXfer)
{
    auto search = m_tagToPath.find(tag);
    if (search == m_tagToPath.end())
        return {};

    //fprintf(stderr, "Loading resource %s\n", search->second.getRelativePath().c_str());
    athena::io::FileReader fr(search->second.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
        return {};

    return m_factoryMgr.MakeObject(tag, fr, paramXfer);
}

void ProjectResourceFactory::BuildAsync(const urde::SObjectTag& tag,
                                        const urde::CVParamTransfer& paramXfer,
                                        urde::IObj** objOut)
{
    std::unique_ptr<urde::IObj> obj = Build(tag, paramXfer);
    *objOut = obj.release();
}

void ProjectResourceFactory::CancelBuild(const urde::SObjectTag&)
{
}

bool ProjectResourceFactory::CanBuild(const urde::SObjectTag& tag)
{
    auto search = m_tagToPath.find(tag);
    if (search == m_tagToPath.end())
        return false;

    athena::io::FileReader fr(search->second.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
        return false;

    return true;
}

const urde::SObjectTag* ProjectResourceFactory::GetResourceIdByName(const char* name) const
{
    if (m_catalogNameToTag.find(name) == m_catalogNameToTag.end())
        return nullptr;
    const urde::SObjectTag& tag = m_catalogNameToTag.at(name);
    return &tag;
}

}
