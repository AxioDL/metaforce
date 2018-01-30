#ifndef __URDE_CANIMTREETIMESCALE_HPP__
#define __URDE_CANIMTREETIMESCALE_HPP__

#include "CAnimTreeSingleChild.hpp"
#include "CTimeScaleFunctions.hpp"

namespace urde
{

class CAnimTreeTimeScale : public CAnimTreeSingleChild
{
    std::unique_ptr<IVaryingAnimationTimeScale> x18_timeScale;
    CCharAnimTime x20_curAccelTime;
    CCharAnimTime x28_targetAccelTime;
    CCharAnimTime x30_initialTime;
public:
    CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node, float timeScale, std::string_view name);
    CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node,
                       std::unique_ptr<IVaryingAnimationTimeScale>&& timeScale,
                       const CCharAnimTime& time, std::string_view name);

    static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, float, const CCharAnimTime&, float);

    CCharAnimTime GetRealLifeTime(const CCharAnimTime&) const;
    void VSetPhase(float);
    std::experimental::optional<std::unique_ptr<IAnimReader>> VSimplified();

    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    bool VGetBoolPOIState(const char* name) const;
    s32 VGetInt32POIState(const char* name) const;
    CParticleData::EParentedMode VGetParticlePOIState(const char* name) const;

    CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const;
    std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
    std::unique_ptr<IAnimReader> VClone() const;
    CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
    CCharAnimTime VGetTimeRemaining() const;
    SAdvancementResults VAdvanceView(const CCharAnimTime& dt);
};

}

#endif // __URDE_CANIMTREETIMESCALE_HPP__
