#include "Kyoto/Animation/CMetaTransTrans.hpp"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaTransTrans::CMetaTransTrans(CInputStream& in)
: mTransDur(CCharAnimTime(in))
, xc_(in.ReadBool())
, mRunA(in.ReadBool())
, mFlags(in.ReadLong()) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransTrans::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                    const CAnimSysContext& animSys) const {
  return rs_new CAnimTreeTransition(
      xc_, a, b, mTransDur, mRunA, mFlags,
      CAnimTreeTransition::CreatePrimitiveName(a, b, mTransDur.GetSeconds()));
}

void CMetaTransTrans::WriteTransData(COutputStream& out) const {
  mTransDur.PutTo(out);
  out.WriteChar(bool(xc_));
  out.WriteChar(bool(mRunA));
  out.WriteLong(mFlags);
}
