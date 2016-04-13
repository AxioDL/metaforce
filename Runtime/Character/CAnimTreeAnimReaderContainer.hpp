#ifndef __URDE_CANIMTREEANIMREADERCONTAINER_HPP__
#define __URDE_CANIMTREEANIMREADERCONTAINER_HPP__

#include "CAnimTreeNode.hpp"

namespace urde
{

class CAnimTreeAnimReaderContainer : public CAnimTreeNode
{
    std::shared_ptr<IAnimReader> x14_
public:
    CAnimTreeAnimReaderContainer(const std::string& name, std::shared_ptr<>);
};

}

#endif // __URDE_CANIMTREEANIMREADERCONTAINER_HPP__
