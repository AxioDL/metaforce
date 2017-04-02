#ifndef __URDE_CHUDHELMETINTERFACE_HPP__
#define __URDE_CHUDHELMETINTERFACE_HPP__

namespace urde
{
class CGuiFrame;

class CHudHelmetInterface
{
public:
    CHudHelmetInterface(CGuiFrame& helmetFrame);
    void UpdateHelmetAlpha();
};

}

#endif // __URDE_CHUDHELMETINTERFACE_HPP__
