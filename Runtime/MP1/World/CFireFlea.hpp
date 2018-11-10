#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CFireFlea : public CPatterned
{
   class CDeathCameraEffect : CEntity
   {
       u32 x34_ = 13;
       u32 x38_ = 5;
       u32 x3c_ = 60;
       u32 x40_ = 190;
       u32 x44_ = 0;
   public:
       static const zeus::CColor skStartFadeColor;
       static const zeus::CColor skEndFadeColor;
       static zeus::CColor sCurrentFadeColor;
       CDeathCameraEffect(TUniqueId, TAreaId, std::string_view);

       void Accept(IVisitor&);
       void PreThink(float, CStateManager&);
       void Think(float, CStateManager&);
   };

public:
    DEFINE_PATTERNED(FireFlea)

    CFireFlea(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const CPatternedInfo&, float);
};
}