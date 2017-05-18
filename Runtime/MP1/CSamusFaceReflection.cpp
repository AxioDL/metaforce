#include "CSamusFaceReflection.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
namespace MP1
{

CSamusFaceReflection::CSamusFaceReflection(CStateManager& stateMgr)
: x0_modelData(CAnimRes(g_ResFactory->GetResourceIdByName("ACS_SamusFace")->id,
                        0, zeus::CVector3f::skOne, 0, true)),
  x4c_lights(std::make_unique<CActorLights>(8, zeus::CVector3f::skZero, 4, 4, false, 0, 0, 0.1f))
{
    x60_ = zeus::CVector3f::skForward;
    CAnimPlaybackParms parms(0, -1, 1.f, true);
    x0_modelData.AnimationData()->SetAnimation(parms, false);
}

void CSamusFaceReflection::PreDraw(const CStateManager& stateMgr)
{

}

void CSamusFaceReflection::Update(float dt, const CStateManager& stateMgr, CRandom16& rand)
{

}

}
}
