#ifndef __URDE_CHUDTHREATINTERFACE_HPP__
#define __URDE_CHUDTHREATINTERFACE_HPP__

namespace urde
{
class CGuiFrame;

class IHudThreatInterface
{
public:
    virtual ~IHudThreatInterface() = default;
    virtual void Update(float dt)=0;
};

class CHudThreatInterfaceCombat : public IHudThreatInterface
{
public:
    CHudThreatInterfaceCombat(CGuiFrame& selHud, float);
    void Update(float dt);
};

}

#endif // __URDE_CHUDTHREATINTERFACE_HPP__
