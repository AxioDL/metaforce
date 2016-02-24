#ifndef URDE_PROJECT_RESOURCE_FACTORY_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_HPP

#include "Runtime/IFactory.hpp"
#include "Runtime/CFactoryMgr.hpp"
#include "DataSpec/DNACommon/NamedResourceCatalog.hpp"

namespace URDE
{

class ProjectResourceFactory : public pshag::IFactory
{
    std::unordered_map<pshag::SObjectTag, HECL::ProjectPath> m_resPaths;
    std::unordered_map<std::string, pshag::SObjectTag> m_namedResources;
    pshag::CFactoryMgr m_factoryMgr;
    template <class IDType>
    void RecursiveAddDirObjects(const HECL::ProjectPath& path, const DataSpec::NamedResourceCatalog<IDType>& catalog)
    {
        HECL::DirectoryEnumerator de = path.enumerateDir();
        const int idLen = 5 + (IDType::BinarySize() * 2);
        for (const HECL::DirectoryEnumerator::Entry& ent : de)
        {
            if (ent.m_isDir)
                RecursiveAddDirObjects(HECL::ProjectPath(path, ent.m_name), catalog);
            if (ent.m_name.size() == idLen && ent.m_name[4] == _S('_'))
            {
                HECL::SystemUTF8View entu8(ent.m_name);
#if _WIN32
                u64 id = _strtoui64(entu8.c_str() + 5, nullptr, 16);
#else
                u64 id = strtouq(entu8.c_str() + 5, nullptr, 16);
#endif

                if (id)
                {
                    pshag::SObjectTag objTag = {HECL::FourCC(entu8.c_str()), id};
                    if (m_resPaths.find(objTag) == m_resPaths.end())
                        m_resPaths[objTag] = HECL::ProjectPath(path, ent.m_name);
                }
            }
            else
            {
                HECL::SystemUTF8View nameView(ent.m_name);
                auto it = std::find_if(catalog.namedResources.begin(), catalog.namedResources.end(),
                                       [&nameView](const typename DataSpec::NamedResourceCatalog<IDType>::NamedResource& res) -> bool
                { return res.name == nameView.str(); });
                if (it == catalog.namedResources.end())
                    continue;

                const typename DataSpec::NamedResourceCatalog<IDType>::NamedResource& nr = *it;
                pshag::SObjectTag objTag = GetTag<IDType>(nr);

                m_namedResources[nr.name.c_str()] = objTag;
                m_resPaths[objTag] = HECL::ProjectPath(path, ent.m_name);
            }
        }
    }

    template <class IDType>
    pshag::SObjectTag GetTag(const DataSpec::NamedResourceCatalog<DataSpec::UniqueID32>::NamedResource &nr,
                typename std::enable_if<std::is_same<IDType, DataSpec::UniqueID32>::value>::type* = 0)
    { return { nr.type, nr.uid.toUint32() }; }

    template <class IDType>
    pshag::SObjectTag GetTag(const typename DataSpec::NamedResourceCatalog<IDType>::NamedResource& nr,
                typename std::enable_if<std::is_same<IDType, DataSpec::UniqueID64>::value>::type* = 0)
    { return { nr.type, nr.uid.toUint64() }; }

public:
    ProjectResourceFactory();
    void BuildObjectMap(const HECL::Database::Project::ProjectDataSpec& spec);

    std::unique_ptr<pshag::IObj> Build(const pshag::SObjectTag&, const pshag::CVParamTransfer&);
    void BuildAsync(const pshag::SObjectTag&, const pshag::CVParamTransfer&, pshag::IObj**);
    void CancelBuild(const pshag::SObjectTag&);
    bool CanBuild(const pshag::SObjectTag&);
    const pshag::SObjectTag* GetResourceIdByName(const char*) const;
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_HPP
