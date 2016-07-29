#ifndef __URDE_CMAPPABLEOBJECT_HPP__
#define __URDE_CMAPPABLEOBJECT_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CStateManager;
class ITweakAutoMapper;
class CMappableObject
{
public:
    enum class EMappableObjectType
    {
        BlueDoor         = 0,
        ShieldDoor       = 1,
        IceDoor          = 2,
        WaveDoor         = 3,
        PlasmaDoor       = 4,
        BigDoor1         = 5,
        BigDoor2         = 6,
        IceDoorCeiling   = 7,
        IceDoorFloor     = 8,
        WaveDoorCeiling  = 9,
        WaveDoorFloor    = 10,
        IceDoorFloor2    = 13,
        WaveDoorFloor2   = 14,
        DownArrowYellow  = 27, /* Maintenance Tunnel */
        UpArrowYellow    = 28, /* Phazon Processing Center */
        DownArrowGreen   = 29, /* Elevator A */
        UpArrowGreen     = 30, /* Elite Control Access */
        DownArrowRed     = 31, /* Elevator B */
        UpArrowRed       = 32, /* Fungal Hall Access */
        TransportLift    = 33,
        SaveStation      = 34,
        MissileStation   = 37
    };

private:
    static const zeus::CVector3f skDoorVerts[8];

public:
    void PostConstruct(const void*);
    const zeus::CTransform& GetTransform() const;
    EMappableObjectType GetType() const;
    void Draw(int, const CStateManager&, float, bool) const;
    void DrawDoorSurface(int, const CStateManager&, float, int, bool) const;
    void BuildSurfaceCenterPoint(s32) const;
    bool IsDoorConnectedToArea(s32, const CStateManager&) const;
    bool IsDoorConnectedToVisitedArea(const CStateManager&) const;
    bool GetIsVisibleToAutoMapper(bool) const;
    bool GetIsSeen() const;

    void ReadAutoMapperTweaks(const ITweakAutoMapper&);
    static bool GetTweakIsMapVisibilityCheat();
    static bool IsDoorType(EMappableObjectType);
};
}
#endif // __URDE_CMAPPABLEOBJECT_HPP__
