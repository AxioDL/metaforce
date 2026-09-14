#ifndef _CENERGYBALL
#define _CENERGYBALL
#include "MetroidPrime/Enemies/CPatterned.hpp"

class CEnergyBall : public CPatterned {
public:
  CEnergyBall(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
              const CPatternedInfo& pInfo, const int w1, const float f1, const CDamageInfo& dInfo1,
              const float f2, const CAssetId a1, const ushort sfxId1, const CAssetId a2,
              const CAssetId a3, const ushort sfxId2, const float f3, const float f4,
              const CAssetId a4, const CDamageInfo& dInfo2, const float f5);
  ~CEnergyBall();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Think(float dt, CStateManager& mgr) override;
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

private:
  void Detonate(CStateManager& mgr);
  static void
  CreateVisorEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                    const rstl::optional_object< TToken< CElectricDescription > >& electric,
                    const rstl::string& name, CStateManager& mgr);

  CSteeringBehaviors mSteeringBehaviors;
  float x56c;
  int x570;
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
CHECK_SIZEOF(CEnergyBall, (VERSION >= VERSION_GM8P_00 ? 0x600 : 0x5f0))
#endif
