#ifndef _CCOLLIDABLEAABOX
#define _CCOLLIDABLEAABOX

#include "types.h"

#include "Collision/CCollisionPrimitive.hpp"

class CCollidableAABox : public CCollisionPrimitive {
public:
  CCollidableAABox();
  CCollidableAABox(const CAABox& box, const CMaterialList& matList)
  : CCollisionPrimitive(matList), mAabb(box) {}

  static bool CollideMovingAABox(const CInternalCollisionStructure& collision, const CVector3f& dir,
                                 double& dOut, CCollisionInfo& infoOut);
  static bool CollideMovingSphere(const CInternalCollisionStructure& collision,
                                  const CVector3f& dir, double& dOut, CCollisionInfo& infoOut);

  uint GetTableIndex() const;
  CAABox CalculateAABox(const CTransform4f&) const;
  CAABox CalculateLocalAABox() const;
  CAABox Transform(const CTransform4f& xf) const;
  FourCC GetPrimType() const;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const;

  const CAABox& GetBox() const { return mAabb; }
  CAABox& Box() { return mAabb; }

  static void SetStaticTableIndex(uint idx) { sTableIndex = idx; }
  static CCollisionPrimitive::Type GetType();

private:
  static uint sTableIndex;

  CAABox mAabb;
};
CHECK_SIZEOF(CCollidableAABox, 0x28)

namespace Collide {
bool AABox_AABox_Bool(const CInternalCollisionStructure&);
bool AABox_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
bool Sphere_AABox(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
} // namespace Collide
#endif // _CCOLLIDABLEAABOX
