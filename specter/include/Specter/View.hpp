#ifndef SPECTER_VIEW_HPP
#define SPECTER_VIEW_HPP

#include <boo/boo.hpp>

namespace Specter
{

class View
{
    boo::SWindowRect m_rect;
public:
    void bindViewport(boo::IGraphicsCommandQueue* gfxQ)
    {
        gfxQ->setViewport(m_rect);
    }
    virtual void draw(boo::IGraphicsCommandQueue* gfxQ)=0;
};

}

#endif // SPECTER_VIEW_HPP
