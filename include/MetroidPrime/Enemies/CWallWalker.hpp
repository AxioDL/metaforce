#ifndef _CWALLWALKER
#define _CWALLWALKER

#include "types.h"

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

class CWallWalker : public CPatterned {
public:
  enum EType {
    kWT_Parasite = 0,
    kWT_Oculus = 1,
    kWT_Geemer = 2,
    kWT_IceZoomer = 3,
    kWT_Seedling = 4,
  };

  CWallWalker(const EPatternedAI chr, const TUniqueId uid, const rstl::string& name,
              const EFlavorType flavorType, const CEntityInfo& info, const CTransform4f& xf,
              const CModelData& mData, const CPatternedInfo& pInfo, const EMovementType moveType,
              const EColliderType colType, const EBodyType bodyType,
              const CActorParameters& actParms, const ECreatureSize kbVariant,
              const float collisionCloseMargin, const EType walkerType, const bool disableMove,
              const float alignAngVel, const float advanceWpRadius,
              const float playerObstructionMinDist
#if VERSION >= VERSION_GM8P_00
              , const float touchBoundsScale
#endif
  );

  // CEntity
  ~CWallWalker() override {}
  void PreThink(float dt, CStateManager& mgr) override;
  DECLARE_TYPES_MATCH;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mColSphere; }
#if VERSION >= VERSION_GM8P_00
  rstl::optional_object< CAABox > GetTouchBounds() const override;
#endif

  static bool PointOnSurface(const CCollisionSurface& surface, const CVector3f& point);
  static CVector3f ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                      const CVector3f& normal);
  static CVector3f ProjectVectorToPlane(const CVector3f& vec, const CVector3f& planeDir);

protected:
  void OrientToSurfaceNormal(const CVector3f& normal, float clampAngle);
  void AlignToFloor(CStateManager& mgr, float radius, const CVector3f& newPos, float dt);
  void GotoNextWaypoint(CStateManager& mgr);

  CCollisionSurface mAlignNormal;
  CCollidableSphere mColSphere;
  float mCollisionCloseMargin;
  float mAlignAngVel;
  float mTumbleAngle;
  float mPatrolPauseRemTime;
  float mAdvanceWpRadius;
  float mPlayerObstructionMinDist;
  float mBendingHackWeight;
  int mBendingHackAnim;
  EType mWalkerType;
  short mThinkCounter;
  bool mAlignToFloor : 1;
  bool mHasAlignSurface : 1;
  bool mPlayerObstructed : 1;
  bool mDisableMove : 1;
  bool mAddBendingWeight : 1;
  bool mApplyBendingHack : 1;
#if VERSION >= VERSION_GM8P_00
  float mTouchBoundsScale;
#endif
};
CHECK_CHILD_SIZEOF(CWallWalker, CPatterned, VERSION >= VERSION_GM8P_00 ? 0x78 : 0x70)

#endif // _CWALLWALKER
