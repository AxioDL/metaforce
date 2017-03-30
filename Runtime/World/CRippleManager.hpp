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
    void Ripples();
    void GetRipples() const;
    void Update(float dt);
    float GetLastRippleDeltaTime(TUniqueId rippler) const;
    void AddRipple(const CRipple& ripple);
    void SetMaxTimeFalloff(float time);
    float GetMaxTimeFalloff() const;
};

}

#endif // __URDE_CRIPPLEMANAGER_HPP__
