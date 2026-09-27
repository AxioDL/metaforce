#ifndef _CPARTICLEDATABASE
#define _CPARTICLEDATABASE

#include "types.h"

#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class CElectricDescription;
class CGenDescription;
class CParticleGenInfo;
class CStateManager;
class CSwooshDescription;
class CCharLayoutInfo;
class CColor;
class CFrustumPlanes;
class CPoseAsTransforms;
class CTransform4f;
class CVector3f;
class CAuxiliaryParticleData;

class CParticleDatabase {
public:
  typedef rstl::map< rstl::string, rstl::auto_ptr< CParticleGenInfo > > DrawMap;

  CParticleDatabase();
  ~CParticleDatabase();

  void CacheParticleDesc(const CCharacterInfo::CParticleResData& partData);
  void CacheParticleDesc(const SObjectTag& tag);
  void SetParticleEffectState(const rstl::string& name, bool active, CStateManager& mgr);
  void SuspendAllActiveEffects(CStateManager& mgr);
  void AddParticleEffect(const rstl::string& name, int flags, const CParticleData& data,
                         const CVector3f& scale, CStateManager& mgr, TAreaId areaId, bool oneShot,
                         int particleLightIdx);
  void Update(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& layoutInfo,
              const CTransform4f& xf, const CVector3f& scale, CStateManager& mgr);
  void AddToRendererClipped(const CFrustumPlanes&) const;
  void RenderSystemsToBeDrawnFirst() const;
  void RenderSystemsToBeDrawnLast() const;
  void DeleteAllLights(CStateManager& mgr);
  void AddToRendererClippedMasked(const CFrustumPlanes&, int mask, int target) const;
  void RenderSystemsToBeDrawnFirstMasked(int mask, int target) const;
  void RenderSystemsToBeDrawnLastMasked(int mask, int target) const;
  void SetModulationColorAllActiveEffects(const CColor& color);
  CParticleGenInfo* GetParticleEffect(const rstl::string& name);
  void SetExternalVarValue(const rstl::string& name, int index, float value);
  void AddAuxiliaryParticleEffect(const rstl::string& name, int flags,
                                  const CAuxiliaryParticleData& data, const CVector3f& scale,
                                  CStateManager& mgr, TAreaId areaId, int lightIdx);
  void SetUpdatesEnabled(bool enabled) { mUpdatesEnabled = enabled; }

  bool AreAnySystemsDrawnWithModel() const { return mAnySystemsDrawnWithModel; }

private:
  static void SetModulationColorAllActiveEffectsForParticleDB(const CColor& color, DrawMap& map);
  static void SuspendAllActiveEffectsForParticleDB(CStateManager& mgr, DrawMap& map);
  static void DeleteAllLightsForParticleDB(CStateManager& mgr, DrawMap& map);
  static void RenderParticleGenMap(const DrawMap& map);
  static void RenderParticleGenMapMasked(const DrawMap& map, int mask, int target);
  void AddToRendererClippedParticleGenMap(const DrawMap& map, const CFrustumPlanes& frustum) const;
  void AddToRendererClippedParticleGenMapMasked(const DrawMap& map, const CFrustumPlanes& frustum,
                                                int mask, int target) const;
  static void UpdateParticleGenDB(float dt, const CPoseAsTransforms& pose,
                                  const CCharLayoutInfo& layout, const CTransform4f& xf,
                                  const CVector3f& scale, CStateManager& mgr, DrawMap& map,
                                  bool deleteIfDone);
  void InsertParticleGen(const bool oneShot, int flags, const rstl::string& name,
                         const rstl::auto_ptr< CParticleGenInfo >& gen);

  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > > mParticleDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CSwooshDescription > > > mSwooshDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CElectricDescription > > > mElectricDescs;
  DrawMap mRendererDrawLoop;
  DrawMap mFirstDrawLoop;
  DrawMap mLastDrawLoop;
  DrawMap mRendererDraw;
  DrawMap mFirstDraw;
  DrawMap mLastDraw;
  bool mUpdatesEnabled : 1;
  bool mAnySystemsDrawnWithModel : 1;
};
CHECK_SIZEOF(CParticleDatabase, 0xb8)

#endif // _CPARTICLEDATABASE
