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

void ProjectResourceFactory::RecursiveAddDirObjects(const HECL::ProjectPath& path)
{
    HECL::DirectoryEnumerator de = path.enumerateDir();
    for (const HECL::DirectoryEnumerator::Entry& ent : de)
    {
        if (ent.m_isDir)
            RecursiveAddDirObjects(HECL::ProjectPath(path, ent.m_name));
        if (ent.m_name.size() == 13 && ent.m_name[4] == _S('_'))
        {
            HECL::SystemUTF8View entu8(ent.m_name);
            u32 id = strtoul(entu8.c_str() + 5, nullptr, 16);
            if (id)
            {
                pshag::SObjectTag objTag = {HECL::FourCC(entu8.c_str()), id};
                if (m_resPaths.find(objTag) == m_resPaths.end())
                    m_resPaths[objTag] = HECL::ProjectPath(path, ent.m_name);
            }
        }
    }
}

void ProjectResourceFactory::BuildObjectMap(const HECL::Database::Project::ProjectDataSpec& spec)
{
    m_resPaths.clear();
    RecursiveAddDirObjects(spec.cookedPath);
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

const pshag::SObjectTag* ProjectResourceFactory::GetResourceIdByName(const char*) const
{
    return nullptr;
}

}
