#ifndef CSCRIPTCAMERAFILTERKEYFRAME_HPP
#define CSCRIPTCAMERAFILTERKEYFRAME_HPP

#include "CEntity.hpp"
#include "zeus/CColor.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{
class CScriptCameraFilterKeyframe : public CEntity
{
    EFilterType x34_type;
    EFilterShape x38_shape;
    u32 x3c_filterIdx;
    u32 x40_;
    zeus::CColor x44_color;
    float x48_timeIn;
    float x4c_timeOut;
    ResId x50_txtr;

public:
    CScriptCameraFilterKeyframe(TUniqueId uid, const std::string& name,
                                const CEntityInfo& info, EFilterType type, EFilterShape shape,
                                u32 filterIdx, u32 unk, const zeus::CColor& color,
                                float timeIn, float timeOut, ResId txtr, bool active);

    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    void Accept(IVisitor& visitor);
};
}

#endif // CSCRIPTCAMERAFILTERKEYFRAME_HPP
