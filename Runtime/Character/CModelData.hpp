#ifndef __URDE_CMODELDATA_HPP__
#define __URDE_CMODELDATA_HPP__

#include "zeus/CVector3f.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CAnimData.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{
class CCharAnimTime;
class CStateManager;
class CActorLights;
struct CModelFlags;
class CRandom16;
class CAnimData;
class CModel;
class CSkinnedModel;
struct SAdvancementDeltas;

class CStaticRes
{
    ResId x0_cmdlId = 0;
    zeus::CVector3f x4_scale;
public:
    CStaticRes(ResId id, const zeus::CVector3f& scale)
        : x0_cmdlId(id),
          x4_scale(scale)
    {}

    ResId GetId() const { return x0_cmdlId; }
    const zeus::CVector3f& GetScale() const { return x4_scale; }
    operator bool() const { return x0_cmdlId != 0; }
};

class CAnimRes
{
    ResId x0_ancsId = -1;
    s32 x4_charIdx = -1;
    zeus::CVector3f x8_scale;
    bool x14_canLoop = false;
    /* NOTE: x18_bodyType - Removed in retail */
    s32 x18_defaultAnim = -1; /* NOTE: used to be x1c in demo */
public:
    CAnimRes() = default;
    CAnimRes(ResId ancs, s32 charIdx, const zeus::CVector3f& scale, const s32 defaultAnim, bool loop)
        : x0_ancsId(ancs),
          x4_charIdx(charIdx),
          x8_scale(scale),
          x14_canLoop(loop),
          x18_defaultAnim(defaultAnim)
    {
    }

    ResId GetId() const { return x0_ancsId; }
    s32 GetCharacterNodeId() const { return x4_charIdx; }
    const zeus::CVector3f& GetScale() const { return x8_scale; }
    bool CanLoop() const { return x14_canLoop; }
    s32 GetDefaultAnim() const { return x18_defaultAnim; }
};

class CModelData
{
    friend class CActor;
    zeus::CVector3f x0_scale;
    bool xc_ = false;
    std::unique_ptr<CAnimData> x10_animData;
    union
    {
        struct
        {
            bool x14_24_renderSorted : 1;
            bool x14_25_sortThermal : 1;
        };
        u32 _flags = 0;
    };
    zeus::CColor x18_ambientColor;

    TLockedToken<CModel> x1c_normalModel;
    TLockedToken<CModel> x2c_xrayModel;
    TLockedToken<CModel> x3c_infraModel;

    std::unique_ptr<CBooModel> m_normalModelInst;
    std::unique_ptr<CBooModel> m_xrayModelInst;
    std::unique_ptr<CBooModel> m_infraModelInst;

    struct WeakModelInsts
    {
        std::weak_ptr<CBooModel> m_normalModelInst;
        std::weak_ptr<CBooModel> m_xrayModelInst;
        std::weak_ptr<CBooModel> m_infraModelInst;
    };
    std::vector<WeakModelInsts> m_weakInsts;

public:
    enum class EWhichModel
    {
        Normal,
        XRay,
        Thermal
    };

    void SetSortThermal(bool v) { x14_25_sortThermal = v; }

    ~CModelData();
    CModelData(const CStaticRes& res);
    CModelData(const CAnimRes& res);
    CModelData(CModelData&&) = default;
    CModelData& operator=(CModelData&&) = default;
    CModelData();
    static CModelData CModelDataNull();

    SAdvancementDeltas GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const;
    void Render(const CStateManager& stateMgr, const zeus::CTransform& xf,
                const CActorLights* lights, const CModelFlags& drawFlags);
    EWhichModel GetRenderingModel(const CStateManager& stateMgr) const;
    CSkinnedModel& PickAnimatedModel(EWhichModel which) const;
    const std::unique_ptr<CBooModel>& PickStaticModel(EWhichModel which) const;
    void SetXRayModel(const std::pair<ResId, ResId>& modelSkin);
    void SetInfraModel(const std::pair<ResId, ResId>& modelSkin);
    bool IsDefinitelyOpaque(EWhichModel);
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
    SAdvancementDeltas AdvanceAnimationIgnoreParticles(float dt, CRandom16&, bool advTree);
    SAdvancementDeltas AdvanceAnimation(float dt, CStateManager& stateMgr, TAreaId aid, bool advTree);
    bool IsAnimating() const;
    bool IsInFrustum(const zeus::CTransform& xf, const zeus::CFrustum& frustum) const;
    void RenderParticles(const zeus::CFrustum& frustum) const;
    void Touch(EWhichModel, int shaderIdx);
    void Touch(const CStateManager& stateMgr, int shaderIdx);
    void RenderThermal(const zeus::CTransform& xf, const zeus::CColor& a, const zeus::CColor& b);
    void RenderUnsortedParts(EWhichModel, const zeus::CTransform& xf,
                             const CActorLights* lights, const CModelFlags& drawFlags);
    void Render(EWhichModel, const zeus::CTransform& xf,
                const CActorLights* lights, const CModelFlags& drawFlags);

    CAnimData* AnimationData() { return x10_animData.get(); }
    bool IsNull() { return !x10_animData && !x1c_normalModel; }
};

}

#endif // __URDE_CMODELDATA_HPP__
