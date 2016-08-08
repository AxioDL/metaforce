#ifndef __URDE_CSCRIPTDISTANCEFOG_HPP__
#define __URDE_CSCRIPTDISTANCEFOG_HPP__

#include "Graphics/CGraphics.hpp"
#include "CEntity.hpp"

namespace urde
{
class CScriptDistanceFog : public CEntity
{
    ERglFogMode x34_mode;
    zeus::CColor x38_col;
    zeus::CVector2f x3c_;
    float x44_;
    zeus::CVector2f x48_;
    float x50_;
    float x54_;
    float x58_;
    float x5c_;
    bool x60_;
    bool x61_;
public:
    CScriptDistanceFog(TUniqueId, const std::string&, const CEntityInfo&, const ERglFogMode&,
                       const zeus::CColor&, const zeus::CVector2f&, float, const zeus::CVector2f&,
                       bool, bool, float, float, float, float);
};
}

#endif // __URDE_CSCRIPTDISTANCEFOG_HPP__
