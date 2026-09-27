#ifndef _CTRANSITIONMANAGER
#define _CTRANSITIONMANAGER

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"

class CTransitionManager {
public:
  CTransitionManager(const CAnimSysContext& context);

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b) const;

private:
  CAnimSysContext mContext;
};

CHECK_SIZEOF(CTransitionManager, 0x10)

#endif // _CTRANSITIONMANAGER
