#ifndef CSCRIPTCAMERAFILTERKEYFRAME_HPP
#define CSCRIPTCAMERAFILTERKEYFRAME_HPP

#include "CEntity.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
class CScriptCameraFilterKeyframe : public CEntity
{
    u32 x34_;
    u32 x38_;
    u32 x3c_;
    u32 x40_;
    zeus::CColor x44_;
    float x48_;
    float x4c_;
    u32 x50_;
public:
    CScriptCameraFilterKeyframe(TUniqueId, const std::string&, const CEntityInfo&, u32, u32, u32, u32, const zeus::CColor&,
                                float, float, u32, bool);
};
}

#endif // CSCRIPTCAMERAFILTERKEYFRAME_HPP
