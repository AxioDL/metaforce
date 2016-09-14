#ifndef __URDE_CANIMTREETIMESCALE_HPP__
#define __URDE_CANIMTREETIMESCALE_HPP__

#include "CAnimTreeSingleChild.hpp"
#include "CConstantAnimationTimeScale.hpp"

namespace urde
{

class CAnimTreeTimeScale : public CAnimTreeSingleChild
{
    std::unique_ptr<CConstantAnimationTimeScale> x18_timeScale;
    CCharAnimTime x20_;
    CCharAnimTime x28_;
public:
    CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>&, float, const std::string&);

    static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, float, const CCharAnimTime&, float);

    CCharAnimTime GetRealLifeTime(const CCharAnimTime&) const;
    void VSetPhase(float);
    std::shared_ptr<IAnimReader> VSimplified();
};

}

#endif // __URDE_CANIMTREETIMESCALE_HPP__
