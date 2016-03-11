#ifndef __URDE_CGUIAUTOREPEATDATA_HPP__
#define __URDE_CGUIAUTOREPEATDATA_HPP__

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

class CGuiAutoRepeatData
{
    EPhysicalControllerID x0_a;
    EPhysicalControllerID x4_b;
    float x8_[8] = {0.f, 0.f, 0.f, 0.f, 0.2f, 0.2f, 0.2f, 0.2f};
    bool x28_[8] = {};
public:
    CGuiAutoRepeatData(EPhysicalControllerID a, EPhysicalControllerID b)
    : x0_a(a), x4_b(b) {}
};

}

#endif // __URDE_CGUIAUTOREPEATDATA_HPP__
