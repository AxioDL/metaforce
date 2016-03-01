#ifndef URDE_PROJECT_RESOURCE_FACTORY_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_HPP

#include "Runtime/IFactory.hpp"
#include "Runtime/CFactoryMgr.hpp"

namespace URDE
{

class ProjectResourceFactory : public pshag::IFactory
{
    std::unordered_map<pshag::SObjectTag, HECL::ProjectPath> m_tagToPath;
    std::unordered_map<std::string, pshag::SObjectTag> m_catalogNameToTag;
    std::unordered_map<std::string, HECL::ProjectPath> m_catalogNameToPath;
    pshag::CFactoryMgr m_factoryMgr;
    void RecursiveAddDirObjects(const HECL::ProjectPath& path)
    {
        HECL::DirectoryEnumerator de = path.enumerateDir();
        const int idLen = 5 + 8;
        for (const HECL::DirectoryEnumerator::Entry& ent : de)
        {
            if (ent.m_isDir)
                RecursiveAddDirObjects(HECL::ProjectPath(path, ent.m_name));
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
                    if (m_tagToPath.find(objTag) == m_tagToPath.end())
                        m_tagToPath[objTag] = HECL::ProjectPath(path, ent.m_name);
                }
            }
            else
            {
#if 0
                HECL::SystemUTF8View nameView(ent.m_name);
                auto it = std::find_if(catalog.namedResources.begin(), catalog.namedResources.end(),
                                       [&nameView](const typename DataSpec::NamedResourceCatalog<IDType>::NamedResource& res) -> bool
                { return res.name == nameView.str(); });
                if (it == catalog.namedResources.end())
                    continue;

                const typename DataSpec::NamedResourceCatalog<IDType>::NamedResource& nr = *it;
                pshag::SObjectTag objTag = GetTag<IDType>(nr);

                m_catalogNameToTag[nr.name.c_str()] = objTag;
                m_tagToPath[objTag] = HECL::ProjectPath(path, ent.m_name);
#endif
            }
        }
    }

#if 0
    template <class IDType>
    pshag::SObjectTag GetTag(const DataSpec::NamedResourceCatalog<DataSpec::UniqueID32>::NamedResource &nr,
                typename std::enable_if<std::is_same<IDType, DataSpec::UniqueID32>::value>::type* = 0)
    { return { nr.type, nr.uid.toUint32() }; }

    template <class IDType>
    pshag::SObjectTag GetTag(const typename DataSpec::NamedResourceCatalog<IDType>::NamedResource& nr,
                typename std::enable_if<std::is_same<IDType, DataSpec::UniqueID64>::value>::type* = 0)
    { return { nr.type, nr.uid.toUint64() }; }
#endif

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
