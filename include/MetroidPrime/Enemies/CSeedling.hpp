#ifndef _CSEEDLING
#define _CSEEDLING

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CWallWalker.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Math/CAABox.hpp"

class CModelData;

class CSeedling : public CWallWalker {
public:
  CSeedling(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, const CAssetId& needleModel, const CAssetId& weaponId,
            const CDamageInfo& projectileDamage, const CDamageInfo& deathDamage, float f1, float f2,
            float f3, float f4);

  // CEntity
  ~CSeedling() override;
  DECLARE_TYPES_MATCH;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;

  // CPatterned
  void MassiveDeath(CStateManager& mgr) override;
  CPathFindSearch* GetSearchPath() override { return &mSearchPath; }
  CProjectileInfo* ProjectileInfo() override;

private:
  void FireSpikes(CStateManager& mgr);
  
  CPathFindSearch mSearchPath;
  rstl::single_ptr< CModelData > mSpikeData;
  CProjectileInfo mProjectileInfo;
  CDamageInfo mDeathDamage;
  CAABox mModelBounds;
  float mAttackCoolOff;
  TUniqueId mPrevObj;
  bool mRenderOnlyClusterA : 1;
  bool mCurNeedleCluster : 1;

  static const char* const skSpikeLocators[12];
};
CHECK_SIZEOF(CSeedling, (VERSION >= VERSION_GM8E_02 ? 0x738 : 0x728))

#endif // _CSEEDLING
