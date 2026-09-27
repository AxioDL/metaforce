#ifndef _CFIREFLEA
#define _CFIREFLEA

#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

class CFireFlea : public CPatterned {
public:
  class CDeathCameraEffect : public CEntity {
  public:
    ~CDeathCameraEffect() {}
    void PreThink(float, CStateManager& mgr) override;
    void Think(float, CStateManager& mgr) override;
    DECLARE_TYPES_MATCH_OR_ACCEPT;

    CDeathCameraEffect(const TUniqueId uid, const TAreaId aid, const rstl::string& name);

  private:
    static const CColor skEndFadeColor;
    static const CColor skStartFadeColor;
    static CColor sCurrentFadeColor;

    uint mStartFadeTime;
    uint mFadeDuration;
    uint mReverseFadeDuration;
    uint mTotalFadeDuration;
    uint mCurrentTime;
  };

  CFireFlea(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParams,
            const CPatternedInfo& pInfo, float f1);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  bool HearShot(CStateManager& mgr, float arg) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool Delay(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override;
  bool InPosition(CStateManager& mgr, float arg) override;
  void Flee(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;

private:
  CVector3f AdjustMovementVec(CStateManager& mgr, const CVector3f& forward) const;
  bool MoveTooCloseToWater(CStateManager& mgr, const CVector3f& forward) const;
  const bool HeardShot() const { return !(mNearList.size() <= 0); }
  float x568_;
  float x56c_;
  TEntityList mNearList;
  CVector3f xd74_;
  CVector3f mTargetPos;
  CPathFindSearch mPathFind;

  static int sLightIdx;
};

CHECK_SIZEOF(CFireFlea, (VERSION >= VERSION_GM8E_02 ? 0xe80 : 0xe70))

#endif // _CFIREFLEA
