#ifndef __URDE_CHUDHELMETINTERFACE_HPP__
#define __URDE_CHUDHELMETINTERFACE_HPP__

namespace urde
{
class CGuiFrame;

class CHudHelmetInterface
{
public:
    CHudHelmetInterface(CGuiFrame& helmetFrame);
    void Update(float dt);
    void SetIsVisibleDebug(bool helmet, bool glow);
    void UpdateCameraDebugSettings(float fov, float y, float z);
    void UpdateHelmetAlpha();
};

}

#endif // __URDE_CHUDHELMETINTERFACE_HPP__
