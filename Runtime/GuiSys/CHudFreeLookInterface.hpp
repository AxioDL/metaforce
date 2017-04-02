#ifndef __URDE_CHUDFREELOOKINTERFACE_HPP__
#define __URDE_CHUDFREELOOKINTERFACE_HPP__

namespace urde
{
class CGuiFrame;

class IFreeLookInterface
{
public:
    virtual ~IFreeLookInterface() = default;
};

class CHudFreeLookInterfaceCombat : public IFreeLookInterface
{
public:
    CHudFreeLookInterfaceCombat(CGuiFrame& selHud, bool, bool, bool, bool grapplePoint);
};

class CHudFreeLookInterfaceXRay : public IFreeLookInterface
{
public:
    CHudFreeLookInterfaceXRay(CGuiFrame& selHud, bool, bool, bool grapplePoint);
};

}

#endif // __URDE_CHUDFREELOOKINTERFACE_HPP__
