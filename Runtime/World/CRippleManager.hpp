#ifndef __URDE_CRIPPLEMANAGER_HPP__
#define __URDE_CRIPPLEMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CRipple.hpp"

namespace urde
{

class CRippleManager
{
    float x0_ = 0.f;
    std::vector<CRipple> x4_ripples;
    float x14_;
public:
    CRippleManager(int maxRipples, float);
    void Init(int maxRipples);
    void SetTime(float);
    std::vector<CRipple>& Ripples() { return x4_ripples; }
    const std::vector<CRipple>& GetRipples() const { return x4_ripples; }
    void Update(float dt);
    float GetLastRippleDeltaTime(TUniqueId rippler) const;
    void AddRipple(const CRipple& ripple);
    void SetMaxTimeFalloff(float time);
    float GetMaxTimeFalloff() const;
};

}
;
#endif // __URDE_CRIPPLEMANAGER_HPP__
