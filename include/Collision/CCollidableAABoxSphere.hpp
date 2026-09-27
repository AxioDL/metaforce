#ifndef _CCOLLIDABLEAABOXSPHERE
#define _CCOLLIDABLEAABOXSPHERE

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionPrimitive.hpp"

class CCollidableAABoxSphere : public CCollisionPrimitive {
public:
  const CCollidableAABox& GetCollidableAABox() const { return mAabox; }
  const CCollidableSphere& GetCollidableSphere() const { return mSphere; }

private:
  CCollidableAABox mAabox;
  CCollidableSphere mSphere;
};
CHECK_SIZEOF(CCollidableAABoxSphere, 0x58)

#endif
