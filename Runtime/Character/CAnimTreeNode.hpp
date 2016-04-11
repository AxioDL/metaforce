#ifndef __PSHAG_CANIMTREENODE_HPP__
#define __PSHAG_CANIMTREENODE_HPP__

#include "IAnimReader.hpp"

namespace urde
{

class CAnimTreeNode : public IAnimReader
{
    std::string x4_name;
public:
    CAnimTreeNode(const std::string& name) : x4_name(name) {}
    bool IsCAnimTreeNode() const {return true;}
};

}

#endif // __PSHAG_CANIMTREENODE_HPP__
