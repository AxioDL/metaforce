#ifndef _CMETATRANSMETAANIM
#define _CMETATRANSMETAANIM

#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/rc_ptr.hpp"
class IMetaAnim;
class CInputStream;
class COutputStream;

class CMetaTransMetaAnim : public IMetaTrans {
public:
  explicit CMetaTransMetaAnim(CInputStream& in);
  EMetaTransType GetType() const override { return kMTT_MetaAnim; }

  rstl::ncrc_ptr< CAnimTreeNode > VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                     const CAnimSysContext& animSys) const override;

  void WriteTransData(COutputStream&) const override;

private:
  rstl::rc_ptr< IMetaAnim > mMetaAnim;
};
CHECK_SIZEOF(CMetaTransMetaAnim, 0x8)

#endif // _CMETATRANSMETAANIM
