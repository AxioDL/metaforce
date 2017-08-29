#ifndef __URDE_CSCRIPTSOUND_HPP__
#define __URDE_CSCRIPTSOUND_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptSound : public CActor
{
    static bool sFirstInFrame;

    float xe8_ = 0.f;
    std::unique_ptr<CSfxHandle> xec_sfxHandle;
    s16 xf0_ = 0;
    s16 xf4_ = 0;
    float xf8_ = 0.f;
    float xfc_;
    s16 x100_soundId;
    float x104_;
    float x108_;
    s16 x10c_;
    s16 x10e_;
    s16 x110_;
    s16 x112_;
    s16 x114_;
    bool x116_;
    u32 x118_;
    union
    {
        struct
        {
            bool x11c_24_ : 1;
            bool x11c_25_ : 1;
            bool x11c_26_ : 1;
            bool x11c_27_autoStart : 1;
            bool x11c_28_ : 1;
            bool x11c_29_ : 1;
            bool x11c_30_ : 1;
            bool x11c_31_ : 1;
            bool x11d_24_ : 1;
            bool x11d_25_ : 1;
        };
        u32 x11c_dummy = 0;
    };

public:
    CScriptSound(TUniqueId, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf, s16 soundId,
                 bool, float, float, float, u32, u32, u32, u32, u32, u32, bool, bool, bool, bool, bool, bool, bool, u32);

    void Accept(IVisitor& visitor);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const {}
    void PreThink(float, CStateManager&);
    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void GetOccludedVolumeAmount(const zeus::CVector3f&, const CStateManager&);
    void PlaySound(CStateManager&);
    void StopSound(CStateManager&);
};
}

#endif // __URDE_CSCRIPTSOUND_HPP__
