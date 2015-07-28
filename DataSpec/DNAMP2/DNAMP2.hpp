#ifndef __DNAMP2_HPP__
#define __DNAMP2_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "../DNAMP1/PAK.hpp"

namespace Retro
{
namespace DNAMP2
{

extern LogVisor::LogModule Log;

/* MP2-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    HECL::Database::Project& m_project;
    const NOD::DiscBase::IPartition::Node& m_node;
    DNAMP1::PAK m_pak;
public:
    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    static ResExtractor LookupExtractor(const DNAMP1::PAK::Entry& entry);
    const std::string& getName() const {return m_node.getName();}
    HECL::SystemString getLevelString() const;
    bool extractResources(const PAKRouter<PAKBridge>& router,
                          bool force,
                          std::function<void(float)> progress);

    typedef DNAMP1::PAK PAKType;
    const PAKType& getPAK() const {return m_pak;}
};

}
}

#endif // __DNAMP2_HPP__
