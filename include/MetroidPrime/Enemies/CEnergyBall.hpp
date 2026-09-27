#ifndef _CENERGYBALL
#define _CENERGYBALL

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

class CEnergyBall : public CPatterned {
public:
  CEnergyBall(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
              const CPatternedInfo& pInfo, int w1, float f1, const CDamageInfo& dInfo1,
              float f2, CAssetId a1, ushort sfxId1, CAssetId a2, CAssetId a3, ushort sfxId2,
              float f3, float f4, CAssetId a4, const CDamageInfo& dInfo2, float f5);
  ~CEnergyBall();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Think(float dt, CStateManager& mgr) override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  int GetBallType() const { return mBallType; }

private:
  void Detonate(CStateManager& mgr);
  static void
  CreateVisorEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                    const rstl::optional_object< TToken< CElectricDescription > >& electric,
                    const rstl::string& name, CStateManager& mgr);

  CSteeringBehaviors mSteeringBehaviors;
  float x56c;
  int mBallType;
  float x574;
  CDamageInfo x578;
  float mInitialTurnSpeed;
  float x598;
  CAssetId x59c;
  ushort x5a0;
  CAssetId x5a4;
  rstl::optional_object< TToken< CElectricDescription > > x5a8;
  ushort x5b4;
  float x5b8;
  float x5bc;
  rstl::optional_object< TToken< CGenDescription > > x5c0;
  CDamageInfo x5cc;
  float x5e8;
};
CHECK_SIZEOF(CEnergyBall, (VERSION >= VERSION_GM8E_02 ? 0x600 : 0x5f0))

#endif // _CENERGYBALL
