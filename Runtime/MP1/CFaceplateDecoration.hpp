#ifndef __URDE_CFACEPLATEDECORATION_HPP__
#define __URDE_CFACEPLATEDECORATION_HPP__

#include "RetroTypes.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"

namespace urde
{
class CStateManager;

namespace MP1
{

class CFaceplateDecoration
{
    ResId x0_id;
    TToken<CTexture> x4_tex;
    bool xc_ready = false;
    std::experimental::optional<CTexturedQuadFilter> m_texFilter;
public:
    CFaceplateDecoration(CStateManager& stateMgr);
    void Update(float dt, CStateManager& stateMgr);
    void Draw(CStateManager& stateMgr);
};

}
}

#endif // __URDE_CFACEPLATEDECORATION_HPP__
