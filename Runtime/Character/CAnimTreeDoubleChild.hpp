#ifndef __URDE_CANIMTREEDOUBLECHILD_HPP__
#define __URDE_CANIMTREEDOUBLECHILD_HPP__

#include "CAnimTreeNode.hpp"

namespace urde
{

class CAnimTreeDoubleChild : public CAnimTreeNode
{
public:
    class CDoubleChildAdvancementResult
    {
        CCharAnimTime x0_;
        SAdvancementDeltas x8_;
        SAdvancementDeltas x24_;
    public:
        CDoubleChildAdvancementResult(const CCharAnimTime&, const SAdvancementDeltas&, const SAdvancementDeltas);
        void GetLeftAdvancementDeltas() const;
        void GetRightAdvancementDeltas() const;
        void GetTrueAdvancement() const;
    };
protected:
    std::shared_ptr<CAnimTreeNode> x14_a;
    std::shared_ptr<CAnimTreeNode> x18_b;

public:
    CAnimTreeDoubleChild(const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                         std::string_view name);
    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator,
                            u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    bool VGetBoolPOIState(const char* name) const;
    s32 VGetInt32POIState(const char* name) const;
    CParticleData::EParentedMode VGetParticlePOIState(const char* name) const;
    void VSetPhase(float);
    SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
    u32 Depth() const;
    CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const;
    u32 VGetNumChildren() const;
    std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
    void VGetWeightedReaders(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const;

    //virtual float VGetTotalChildWeight(float) const = 0;
    //float GetTotalChildWeight(float f) const { return VGetTotalChildWeight(f); }
    virtual float VGetRightChildWeight() const = 0;
    float GetRightChildWeight() const { return VGetRightChildWeight(); }

    const std::shared_ptr<CAnimTreeNode>& GetLeftChild() const { return x14_a; }
    const std::shared_ptr<CAnimTreeNode>& GetRightChild() const { return x18_b; }
};
}

#endif // __URDE_CANIMTREEDOUBLECHILD_HPP__
