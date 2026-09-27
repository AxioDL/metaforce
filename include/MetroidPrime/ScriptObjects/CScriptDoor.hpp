#ifndef _CSCRIPTDOOR
#define _CSCRIPTDOOR

#include "MetroidPrime/CPhysicsActor.hpp"

class CScriptDoor : public CPhysicsActor {
public:
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;

  enum EDoorAnimType {
    kDAT_Open,
    kDAT_Close,
    kDAT_Ready,
  };

  enum EDoorOpenCondition {
    kDOC_NotReady,
    kDOC_Loading,
    kDOC_Ready,
  };

  CScriptDoor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& modelData,
              const CActorParameters& actorParameters, const CVector3f& orbitPosition,
              const CAABox& bounds, const bool active, const bool open,
              const bool projectilesCollide, float animationLength, const bool ballDoor);

  void SetDoorAnimation(EDoorAnimType state);

  rstl::optional_object< CAABox > GetProjectileBounds() const;

  EDoorOpenCondition GetDoorOpenCondition(CStateManager& mgr);
  void OpenDoor(TUniqueId uid, CStateManager& mgr);

  const TUniqueId GetConnectedDockID() const { return mDockId; }
  bool IsOpen() const { return mIsOpen; }
  bool IsBallDoor() const { return mBallDoor; }
  void SetDoClose(const bool close) { mDoClose = close; }

  bool IsConnectedToArea(const CStateManager& mgr, TAreaId area) const;
  void ForceClosed(CStateManager& mgr);

private:
  float mAnimLength;
  float mAnimTime;
  EDoorAnimType mDoorState;
  CAABox x264_;
  TUniqueId mPartner1;
  TUniqueId mPartner2;
  TUniqueId mPrevDoor;
  TUniqueId mDockId;
  CAABox mModelBounds;
  CVector3f mOrbitPos;

  bool mClosing : 1;
  bool mWasOpen : 1;
  bool mIsOpen : 1;
  bool mConditionsMet : 1;
  bool mProjectilesCollide : 1;
  bool mBallDoor : 1;
  bool mDoClose : 1;
};
CHECK_SIZEOF(CScriptDoor, (VERSION >= VERSION_GM8E_02 ? 0x2c0 : 0x2b0))

#endif // _CSCRIPTDOOR
