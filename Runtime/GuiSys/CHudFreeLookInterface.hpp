#ifndef __URDE_CHUDFREELOOKINTERFACE_HPP__
#define __URDE_CHUDFREELOOKINTERFACE_HPP__

namespace urde
{
class CGuiFrame;

class IFreeLookInterface
{
public:
    virtual ~IFreeLookInterface() = default;
    virtual void Update(float dt)=0;
};

class CHudFreeLookInterfaceCombat : public IFreeLookInterface
{
public:
    CHudFreeLookInterfaceCombat(CGuiFrame& selHud, bool, bool, bool, bool grapplePoint);
    void Update(float dt);
};

class CHudFreeLookInterfaceXRay : public IFreeLookInterface
{
public:
    CHudFreeLookInterfaceXRay(CGuiFrame& selHud, bool, bool, bool grapplePoint);
    void Update(float dt);
};

}

#endif // __URDE_CHUDFREELOOKINTERFACE_HPP__
