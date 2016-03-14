#ifndef __URDE_CGUIPHYSICALMSG_HPP__
#define __URDE_CGUIPHYSICALMSG_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"

namespace urde
{
enum class EPhysicalControllerID
{
    Seventeen = 17,
    Eighteen = 18,
    Nineteen = 19,
    Twenty = 20,
    TwentyOne = 21,
    TwentyTwo = 22,
    TwentyThree = 23,
    TwentyFour = 24,
    ThirtyThree = 33,
    ThirtyFour = 34,
    ThirtyFive = 35,
    ThirtySix = 36,
    ThirtySeven = 37,
    ThirtyEight = 38,
    ThirtyNine = 39,
    Forty = 40,
    FortyOne = 41,
    FortyTwo = 42,
    FortyThree = 43,
    FortyFour = 44,
    FortyFive = 45,
    FortySix = 46,
    FortySeven = 47,
    FortyEight = 48,
    TwentyFive = 25,
    TwentySix = 26,
    TwentySeven = 27,
    TwentyEight = 28,
    TwentyNine = 29,
    Thirty = 30,
    ThirtyOne = 31,
    ThirtyTwo = 32
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
        bool x4_active;
        CPhysicalID(EPhysicalControllerID key, bool active) : x0_key(key), x4_active(active) {}
    };
    using PhysicalMap = std::unordered_map<EPhysicalControllerID, CPhysicalID>;

    CGuiPhysicalMsg(const PhysicalMap& map);
    bool Exists(const CGuiPhysicalMsg& other) const;
    void SetMessage(const PhysicalMap& map, bool pressed);
    static void AddControllerID(PhysicalMap& map, EPhysicalControllerID key, bool pressed);

    enum class ControllerState
    {
        NotFound = -1,
        NotPressed = 0,
        Pressed = 1
    };
    static ControllerState FindControllerID(const PhysicalMap& map, EPhysicalControllerID key);
};

}

#endif // __URDE_CGUIPHYSICALMSG_HPP__
