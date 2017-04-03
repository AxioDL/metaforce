#ifndef __URDE_CHUDBOSSENERGYINTERFACE_HPP__
#define __URDE_CHUDBOSSENERGYINTERFACE_HPP__

namespace urde
{

class CHudBossEnergyInterface
{
    float x0_alpha;
public:
    void SetAlpha(float a) { x0_alpha = a; }
    void Update(float dt);
};

}

#endif // __URDE_CHUDBOSSENERGYINTERFACE_HPP__
