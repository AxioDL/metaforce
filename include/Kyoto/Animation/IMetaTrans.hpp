#ifndef _IMETATRANS
#define _IMETATRANS

#include "Kyoto/Animation/CAnimTreeNode.hpp"

#include "rstl/rc_ptr.hpp"

class CAnimSysContext;
class COutputStream;

enum EMetaTransType {
  kMTT_MetaAnim,
  kMTT_Trans,
  kMTT_PhaseTrans,
  kMTT_Snap,
};

class IMetaTrans {
public:
  virtual ~IMetaTrans() = 0;
  virtual rstl::ncrc_ptr< CAnimTreeNode >
  VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                     const CAnimSysContext& animSys) const = 0;
  virtual EMetaTransType GetType() const = 0;

  virtual void WriteTransData(COutputStream&) const = 0;

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                    const CAnimSysContext& animSys) const {
    return VGetTransitionTree(a, b, animSys);
  }
};

inline IMetaTrans::~IMetaTrans() {}

#endif // _IMETATRANS
