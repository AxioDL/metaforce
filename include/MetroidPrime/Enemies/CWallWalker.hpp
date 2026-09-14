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
              const float playerObstructionMinDist);

  // CEntity
  ~CWallWalker() override {}
  DECLARE_TYPES_MATCH;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x590_colSphere; }

  static bool PointOnSurface(const CCollisionSurface& surface, const CVector3f& point);
  static CVector3f ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                      const CVector3f& normal);
  static CVector3f ProjectVectorToPlane(const CVector3f& vec, const CVector3f& planeDir);

protected:
  void OrientToSurfaceNormal(const CVector3f& normal, float clampAngle);
  void AlignToFloor(CStateManager& mgr, float radius, const CVector3f& newPos, float dt);
  void GotoNextWaypoint(CStateManager& mgr);

  CCollisionSurface x568_alignNormal;
  CCollidableSphere x590_colSphere;
  float x5b0_collisionCloseMargin;
  float x5b4_alignAngVel;
  float x5b8_tumbleAngle;
  float x5bc_patrolPauseRemTime;
  float x5c0_advanceWpRadius;
  float x5c4_playerObstructionMinDist;
  float x5c8_bendingHackWeight;
  int x5cc_bendingHackAnim;
  EType x5d0_walkerType;
  short x5d4_thinkCounter;
  bool x5d6_24_alignToFloor : 1;
  bool x5d6_25_hasAlignSurface : 1;
  bool x5d6_26_playerObstructed : 1;
  bool x5d6_27_disableMove : 1;
  bool x5d6_28_addBendingWeight : 1;
  bool x5d6_29_applyBendingHack : 1;
};
CHECK_SIZEOF(CWallWalker, (VERSION >= VERSION_GM8P_00 ? 0x5e8 : 0x5D8))

#endif // _CWALLWALKER
