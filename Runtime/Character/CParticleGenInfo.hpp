#ifndef __URDE_CPARTICLEGENINFO_HPP__
#define __URDE_CPARTICLEGENINFO_HPP__

#include "RetroTypes.hpp"
#include "CParticleData.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
struct SObjectTag;
class CParticleGen;
class CStateManager;

class CParticleGenInfo
{
    SObjectTag x4_part;
    float xc_seconds;
    std::string x10_boneName;
    float x20_ = 0.f;
    bool x24_ = false;
    CParticleData::EParentedMode x28_parentMode;
    int x2c_a;
    zeus::CVector3f x30_particleScale;
    float x3c_ = 0.f;
    bool x40_ = false;
    zeus::CTransform x44_;
    zeus::CVector3f x74_;
public:
    CParticleGenInfo(const SObjectTag& part, int frameCount, const std::string& boneName,
                     const zeus::CVector3f&, CParticleData::EParentedMode parentMode, int a);

    virtual ~CParticleGenInfo() = default;
    virtual void AddToRenderer()=0;
    virtual void Render()=0;
    virtual void Update(float dt, CStateManager& stateMgr)=0;
    virtual void SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr)=0;
    virtual void SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr)=0;
    virtual void SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr)=0;
    virtual void SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr)=0;
    virtual void SetGlobalScale(const zeus::CVector3f& scale)=0;
    virtual void SetParticleEmission(bool, CStateManager& stateMgr)=0;
    virtual bool IsSystemDeletable() const=0;
    virtual zeus::CAABox GetBounds() const=0;
    virtual bool HasActiveParticles() const=0;
    virtual void DestroyParticles()=0;
    virtual bool HasLight() const=0;
    virtual TUniqueId GetLightId() const=0;
    virtual void SetModulationColor(const zeus::CColor& color)=0;
};

class CParticleGenInfoGeneric : public CParticleGenInfo
{
    std::shared_ptr<CParticleGen> x80_system;
    TUniqueId x84_lightId;
public:
    CParticleGenInfoGeneric(const SObjectTag& part, const std::weak_ptr<CParticleGen>& system,
                            int, const std::string& boneName, const zeus::CVector3f& scale,
                            CParticleData::EParentedMode parentMode, int a, CStateManager& stateMgr,
                            int lightId);

    void AddToRenderer();
    void Render();
    void Update(float dt, CStateManager& stateMgr);
    void SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr);
    void SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr);
    void SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr);
    void SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr);
    void SetGlobalScale(const zeus::CVector3f& scale);
    void SetParticleEmission(bool, CStateManager& stateMgr);
    bool IsSystemDeletable() const;
    zeus::CAABox GetBounds() const;
    bool HasActiveParticles() const;
    void DestroyParticles();
    bool HasLight() const;
    TUniqueId GetLightId() const;
    void SetModulationColor(const zeus::CColor& color);
};

}

#endif // __URDE_CPARTICLEGENINFO_HPP__
