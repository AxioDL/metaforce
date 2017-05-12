#ifndef __URDE_CARTIFACTDOLL_HPP__
#define __URDE_CARTIFACTDOLL_HPP__

#include "RetroTypes.hpp"
#include "Character/CActorLights.hpp"
#include "CToken.hpp"

namespace urde
{
class CModel;
namespace MP1
{

class CArtifactDoll
{
    std::vector<TLockedToken<CModel>> x0_models;
    std::vector<CLight> x10_lights;
    std::unique_ptr<CActorLights> x20_actorLights;
    float x24_fader = 0.f;
    bool x28_24_loaded : 1;
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

#endif // __URDE_CARTIFACTDOLL_HPP__
