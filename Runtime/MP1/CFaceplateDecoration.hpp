#pragma once

#include "RetroTypes.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"

namespace urde
{
class CStateManager;

namespace MP1
{

class CFaceplateDecoration
{
    CAssetId x0_id;
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

