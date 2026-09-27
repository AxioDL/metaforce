#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"

#include "MetroidPrime/CAreaFog.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Math/CloseEnough.hpp"

CScriptDistanceFog::CScriptDistanceFog(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, ERglFogMode mode,
                                       const CColor& color, const CVector2f& range,
                                       float colorDelta, CVector2f rangeDelta, const bool expl,
                                       const bool active, float thermalTarget, float thermalSpeed,
                                       float xrayTarget, float xraySpeed)
: CEntity(uid, info, active, name)
, mMode(mode)
, mColor(color)
, mRange(range)
, mColorDelta(colorDelta)
, mRangeDelta(rangeDelta)
, mThermalTarget(thermalTarget)
, mThermalSpeed(thermalSpeed)
, mXrayTarget(xrayTarget)
, mXraySpeed(xraySpeed)
, mExplicit(expl)
, mNonZero(!close_enough(rangeDelta, CVector2f(0.f, 0.f)) || !close_enough(colorDelta, 0.f)) {}

CScriptDistanceFog::~CScriptDistanceFog() {}

ENTITY_ACCEPT_IMPL(CScriptDistanceFog)

void CScriptDistanceFog::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                         CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  if (GetCurrentAreaId() != kInvalidAreaId && GetActive()) {
    switch (msg) {
    case kSM_InitializedInArea:
      if (mExplicit) {
        const TAreaId aid = GetCurrentAreaId();
        CGameArea::CAreaFog* fog = stateMgr.World()->Area(aid)->AreaFog();
        if (mMode == kRFM_None) {
          fog->DisableFog();
        } else {
          fog->SetFogExplicit(mMode, mColor, mRange);
        }
      }
      break;
    case kSM_Action:
      if (mNonZero) {
        const TAreaId aid = GetCurrentAreaId();
        CGameArea::CAreaFog* fog = stateMgr.World()->Area(aid)->AreaFog();
        if (mMode != kRFM_None) {
          fog->FadeFog(mMode, mColor, mRange, mColorDelta, mRangeDelta);
        } else {
          fog->RollFogOut(mRangeDelta.GetX(), mColorDelta, mColor);
        }
      }

      if (!close_enough(mThermalSpeed, 0.f)) {
        const TAreaId aid = GetCurrentAreaId();
        stateMgr.World()
            ->Area(aid)
            ->SetThermalSpeedAndTarget(mThermalSpeed, mThermalTarget);
      }
      if (!close_enough(mXraySpeed, 0.f)) {
        const TAreaId aid = GetCurrentAreaId();
        stateMgr.World()
            ->Area(aid)
            ->SetXRaySpeedAndTarget(mXraySpeed, mXrayTarget);
      }
      break;
    }
  }
}
