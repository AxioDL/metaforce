#ifndef __URDE_CGAMECUBEDOLL_HPP__
#define __URDE_CGAMECUBEDOLL_HPP__

#include "RetroTypes.hpp"
#include "Character/CActorLights.hpp"
#include "CToken.hpp"

namespace urde
{
class CModel;
namespace MP1
{

class CGameCubeDoll
{
    TLockedToken<CModel> x0_model;
    std::vector<CLight> x8_lights;
    std::unique_ptr<CActorLights> x18_actorLights;
    float x1c_fader = 0.f;
    bool x20_24_loaded : 1;
    void UpdateActorLights();
public:
    CGameCubeDoll();
    void Update(float dt);
    void Draw(float alpha);
    void Touch();
    bool CheckLoadComplete();
    bool IsLoaded() const { return x20_24_loaded; }

};

}
}

#endif // __URDE_CGAMECUBEDOLL_HPP__
