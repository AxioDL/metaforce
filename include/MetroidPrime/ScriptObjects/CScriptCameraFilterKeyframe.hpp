#ifndef _CSCRIPTCAMERAFILTERKEYFRAME
#define _CSCRIPTCAMERAFILTERKEYFRAME

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Graphics/CColor.hpp"

class CScriptCameraFilterKeyframe : public CEntity {
  CCameraFilterPass::EFilterType mType;
  CCameraFilterPass::EFilterShape mShape;
  CStateManager::ECameraFilterStage mFilterIdx;
  uint x40_;
  CColor mColor;
  float mTimeIn;
  float mTimeOut;
  CAssetId mTxtr;

public:
  CScriptCameraFilterKeyframe(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                              CCameraFilterPass::EFilterType type,
                              CCameraFilterPass::EFilterShape shape, CStateManager::ECameraFilterStage filterIdx, uint unk,
                              float colorR, float colorG, float colorB, float colorA, float timeIn,
                              float timeOut, CAssetId txtr, bool active);
  ~CScriptCameraFilterKeyframe();

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  DECLARE_ACCEPT;
};

#endif // _CSCRIPTCAMERAFILTERKEYFRAME
