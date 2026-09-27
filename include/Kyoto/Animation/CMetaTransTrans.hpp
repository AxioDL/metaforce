#ifndef _CMETATRANSTRANS
#define _CMETATRANSTRANS

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/rc_ptr.hpp"
class IMetaAnim;
class CInputStream;
class COutputStream;

class CMetaTransTrans : public IMetaTrans {
public:
  explicit CMetaTransTrans(CInputStream& in);
  EMetaTransType GetType() const override { return kMTT_Trans; }

  rstl::ncrc_ptr< CAnimTreeNode > VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                     const CAnimSysContext& animSys) const override;

  void WriteTransData(COutputStream&) const override;

private:
  CCharAnimTime mTransDur;
  bool xc_;
  bool mRunA;
  u32 mFlags;
};
CHECK_SIZEOF(CMetaTransTrans, 0x14)

#endif // _CMETATRANSTRANS
