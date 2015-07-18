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
    static ResExtractor LookupExtractor(const PAK::Entry& entry);
public:
    PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node);
    const std::string& getName() const {return m_node.getName();}
    std::string getLevelString() const;
    bool extractResources(const HECL::ProjectPath& dirOut,
                          const HECL::ProjectPath& cookedOut,
                          bool force);
};

}
}

#endif // __DNAMP3_HPP__
