#ifndef _CFLICKERBAT
#define _CFLICKERBAT

#include "MetroidPrime/Enemies/CPatterned.hpp"

class CFlickerBat : public CPatterned {
public:
  enum EFlickerBatState {
    kFBS_Visible,
    kFBS_Hidden,
    kFBS_FadeIn,
    kFBS_FadeOut,
  };

  CFlickerBat(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
              const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
              const CPatternedInfo& pInfo, EColliderType colType, const bool startsVisible,
              const CActorParameters& actParms, const bool enableLineOfSight);
  ~CFlickerBat() override;

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor&, CStateManager&) override;
  
  bool CanBeShot(const CStateManager&, int) override;
  bool HearShot(CStateManager&, float) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Render(const CStateManager&) const override;
  bool InPosition(CStateManager&, float) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;

private:
  void ToggleVisible(CStateManager& mgr);
  void SetFlickerBatState(EFlickerBatState state, CStateManager& mgr);
  void FlickerBatStateChanged(EFlickerBatState state, CStateManager& mgr);
  void SetHeardShot(bool heardShot);
  void CheckFadeEffect(CStateManager& mgr);
  void NotifyNeighbors(CStateManager& mgr);
  bool IsVisible() const { return mState == kFBS_Visible; }
  EFlickerBatState GetFlickerBatState() const;
  float x568_;
  float x56c_;
  float x570_;
  EFlickerBatState mState;
  float mFadeRemTime;
  float mFadeDuration;
  bool mWasInXray : 1;
  bool mHeardShot : 1;
  bool mInLOS : 1;
  bool mEnableLOSCheck : 1;
};

#endif // _CFLICKERBAT
