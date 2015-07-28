#ifndef __DNAMP3_HPP__
#define __DNAMP3_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace Retro
{
namespace DNAMP3
{

extern LogVisor::LogModule Log;

/* MP3-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    HECL::Database::Project& m_project;
    const NOD::DiscBase::IPartition::Node& m_node;
    PAK m_pak;
public:
    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    static ResExtractor LookupExtractor(const PAK::Entry& entry);
    const std::string& getName() const {return m_node.getName();}
    HECL::SystemString getLevelString() const;
    bool extractResources(const PAKRouter<PAKBridge>& router,
                          bool force,
                          std::function<void(float)> progress);

    typedef PAK PAKType;
    const PAKType& getPAK() const {return m_pak;}
};

}
}

#endif // __DNAMP3_HPP__
