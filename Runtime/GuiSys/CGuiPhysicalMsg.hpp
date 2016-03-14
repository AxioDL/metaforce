#ifndef __URDE_CGUIPHYSICALMSG_HPP__
#define __URDE_CGUIPHYSICALMSG_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"

namespace urde
{
enum class EPhysicalControllerID
{
    AInst = 1,
    A = 2,
    BInst = 3,
    B = 4,
    LInst = 5,
    L = 6,
    RInst = 7,
    R = 8,
    XInst = 9,
    X = 10,
    YInst = 11,
    Y = 12,
    ZInst = 13,
    Z = 14,
    StartInst = 15,
    Start = 16,

    DPadUpInst = 17,
    DPadUp = 18,
    DPadRightInst = 19,
    DPadRight = 20,
    DPadDownInst = 21,
    DPadDown = 22,
    DPadLeftInst = 23,
    DPadLeft = 24,

    LeftUpInst = 25,
    LeftUp = 26,
    LeftRightInst = 27,
    LeftRight = 28,
    LeftDownInst = 29,
    LeftDown = 30,
    LeftLeftInst = 31,
    LeftLeft = 32,

    LeftStickUpInst = 33,
    LeftStickUp = 34,
    LeftStickRightInst = 35,
    LeftStickRight = 36,
    LeftStickDownInst = 37,
    LeftStickDown = 38,
    LeftStickLeftInst = 39,
    LeftStickLeft = 40,

    RightStickUpInst = 41,
    RightStickUp = 42,
    RightStickRightInst = 43,
    RightStickRight = 44,
    RightStickDownInst = 45,
    RightStickDown = 46,
    RightStickLeftInst = 47,
    RightStickLeft = 48,

    LeftStickThreshold = 49,
    LeftStickRelease = 50,
    RightStickThreshold = 51,
    RightStickRelease = 52
};
}

namespace std
{
template <> struct hash<urde::EPhysicalControllerID>
{
    size_t operator() (const urde::EPhysicalControllerID& id) const noexcept {return size_t(id);}
};
}

namespace urde
{

struct CGuiPhysicalMsg
{
    u64 x0_curStates = 0;
    u64 x8_repeatStates = 0;

    struct CPhysicalID
    {
        EPhysicalControllerID x0_key;
        bool x4_repeat;
        CPhysicalID(EPhysicalControllerID key, bool repeat) : x0_key(key), x4_repeat(repeat) {}
    };
    using PhysicalMap = std::unordered_map<EPhysicalControllerID, CPhysicalID>;

    CGuiPhysicalMsg(const PhysicalMap& map);
    bool Exists(const CGuiPhysicalMsg& other) const;
    void SetMessage(const PhysicalMap& map, bool pressed);
    static void AddControllerID(PhysicalMap& map, EPhysicalControllerID key, bool repeat);

    enum class EControllerState
    {
        NotPressed = -1,
        Press = 0,
        PressRepeat = 1
    };
    static EControllerState FindControllerID(const PhysicalMap& map, EPhysicalControllerID key);
};

}

#endif // __URDE_CGUIPHYSICALMSG_HPP__
