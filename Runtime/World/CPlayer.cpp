#include "CPlayer.hpp"
#include "CActorParameters.hpp"
#include "CMorphBall.hpp"

namespace urde
{

static CModelData MakePlayerAnimRes(ResId resId, const zeus::CVector3f& scale)
{
    return CAnimRes(resId, 0, scale, 0, true);
}

CPlayer::CPlayer(TUniqueId uid, const zeus::CTransform& xf, const zeus::CAABox& aabb, unsigned int resId,
                 const zeus::CVector3f& playerScale, float f1, float f2, float f3, float f4, const CMaterialList& ml)
: CPhysicsActor(uid, true, "CPlayer", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId),
                xf, MakePlayerAnimRes(resId, playerScale), ml, aabb, SMoverData(f1), CActorParameters::None(), f2, f3)
{
    x768_morphball.reset(new CMorphBall(*this, f4));
}

}
