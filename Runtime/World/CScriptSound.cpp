#include "CScriptSound.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{
bool CScriptSound::sFirstInFrame = false;

CScriptSound::CScriptSound(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                           s16 soundId, bool active, float f1, float f2, float f3, u32 w1, u32 w2, u32 w3, u32 w4,
                           u32 w5, u32 w6, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, u32 w7)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Unknown),
         CActorParameters::None(), kInvalidUniqueId)
, xfc_(f3)
, x100_soundId(CSfxManager::TranslateSFXID(soundId))
, x104_(f1)
, x108_(f2)
, x10c_(w1)
, x10e_(w2)
, x110_(w3)
, x112_(w4)
, x114_(w5)
, x116_(w6)
, x118_(w7 + 8192)
{
    x11c_25_ = b1;
    x11c_26_ = b2;
    x11c_27_ = b3;
    x11c_28_ = b4;
    x11c_29_ = b5;
    x11c_30_ = b6;
    x11d_24_ = b7;
    if (x11c_30_ && (!x11c_26_ || !x11c_25_))
        x11c_30_ = false;
}

void CScriptSound::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSound::PreThink(float dt, CStateManager& mgr)
{
    CEntity::PreThink(dt, mgr);
    sFirstInFrame = true;
    x11d_25_ = false;
}

void CScriptSound::Think(float dt, CStateManager& mgr)
{
}

void CScriptSound::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);

    switch (msg)
    {
    case EScriptObjectMessage::Registered:
    {
    }
    break;
    case EScriptObjectMessage::Play:
    {
    }
    break;
    case EScriptObjectMessage::Stop:
    {
    }
    break;
    case EScriptObjectMessage::Deactivate:
    {
    }
    break;
    case EScriptObjectMessage::Activate:
    {
    }
    break;
    case EScriptObjectMessage::Deleted:
    {
    }
    break;
    default:break;
    }
}

void CScriptSound::PlaySound(CStateManager&) {}

void CScriptSound::StopSound(CStateManager& mgr)
{
    x11c_24_playing = false;
    if (x11c_30_ && x11c_26_)
    {
        mgr.WorldNC()->StopSound(x100_soundId);
        xec_sfxHandle.reset();
    }
    else if (xec_sfxHandle)
    {
        CSfxManager::RemoveEmitter(*xec_sfxHandle.get());
        xec_sfxHandle.reset();
    }
}
}
