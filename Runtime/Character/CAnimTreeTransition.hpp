#ifndef __URDE_CANIMTREETRANSITION_HPP__
#define __URDE_CANIMTREETRANSITION_HPP__

#include "RetroTypes.hpp"
#include "CAnimTreeTweenBase.hpp"

namespace urde
{

class CAnimTreeTransition : public CAnimTreeTweenBase
{
protected:
    CCharAnimTime x24_;
    CCharAnimTime x2c_;
    bool x34_;
    bool x35_;
    bool x36_ = false;
public:
    static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, const std::weak_ptr<CAnimTreeNode>&,
                                           float);

    CAnimTreeTransition(bool, const std::weak_ptr<CAnimTreeNode>&, const std::weak_ptr<CAnimTreeNode>&,
                        const CCharAnimTime&, const CCharAnimTime&, bool, bool, int, const std::string&, bool);
    CAnimTreeTransition(bool, const std::weak_ptr<CAnimTreeNode>&, const std::weak_ptr<CAnimTreeNode>&,
                        const CCharAnimTime&, bool, int, const std::string&);
    CCharAnimTime VGetTimeRemaining() const;
    CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
    std::shared_ptr<IAnimReader> VClone() const;
    void SetBlendingWeight(float w);
    float VGetBlendingWeight() const;
};
}

#endif // __URDE_CANIMTREETRANSITION_HPP__
