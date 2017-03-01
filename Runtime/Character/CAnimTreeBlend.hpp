#ifndef __URDE_CANIMTREEBLEND_HPP__
#define __URDE_CANIMTREEBLEND_HPP__

#include "CAnimTreeTweenBase.hpp"

namespace urde
{

class CAnimTreeBlend : public CAnimTreeTweenBase
{
    float x24_blendWeight;
public:
    static std::string CreatePrimitiveName(const std::shared_ptr<CAnimTreeNode>& a,
                                           const std::shared_ptr<CAnimTreeNode>& b,
                                           float scale);

    CAnimTreeBlend(bool,
                   const std::shared_ptr<CAnimTreeNode>& a,
                   const std::shared_ptr<CAnimTreeNode>& b,
                   float blendWeight, const std::string& name);

    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    CCharAnimTime VGetTimeRemaining() const;
    CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
    std::unique_ptr<IAnimReader> VClone() const;
    void SetBlendingWeight(float w);
    float VGetBlendingWeight() const;
};

}

#endif // __URDE_CANIMTREEBLEND_HPP__
