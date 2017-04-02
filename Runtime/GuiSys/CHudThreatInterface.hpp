#ifndef __URDE_CHUDTHREATINTERFACE_HPP__
#define __URDE_CHUDTHREATINTERFACE_HPP__

namespace urde
{
class CGuiFrame;

class IHudThreatInterface
{
public:
    virtual ~IHudThreatInterface() = default;
};

class CHudThreatInterfaceCombat : public IHudThreatInterface
{
public:
    CHudThreatInterfaceCombat(CGuiFrame& selHud, float);
};

}

#endif // __URDE_CHUDTHREATINTERFACE_HPP__
