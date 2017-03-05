#ifndef __URDE_CMORPHBALLSHADOW_HPP__
#define __URDE_CMORPHBALLSHADOW_HPP__

#include "CToken.hpp"
#include "Graphics/CTexture.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CStateManager;
class CPlayer;
class CGameArea;
class CActor;

class CMorphBallShadow
{
    std::list<const CActor*> x0_actors;
    std::list<TAreaId> x18_areas;
    std::vector<u32> x30_worldModelBits;
    //CTexture x40_;
    //TToken<CTexture> xa8_ballFade;
    //int xb0_idW;
    //int xb4_idH;
    zeus::CAABox xb8_;
    bool xd0_hasIds = false;
    void GatherAreas(CStateManager& mgr);
    bool AreasValid(CStateManager& mgr) const;
public:
    void RenderIdBuffer(const zeus::CAABox& aabb, CStateManager& mgr, CPlayer& player);
    void Render(CStateManager& mgr, float alpha);
};

}

#endif // __URDE_CMORPHBALLSHADOW_HPP__
