#ifndef _CCOLLIDABLESPHERE
#define _CCOLLIDABLESPHERE

#include "types.h"

#include "Collision/CCollisionPrimitive.hpp"

#include "Kyoto/Math/CSphere.hpp"

class CCollidableSphere : public CCollisionPrimitive {
public:
  CCollidableSphere(const CSphere& sphere, const CMaterialList& material)
  : CCollisionPrimitive(material), mSphere(sphere) {}
  CCollidableSphere(const CCollidableSphere& other)
  : CCollisionPrimitive(other), mSphere(other.mSphere) {}

  static bool CollideMovingAABox(const CInternalCollisionStructure& collision, const CVector3f& dir,
                                 double& dOut, CCollisionInfo& infoOut);
  static bool CollideMovingSphere(const CInternalCollisionStructure& collision,
                                  const CVector3f& dir, double& dOut, CCollisionInfo& infoOut);

  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f&) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  CSphere Transform(const CTransform4f& xf) const;
  const CSphere& GetSphere() const { return mSphere; }
  void SetSphere(const CSphere& sphere) { mSphere = sphere; }
  void SetSphereCenter(const CVector3f& center) { mSphere = CSphere(center, mSphere.GetRadius()); }
  ~CCollidableSphere() override {}
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& internalRayCast) const override;


  static void SetStaticTableIndex(uint idx);
  static Type GetType();
  static bool Sphere_AABox_Bool(const CSphere& sphere, const CAABox& aabox);

private:
  static uint sTableIndex;
  CSphere mSphere;
};
CHECK_SIZEOF(CCollidableSphere, 0x20)

namespace Collide {
bool Sphere_Sphere(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
bool Sphere_AABox_Bool(const CInternalCollisionStructure& collision);
bool Sphere_Sphere_Bool(const CInternalCollisionStructure& collision);
} // namespace Collide

#endif // _CCOLLIDABLESPHERE
