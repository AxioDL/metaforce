#ifndef _CSCRIPTCOVERPOINT
#define _CSCRIPTCOVERPOINT

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"

class CScriptCoverPoint : public CActor {
public:
  CScriptCoverPoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, bool active, uint flags, bool crouch, float horizontalAngle,
                    float verticalAngle, float coverTime);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  bool ShouldCrouch() const;
  bool ShouldStay() const;
  bool ShouldWallHang() const;
  bool ShouldLandHere() const;
  void SetInUse(bool inUse);
  bool GetInUse(TUniqueId uid) const;
  const bool Blown(const CVector3f& pos) const;
  float GetSinSqVerticalAngle() const;
  float GetCosHorizontalAngle() const;
  pas::ECoverDirection GetAttackDirection() const;
  void Reserve(TUniqueId id) { xfa_occupant = id; }

private:
  // bool xe8_26_landHere : 1;
  // bool xe8_27_wallHang : 1;
  // bool xe8_28_stay : 1;
  // bool xe8_29_ : 1;
  // bool xe8_30_attackDirection : 1;
  uint xe8_flags;
  float xec_cosHorizontalAngle;
  float xf0_sinVerticalAngle;
  float xf4_coverTime;
  bool xf8_24_crouch : 1;
  bool xf8_25_inUse : 1;
  TUniqueId xfa_occupant;
  TUniqueId xfc_retreating;
  rstl::optional_object< CAABox > x100_touchBounds;
  float x11c_timeLeft;
};

#endif // _CSCRIPTCOVERPOINT
