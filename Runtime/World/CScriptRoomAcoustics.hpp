#ifndef __URDE_CSCRIPTROOMACOUSTICS_HPP__
#define __URDE_CSCRIPTROOMACOUSTICS_HPP__

#include "CEntity.hpp"
#include "amuse/EffectChorus.hpp"
#include "amuse/EffectDelay.hpp"
#include "amuse/EffectReverb.hpp"

namespace urde
{

class CScriptRoomAcoustics : public CEntity
{
    u32 x34_volumeScale;

    bool x38_revHi, x39_revHiDis;
    amuse::EffectReverbHiInfo x3c_revHiInfo;

    bool x54_chorus;
    amuse::EffectChorusInfo x58_chorusInfo;

    bool x64_revStd, x65_revStdDis;
    amuse::EffectReverbStdInfo x68_revStdInfo;

    bool x7c_delay;
    amuse::EffectDelayInfo x80_delayInfo;

public:
    CScriptRoomAcoustics(TUniqueId uid, std::string_view name,
                         const CEntityInfo& info, bool active, u32 volScale,
                         bool revHi, bool revHiDis, float revHiColoration, float revHiMix,
                         float revHiTime, float revHiDamping, float revHiPreDelay, float revHiCrosstalk,
                         bool chorus, float baseDelay, float variation, float period,
                         bool revStd, bool revStdDis, float revStdColoration, float revStdMix,
                         float revStdTime, float revStdDamping, float revStdPreDelay,
                         bool delay, u32 delayL, u32 delayR, u32 delayS,
                         u32 feedbackL, u32 feedbackR, u32 feedbackS,
                         u32 outputL, u32 outputR, u32 outputS);
    void Think(float dt, CStateManager& stateMgr);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    void Accept(IVisitor& visitor);
    void EnableAuxCallbacks();

    static void DisableAuxCallbacks();
};

}

#endif // __URDE_CSCRIPTROOMACOUSTICS_HPP__
