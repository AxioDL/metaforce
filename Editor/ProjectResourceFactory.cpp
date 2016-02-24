#include "ProjectResourceFactory.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/CModel.hpp"
#include "Runtime/CTexture.hpp"

namespace URDE
{

ProjectResourceFactory::ProjectResourceFactory()
{
    m_factoryMgr.AddFactory(HECL::FOURCC('TXTR'), pshag::FTextureFactory);
    m_factoryMgr.AddFactory(HECL::FOURCC('PART'), pshag::FParticleFactory);
}

void ProjectResourceFactory::BuildObjectMap(const HECL::Database::Project::ProjectDataSpec &spec)
{
    m_resPaths.clear();
    m_namedResources.clear();
    HECL::SystemString catalogPath = HECL::ProjectPath(spec.cookedPath, HECL::SystemString(spec.spec.m_name) + _S("/catalog.yaml")).getAbsolutePath();
    FILE* catalogFile = HECL::Fopen(catalogPath.c_str(), _S("r"));
    if (!HECL::StrCmp(spec.spec.m_name, _S("MP3")))
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

std::unique_ptr<pshag::IObj> ProjectResourceFactory::Build(const pshag::SObjectTag& tag,
                                                           const pshag::CVParamTransfer& paramXfer)
{
    auto search = m_resPaths.find(tag);
    if (search == m_resPaths.end())
        return {};

    Athena::io::FileReader fr(search->second.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
        return {};

    return m_factoryMgr.MakeObject(tag, fr, paramXfer);
}

void ProjectResourceFactory::BuildAsync(const pshag::SObjectTag& tag,
                                        const pshag::CVParamTransfer& paramXfer,
                                        pshag::IObj** objOut)
{
    std::unique_ptr<pshag::IObj> obj = Build(tag, paramXfer);
    *objOut = obj.release();
}

void ProjectResourceFactory::CancelBuild(const pshag::SObjectTag&)
{
}

bool ProjectResourceFactory::CanBuild(const pshag::SObjectTag& tag)
{
    auto search = m_resPaths.find(tag);
    if (search == m_resPaths.end())
        return false;

    Athena::io::FileReader fr(search->second.getAbsolutePath(), 32 * 1024, false);
    if (fr.hasError())
        return false;

    return true;
}

const pshag::SObjectTag* ProjectResourceFactory::GetResourceIdByName(const char* name) const
{
    if (m_namedResources.find(name) == m_namedResources.end())
        return nullptr;
    const pshag::SObjectTag& tag = m_namedResources.at(name);
    return &tag;
}

}
