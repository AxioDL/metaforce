#ifndef URDE_PROJECT_RESOURCE_FACTORY_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_HPP

#include "Runtime/IFactory.hpp"
#include "Runtime/CFactoryMgr.hpp"

namespace urde
{

class ProjectResourceFactory : public urde::IFactory
{
    std::unordered_map<urde::SObjectTag, hecl::ProjectPath> m_tagToPath;
    std::unordered_map<std::string, urde::SObjectTag> m_catalogNameToTag;
    std::unordered_map<std::string, hecl::ProjectPath> m_catalogNameToPath;
    urde::CFactoryMgr m_factoryMgr;
    void RecursiveAddDirObjects(const hecl::ProjectPath& path)
    {
        hecl::DirectoryEnumerator de = path.enumerateDir();
        const int idLen = 5 + 8;
        for (const hecl::DirectoryEnumerator::Entry& ent : de)
        {
            if (ent.m_isDir)
                RecursiveAddDirObjects(hecl::ProjectPath(path, ent.m_name));
            if (ent.m_name.size() == idLen && ent.m_name[4] == _S('_'))
            {
                hecl::SystemUTF8View entu8(ent.m_name);
#if _WIN32
                u64 id = _strtoui64(entu8.c_str() + 5, nullptr, 16);
#else
                u64 id = strtouq(entu8.c_str() + 5, nullptr, 16);
#endif

                if (id)
                {
                    urde::SObjectTag objTag = {hecl::FourCC(entu8.c_str()), id};
                    if (m_tagToPath.find(objTag) == m_tagToPath.end())
                        m_tagToPath[objTag] = hecl::ProjectPath(path, ent.m_name);
                }
            }
            else
            {
#if 0
                hecl::SystemUTF8View nameView(ent.m_name);
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
    void BuildObjectMap(const hecl::Database::Project::ProjectDataSpec& spec);

    std::unique_ptr<urde::IObj> Build(const urde::SObjectTag&, const urde::CVParamTransfer&);
    void BuildAsync(const urde::SObjectTag&, const urde::CVParamTransfer&, urde::IObj**);
    void CancelBuild(const urde::SObjectTag&);
    bool CanBuild(const urde::SObjectTag&);
    const urde::SObjectTag* GetResourceIdByName(const char*) const;
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_HPP
