#ifndef _CSEQUENCEHELPER
#define _CSEQUENCEHELPER
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CSequenceFundamentals.hpp"

class IMetaAnim;
class CAnimTreeNode;
class CSequenceHelper {
public:
  CSequenceHelper(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                  const rstl::ncrc_ptr< CAnimTreeNode >& b, const CAnimSysContext& context);
  CSequenceHelper(const rstl::vector< rstl::rc_ptr< IMetaAnim > >& anims,
                  const CAnimSysContext& context);

  CSequenceFundamentals ComputeSequenceFundamentals() const;

private:
  CAnimSysContext mContext;
  rstl::vector< rstl::ncrc_ptr< CAnimTreeNode > > mNodes;
  rstl::vector< bool > x20_;
};
CHECK_SIZEOF(CSequenceHelper, 0x30)
#endif // _CSEQUENCEHELPER