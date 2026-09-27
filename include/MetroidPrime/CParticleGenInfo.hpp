#ifndef _CPARTICLEGENINFO
#define _CPARTICLEGENINFO

#include "Kyoto/Particles/CParticleData.hpp"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

enum EParticleGenType { kPGT_Normal, kPGT_Auxiliary };

class CStateManager;
class CColor;
class CAABox;
class CParticleGen;

class CParticleGenInfo {
public:
  CParticleGenInfo(const SObjectTag& part, int frameCount, const rstl::string& boneName,
                   const CVector3f& scale, CParticleData::EParentedMode parentMode, int flags,
                   EParticleGenType type);
  virtual ~CParticleGenInfo() {}

  virtual void AddToRenderer() = 0;
  virtual void Render() = 0;
  virtual void Update(float dt, CStateManager& stateMgr) = 0;
  virtual void SetOrientation(const CTransform4f& xf, CStateManager& stateMgr) = 0;
  virtual void SetTranslation(const CVector3f& trans, CStateManager& stateMgr) = 0;
  virtual void SetGlobalOrientation(const CTransform4f& xf, CStateManager& stateMgr) = 0;
  virtual void SetGlobalTranslation(const CVector3f& trans, CStateManager& stateMgr) = 0;
  virtual void SetGlobalScale(const CVector3f& scale) = 0;
  virtual void SetParticleEmission(bool isActive, CStateManager& stateMgr) = 0;
  virtual bool IsSystemDeletable() const = 0;
  virtual rstl::optional_object< CAABox >
  GetBounds() const = 0; // This should match CParticleGen::GetBounds result
  virtual bool HasActiveParticles() const = 0;
  virtual void DestroyParticles() = 0;
  virtual bool HasLight() const = 0;
  virtual TUniqueId GetLightId() const = 0;
  virtual void DeleteLight(CStateManager& stateMgr) = 0;
  virtual void SetModulationColor(const CColor& color) = 0;

  EParticleGenType GetType() const { return mType; }
  bool GetIsActive() const { return mActive; }
  void SetIsActive(bool active) { mActive = active; }
  bool GetIsGrabInitialData() const { return mGrabInitialData; }
  void SetIsGrabInitialData(bool grab) { mGrabInitialData = grab; }
  const rstl::string& GetLocatorName() const { return mBoneName; }
  CParticleData::EParentedMode GetParentedMode() const { return mParentMode; }
  uint GetFlags() const { return mFlags; }
  void SetFlags(int flags) { mFlags = flags; }
  const CVector3f& GetScale() const { return mParticleScale; }
  const CTransform4f& GetCurTransform() const { return mTransform; }
  void SetCurTransform(const CTransform4f& xf) { mTransform = xf; }
  const CVector3f& GetCurOffset() const { return mOffset; }
  void SetCurOffset(const CVector3f& offset) { mOffset = offset; }
  float GetInactiveStartTime() const { return mSeconds; }
  float GetCurrentTime() const { return mCurTime; }
  float GetFinishTime() const { return mFinishTime; }
  void ResetTime() { mCurTime = 0.f; }
  void OffsetTime(float dt) { mCurTime += dt; }
  void MarkFinishTime() { mFinishTime = mCurTime; }

private:
  SObjectTag mPart;
  float mSeconds;
  rstl::string mBoneName;
  float mCurTime;
  bool mActive;
  CParticleData::EParentedMode mParentMode;
  s32 mFlags;
  CVector3f mParticleScale;
  float mFinishTime;
  bool mGrabInitialData;
  CTransform4f mTransform;
  CVector3f mOffset;
  EParticleGenType mType;
};
CHECK_SIZEOF(CParticleGenInfo, 0x84)

TUniqueId _initializeLight(const rstl::ncrc_ptr< CParticleGen >&, CStateManager&, TAreaId,
                           int lightId);

#endif // _CPARTICLEGENINFO
