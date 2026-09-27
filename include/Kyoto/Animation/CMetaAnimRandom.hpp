#ifndef _CMETAANIMRANDOM
#define _CMETAANIMRANDOM

#include "Kyoto/Animation/IMetaAnim.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CMetaAnimRandom : public IMetaAnim {
public:
  ~CMetaAnimRandom() override {}
  EMetaAnimType GetType() const override { return kMAT_Random; }
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
  rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const override;
  void WriteAnimData(COutputStream& out) const override;

  explicit CMetaAnimRandom(CInputStream& in);

private:
  typedef rstl::vector< rstl::pair< rstl::rc_ptr< IMetaAnim >, int > > RandomData;
  static RandomData CreateRandomData(CInputStream& in);
  RandomData mRandomData;
};
CHECK_SIZEOF(CMetaAnimRandom, 0x14)

#endif // _CMETAANIMRANDOM
