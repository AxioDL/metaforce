#ifndef __URDE_CMODELDATA_HPP__
#define __URDE_CMODELDATA_HPP__

#include "zeus/CVector3f.hpp"
#include "zeus/CAABox.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"

namespace urde
{
class CCharAnimTime;
class CStateManager;
class CActorLights;
class CModelFlags;
class CRandom16;
class CFrustumPlanes;
class CAnimData;
class CModel;

struct CStaticRes
{
};

struct CAnimRes
{
    zeus::CVector3f x8_scale;
};

class CModelData
{
    zeus::CVector3f x0_particleScale;
    std::unique_ptr<CAnimData> xc_animData;
    TLockedToken<CModel> x1c_normalModel;
    TLockedToken<CModel> x2c_xrayModel;
    TLockedToken<CModel> x3c_infraModel;
public:
    enum class EWhichModel
    {
    };

    CModelData();
    CModelData(const CStaticRes& res);
    CModelData(const CAnimRes& res);
    CModelData CModelDataNull() {return CModelData();}

    zeus::CVector3f GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const;
    void Render(const CStateManager& stateMgr, const zeus::CTransform& xf,
                const CActorLights* lights, const CModelFlags& drawFlags) const;
    void GetRenderingModel(const CStateManager& stateMgr);
    void PickAnimatedModel(EWhichModel) const;
    void PickStaticModel(EWhichModel) const;
    void SetXRayModel(const std::pair<TResId, TResId>& modelSkin);
    void SetInfraModel(const std::pair<TResId, TResId>& modelSkin);
    bool IsDefinitelyOpaque(EWhichModel) const;
    bool GetIsLoop() const;
    float GetAnimationDuration(int) const;
    void EnableLooping(bool);
    void AdvanceParticles(const zeus::CTransform& xf, float, CStateManager& stateMgr);
    zeus::CAABox GetBounds() const;
    zeus::CAABox GetBounds(const zeus::CTransform& xf) const;
    zeus::CTransform GetScaledLocatorTransformDynamic(const std::string& name, const CCharAnimTime* time) const;
    zeus::CTransform GetScaledLocatorTransform(const std::string& name) const;
    zeus::CTransform GetLocatorTransformDynamic(const std::string& name, const CCharAnimTime* time) const;
    zeus::CTransform GetLocatorTransform(const std::string& name) const;
    void AdvanceAnimationIgnoreParticles(float dt, CRandom16&, bool);
    void AdvanceAnimation(float dt, CStateManager& stateMgr, bool);
    bool IsAnimating() const;
    bool IsInFrustum(const zeus::CTransform& xf, const CFrustumPlanes& frustum) const;
    void RenderParticles(const CFrustumPlanes& frustum) const;
    void Touch(EWhichModel, int) const;
    void Touch(const CStateManager& stateMgr, int) const;
    void RenderThermal(const zeus::CTransform& xf, const zeus::CColor& a, const zeus::CColor& b) const;
    void RenderUnsortedParts(EWhichModel, const zeus::CTransform& xf,
                             const CActorLights* lights, const CModelFlags& drawFlags) const;
    void Render(EWhichModel, const zeus::CTransform& xf,
                const CActorLights* lights, const CModelFlags& drawFlags) const;
};

}

#endif // __URDE_CMODELDATA_HPP__
