#ifndef __URDE_CPLAYER_HPP__
#define __URDE_CPLAYER_HPP__

#include "CPhysicsActor.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CMaterialList;
class CMorphBall;

class CPlayer : public CPhysicsActor
{
    std::unique_ptr<CMorphBall> x768_morphball;

public:
    CPlayer(TUniqueId, const zeus::CTransform&, const zeus::CAABox&, unsigned int,
            const zeus::CVector3f&, float, float, float, float, const CMaterialList&);
};

}

#endif // __URDE_CPLAYER_HPP__
