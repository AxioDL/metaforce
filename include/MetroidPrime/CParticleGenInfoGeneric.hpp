#ifndef _CPARTICLEGENINFOGENERIC
#define _CPARTICLEGENINFOGENERIC

#include "MetroidPrime/CParticleGenInfo.hpp"

class CParticleGen;

class CParticleGenInfoGeneric : public CParticleGenInfo {
public:
  CParticleGenInfoGeneric(const SObjectTag& part, rstl::ncrc_ptr< CParticleGen > system, int frames,
                          const rstl::string& boneName, CVector3f scale,
                          CParticleData::EParentedMode parentMode, int flags,
                          CStateManager& stateMgr, TAreaId areaId, int lightId,
                          EParticleGenType state);
  ~CParticleGenInfoGeneric();

  virtual void AddToRenderer() override;
  virtual void Render() override;
  virtual void Update(float dt, CStateManager& stateMgr) override;
  virtual void SetOrientation(const CTransform4f& xf, CStateManager& stateMgr) override;
  virtual void SetTranslation(const CVector3f& trans, CStateManager& stateMgr) override;
  virtual void SetGlobalOrientation(const CTransform4f& xf, CStateManager& stateMgr) override;
  virtual void SetGlobalTranslation(const CVector3f& trans, CStateManager& stateMgr) override;
  virtual void SetGlobalScale(const CVector3f& scale) override;
  virtual void SetParticleEmission(bool isActive, CStateManager& stateMgr) override;
  virtual bool IsSystemDeletable() const override;
  virtual rstl::optional_object<CAABox> GetBounds() const override;
  virtual bool HasActiveParticles() const override;
  virtual void DestroyParticles() override;
  virtual bool HasLight() const override;
  virtual TUniqueId GetLightId() const override;
  virtual void DeleteLight(CStateManager& stateMgr) override;
  virtual void SetModulationColor(const CColor& color) override;

  rstl::ncrc_ptr< CParticleGen > GetParticleSystem() const { return mSystem; }

private:
  rstl::ncrc_ptr< CParticleGen > mSystem;
  TUniqueId mLightId;
};

#endif // _CPARTICLEGENINFOGENERIC
