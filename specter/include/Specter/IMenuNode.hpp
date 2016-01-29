#ifndef SPECTER_IMENUNODE_HPP
#define SPECTER_IMENUNODE_HPP

#include "View.hpp"

namespace Specter
{

struct IMenuNode
{
    virtual boo::ITexture* icon() const {return nullptr;}
    virtual const std::string* text() const {return nullptr;}
    virtual size_t subNodeCount() const {return 0;}
    virtual IMenuNode* subNode(size_t idx) {return nullptr;}
    virtual void activated(const boo::SWindowCoord& coord) {}
};

}

#endif // SPECTER_IMENUNODE_HPP
