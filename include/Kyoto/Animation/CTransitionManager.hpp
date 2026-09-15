#ifndef _CTRANSITIONMANAGER
#define _CTRANSITIONMANAGER

#include "Kyoto/Animation/CAnimSysContext.hpp"

class CAnimTreeNode;
class CTransitionManager {
public:
  CTransitionManager(const CAnimSysContext& context);

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                  const rstl::ncrc_ptr< CAnimTreeNode >& b) const;

private:
  CAnimSysContext x0_context;
};

CHECK_SIZEOF(CTransitionManager, 0x10)

#endif // _CTRANSITIONMANAGER
