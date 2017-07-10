#include "CBodyStateCmdMgr.hpp"

namespace urde
{

CBodyStateCmdMgr::CBodyStateCmdMgr()
{
    x40_commandTable.push_back(&xb8_getup);
    x40_commandTable.push_back(&xc4_step);
    x40_commandTable.push_back(&xd4_);
    x40_commandTable.push_back(&xdc_knockDown);
    x40_commandTable.push_back(&xf4_knockBack);
    x40_commandTable.push_back(&x10c_meleeAttack);
    x40_commandTable.push_back(&x128_projectileAttack);
    x40_commandTable.push_back(&x144_loopAttack);
    x40_commandTable.push_back(&x154_loopReaction);
    x40_commandTable.push_back(&x160_loopHitReaction);
    x40_commandTable.push_back(&x16c_);
    x40_commandTable.push_back(&x174_);
    x40_commandTable.push_back(&x17c_);
    x40_commandTable.push_back(&x184_);
    x40_commandTable.push_back(&x18c_generate);
    x40_commandTable.push_back(&x1ac_hurled);
    x40_commandTable.push_back(&x1d0_jump);
    x40_commandTable.push_back(&x1f8_slide);
    x40_commandTable.push_back(&x210_taunt);
    x40_commandTable.push_back(&x21c_scripted);
    x40_commandTable.push_back(&x230_cover);
    x40_commandTable.push_back(&x254_wallHang);
    x40_commandTable.push_back(&x260_);
    x40_commandTable.push_back(&x268_);
    x40_commandTable.push_back(&x270_additiveAim);
    x40_commandTable.push_back(&x278_additiveFlinch);
    x40_commandTable.push_back(&x284_additiveReaction);
    x40_commandTable.push_back(&x298_);
}

void CBodyStateCmdMgr::DeliverCmd(const CBCLocomotionCmd& cmd)
{
    if (cmd.GetWeight() <= FLT_EPSILON)
        return;
    x3c_steeringSpeed += cmd.GetWeight();
    x0_move += cmd.GetMoveVector() * cmd.GetWeight();
    xc_face += cmd.GetFaceVector() * cmd.GetWeight();
}

void CBodyStateCmdMgr::BlendSteeringCmds()
{
    if (x3c_steeringSpeed > FLT_EPSILON)
    {
        float stepMul = 1.f / x3c_steeringSpeed;
        xc_face *= stepMul;

        switch (x30_steeringMode)
        {
        case ESteeringBlendMode::Normal:
            x0_move *= stepMul;
            break;
        case ESteeringBlendMode::FullSpeed:
            if (!zeus::close_enough(x0_move, zeus::CVector3f::skZero, 0.0001f))
            {
                x0_move.normalize();
                x0_move *= x38_steeringSpeedMax;
            }
            break;
        case ESteeringBlendMode::Clamped:
            x0_move *= stepMul;
            if (!zeus::close_enough(x0_move, zeus::CVector3f::skZero, 0.0001f))
            {
                if (x0_move.magnitude() < x34_steeringSpeedMin)
                    x0_move = x0_move.normalized() * x34_steeringSpeedMin;
                else if (x0_move.magnitude() > x38_steeringSpeedMax)
                    x0_move = x0_move.normalized() * x38_steeringSpeedMax;
            }
            break;
        default: break;
        }
    }
}

void CBodyStateCmdMgr::Reset()
{
    x0_move = zeus::CVector3f::skZero;
    xc_face = zeus::CVector3f::skZero;
    x18_target = zeus::CVector3f::skZero;
    x3c_steeringSpeed = 0.f;
    xb4_deliveredCmdMask = 0;
}

void CBodyStateCmdMgr::ClearLocomotionCmds()
{
    x0_move = zeus::CVector3f::skZero;
    xc_face = zeus::CVector3f::skZero;
    x3c_steeringSpeed = 0.f;
}

}
