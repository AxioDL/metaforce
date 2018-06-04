#ifndef __URDE_CRIPPLEMANAGER_HPP__
#define __URDE_CRIPPLEMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CRipple.hpp"

namespace urde
{

class CRippleManager
{
    float x0_maxTimeFalloff = 0.f;
    std::vector<CRipple> x4_ripples;
    float x14_alpha;
public:
    CRippleManager(int maxRipples, float alpha);
    void Init(int maxRipples);
    std::vector<CRipple>& Ripples() { return x4_ripples; }
    const std::vector<CRipple>& GetRipples() const { return x4_ripples; }
    void Update(float dt);
    float GetLastRippleDeltaTime(TUniqueId rippler) const;
    void AddRipple(const CRipple& ripple);
    void SetMaxTimeFalloff(float time) { x0_maxTimeFalloff = time; }
    float GetMaxTimeFalloff() const { return x0_maxTimeFalloff; }
    void SetAlpha(float a) { x14_alpha = a; }
    float GetAlpha() const { return x14_alpha; }
};

}
;
#endif // __URDE_CRIPPLEMANAGER_HPP__
