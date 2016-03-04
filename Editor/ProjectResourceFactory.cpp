#include "ProjectResourceFactory.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CTexture.hpp"

namespace URDE
{

ProjectResourceFactory::ProjectResourceFactory()
{
    m_factoryMgr.AddFactory(hecl::FOURCC('TXTR'), urde::FTextureFactory);
    m_factoryMgr.AddFactory(hecl::FOURCC('PART'), urde::FParticleFactory);
}

void ProjectResourceFactory::BuildObjectMap(const hecl::Database::Project::ProjectDataSpec &spec)
{
    m_resPaths.clear();
    m_namedResources.clear();
    hecl::SystemString catalogPath = hecl::ProjectPath(spec.cookedPath, hecl::SystemString(spec.spec.m_name) + _S("/catalog.yaml")).getAbsolutePath();
    FILE* catalogFile = hecl::Fopen(catalogPath.c_str(), _S("r"));
    if (!hecl::StrCmp(spec.spec.m_name, _S("MP3")))
    {
        DataSpec::NamedResourceCatalog<DataSpec::UniqueID64> catalog;
        if (catalogFile)
            catalog.fromYAMLFile(catalogFile);
        RecursiveAddDirObjects(spec.cookedPath, catalog);
    }
    else
    {
        DataSpec::NamedResourceCatalog<DataSpec::UniqueID32> catalog;
        if (catalogFile)
            catalog.fromYAMLFile(catalogFile);
        RecursiveAddDirObjects(spec.cookedPath, catalog);
    }
}

std::unique_ptr<urde::IObj> ProjectResourceFactory::Build(const urde::SObjectTag& tag,
                                                           const urde::CVParamTransfer& paramXfer)
{
    auto search = m_resPaths.find(tag);
    if (search == m_resPaths.end())
        return {};

    fprintf(stderr, "Loading resource %s\n", search->second.getRelativePath().c_str());
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
    auto search = m_resPaths.find(tag);
    if (search == m_resPaths.end())
        return false;

    athena::io::FileReader fr(search->second.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
        return false;

    return true;
}

const urde::SObjectTag* ProjectResourceFactory::GetResourceIdByName(const char* name) const
{
    if (m_namedResources.find(name) == m_namedResources.end())
        return nullptr;
    const urde::SObjectTag& tag = m_namedResources.at(name);
    return &tag;
}

}
