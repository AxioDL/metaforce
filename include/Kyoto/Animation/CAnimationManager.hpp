#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimationDatabase.hpp"
#include "Kyoto/TToken.hpp"

class CMetaAnimTreeBuildOrders;
class CAnimTreeContext;
class IMetaAnim;
class CAnimationManager {
public:
  CAnimationManager(const TToken< CAnimationDatabase >& animDB, const CAnimSysContext& sysCtx);

  rstl::ncrc_ptr< CAnimTreeNode >
  GetAnimationTree(uint animIdx, const CMetaAnimTreeBuildOrders& orders) const;
  rstl::rc_ptr< IMetaAnim > GetMetaAnimation(uint animIdx) const;

private:
  void ForceOrdering();
  TToken< CAnimationDatabase > mAnimDB;
  CAnimSysContext mSysCtx;
};

CHECK_SIZEOF(CAnimationManager, 0x18)

#endif // _CANIMATIONMANAGER
