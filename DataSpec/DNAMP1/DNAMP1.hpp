#ifndef __DNAMP1_HPP__
#define __DNAMP1_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace Retro
{
namespace DNAMP1
{

extern LogVisor::LogModule Log;

/* MP1-specific, one-shot PAK traversal/extraction class */
class PAKBridge
{
    HECL::Database::Project& m_project;
    const NOD::DiscBase::IPartition::Node& m_node;
    PAK m_pak;
    static ResExtractor LookupExtractor(const PAK::Entry& entry);
public:
    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    const std::string& getName() const {return m_node.getName();}
    HECL::SystemString getLevelString() const;
    bool extractResources(const HECL::ProjectPath& dirOut,
                          const HECL::ProjectPath& cookedOut,
                          bool force);
};

}
}

#endif // __DNAMP1_HPP__
