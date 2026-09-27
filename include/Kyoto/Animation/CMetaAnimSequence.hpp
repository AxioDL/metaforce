#ifndef _CMETAANIMSEQUENCE
#define _CMETAANIMSEQUENCE

#include "Kyoto/Animation/IMetaAnim.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CMetaAnimSequence : public IMetaAnim {
public:
  ~CMetaAnimSequence() override {}
  EMetaAnimType GetType() const override { return kMAT_Sequence; }
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
  rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const override;
  void WriteAnimData(COutputStream& out) const override;

  explicit CMetaAnimSequence(CInputStream& in);

private:
  static rstl::vector< rstl::rc_ptr< IMetaAnim > > CreateSequence(CInputStream& in);
  rstl::vector< rstl::rc_ptr< IMetaAnim > > mSequence;
};
CHECK_SIZEOF(CMetaAnimSequence, 0x14)

#endif // _CMETAANIMSEQUENCE
