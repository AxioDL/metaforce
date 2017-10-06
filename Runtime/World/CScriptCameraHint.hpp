#ifndef __URDE_CSCRIPTCAMERAHINT_HPP__
#define __URDE_CSCRIPTCAMERAHINT_HPP__

#include "CActor.hpp"
#include "Camera/CBallCamera.hpp"

namespace urde
{

class CCameraHint
{
    s32 x4_overrideFlags;
    CBallCamera::EBallCameraBehaviour x8_behaviour;
    float xc_f1;
    float x10_f2;
    float x14_f3;
    zeus::CVector3f x18_r6;
    zeus::CVector3f x24_r7;
    zeus::CVector3f x30_r8;
    float x3c_f4;
    float x40_f5;
    float x44_f6;
    float x48_f7;
    float x4c_f8;
    float x50_f9;
    float x54_f10;
    float x58_f11;
    float x5c_f12;
    float x60_f13;

public:
    CCameraHint(s32 overrideFlags, CBallCamera::EBallCameraBehaviour behaviour, float f1, float f2, float f3,
                const zeus::CVector3f& r6, const zeus::CVector3f& r7, const zeus::CVector3f& r8, float f4,
                float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12, float f13)
    : x4_overrideFlags(overrideFlags), x8_behaviour(behaviour), xc_f1(f1), x10_f2(f2), x14_f3(f3), x18_r6(r6),
      x24_r7(r7), x30_r8(r8), x3c_f4(f4), x40_f5(f5), x44_f6(f6), x48_f7(f7), x4c_f8(f8), x50_f9(f9), x54_f10(f10),
      x58_f11(f11), x5c_f12(f12), x60_f13(f13) {}

    s32 GetOverrideFlags() const { return x4_overrideFlags; }
    CBallCamera::EBallCameraBehaviour GetBehaviourType() const { return x8_behaviour; }
    float GetX4C() const { return x4c_f8; }
    float GetX50() const { return x50_f9; }
    float GetX58() const { return x58_f11; }
    float GetX5C() const { return x5c_f12; }
};

class CScriptCameraHint : public CActor
{
    s32 xe8_priority;
    CCameraHint xec_hint;
    rstl::reserved_vector<TUniqueId, 8> x150_helpers;
    TUniqueId x164_delegatedCamera = kInvalidUniqueId;
    bool x166_inactive = false;
    zeus::CTransform x168_origXf;
    void InitializeInArea(CStateManager& mgr);
    void AddHelper(TUniqueId id);
    void RemoveHelper(TUniqueId id);
public:
    CScriptCameraHint(TUniqueId, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                      bool active, s32 priority, CBallCamera::EBallCameraBehaviour behaviour, s32 overrideFlags,
                      float f1, float f2, float f3, const zeus::CVector3f& r6, const zeus::CVector3f& r7,
                      const zeus::CVector3f& r8, float f4, float f5, float f6, float f7, float f8, float f9, float f10,
                      float f11, float f12, float f13);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);

    void ClearIdList() { x150_helpers.clear(); }
    void SetInactive(bool inactive) { x166_inactive = inactive; }
    bool GetInactive() const { return x166_inactive; }
    size_t GetHelperCount() const { return x150_helpers.size(); }
    TUniqueId GetFirstHelper() const { return x150_helpers.empty() ? kInvalidUniqueId : x150_helpers[0]; }
    s32 GetPriority() const { return xe8_priority; }
    const CCameraHint& GetHint() const { return xec_hint; }
    TUniqueId GetDelegatedCamera() const { return x164_delegatedCamera; }
};
}

#endif // __URDE_CSCRIPTCAMERAHINT_HPP__
