#ifndef _CPARTICLEGEN
#define _CPARTICLEGEN

#include "Kyoto/SObjectTag.hpp"

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CAABox;
class CColor;
class CLight;
class CTransform4f;
class CVector3f;
class CWarp;

class CParticleGen {
public:
  virtual ~CParticleGen() = 0;
  virtual const bool Update(double) = 0;
  virtual void Render() = 0;
  virtual void SetOrientation(const CTransform4f& orientation) = 0;
  virtual void SetTranslation(const CVector3f& translation) = 0;
  virtual void SetGlobalOrientation(const CTransform4f& orientation) = 0;
  virtual void SetGlobalTranslation(const CVector3f& translation) = 0;
  virtual void SetGlobalScale(const CVector3f& scale) = 0;
  virtual void SetLocalScale(const CVector3f& scale) = 0;
  virtual void SetParticleEmission(bool emission) = 0;
  virtual void SetModulationColor(const CColor& col) = 0;
  virtual void SetGeneratorRate(float rate) {}
  virtual const CTransform4f& GetOrientation() const = 0;
  virtual const CVector3f& GetTranslation() const = 0;
  virtual const CTransform4f& GetGlobalOrientation() const = 0;
  virtual const CVector3f& GetGlobalTranslation() const = 0;
  virtual const CVector3f& GetGlobalScale() const = 0;
  virtual bool GetParticleEmission() const = 0;
  virtual const CColor& GetModulationColor() const = 0;
  virtual float GetGeneratorRate() const { return 1.f; }
  virtual bool IsSystemDeletable() const = 0;
  virtual rstl::optional_object< CAABox > GetBounds() const = 0;
  virtual int GetParticleCount() const = 0;
  virtual bool SystemHasLight() const = 0;
  virtual CLight GetLight() const = 0;
  virtual void DestroyParticles() = 0;
  virtual void AddModifier(CWarp*);
  virtual uint Get4CharId() const = 0;

  static FourCC ResType() { return 'PART'; }

protected:
  rstl::list< CWarp* > mModifiersList;
};

inline CParticleGen::~CParticleGen() {}

#endif // _CPARTICLEGEN
