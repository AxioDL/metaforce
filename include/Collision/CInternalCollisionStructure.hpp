#ifndef _CINTERNALCOLLISIONSTRUCTURE
#define _CINTERNALCOLLISIONSTRUCTURE

#include "Kyoto/Math/CTransform4f.hpp"

class CMaterialFilter;
class CCollisionPrimitive;
class CInternalCollisionStructure {
public:
  class CPrimDesc {
  public:
    CPrimDesc(const CCollisionPrimitive& prim, const CMaterialFilter& filter,
              const CTransform4f& xf)
    : mPrim(prim), mFilter(filter), mXf(xf) {}

    const CCollisionPrimitive& GetPrim() const { return mPrim; }
    const CMaterialFilter& GetFilter() const { return mFilter; }
    const CTransform4f& GetTransform() const { return mXf; }

  private:
    const CCollisionPrimitive& mPrim;
    const CMaterialFilter& mFilter;
    CTransform4f mXf;
  };

  CInternalCollisionStructure(const CPrimDesc& left, const CPrimDesc& right)
  : mLeft(left), mRight(right) {}

  const CPrimDesc& GetLeft() const { return mLeft; }
  const CPrimDesc& GetRight() const { return mRight; }
  CInternalCollisionStructure GetSwapped() const {
    return CInternalCollisionStructure(mRight, mLeft);
  }

private:
  CPrimDesc mLeft;
  CPrimDesc mRight;
};

#endif // _CINTERNALCOLLISIONSTRUCTURE
