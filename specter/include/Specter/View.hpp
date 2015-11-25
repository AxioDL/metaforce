#ifndef SPECTER_VIEW_HPP
#define SPECTER_VIEW_HPP

#include <boo/boo.hpp>
#include "CVector3f.hpp"
#include "CMatrix4f.hpp"
#include "CTransform.hpp"
#include "CColor.hpp"

namespace Specter
{

class View
{
protected:
    boo::SWindowRect m_viewport;
    void bindViewport(boo::IGraphicsCommandQueue* gfxQ) {gfxQ->setViewport(m_viewport);}
public:
    virtual void draw(boo::IGraphicsCommandQueue* gfxQ)=0;
};

}

#endif // SPECTER_VIEW_HPP
