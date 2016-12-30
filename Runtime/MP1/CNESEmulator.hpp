#ifndef __URDE_CNESEMULATOR_HPP__
#define __URDE_CNESEMULATOR_HPP__

namespace urde
{
class CFinalInput;

namespace MP1
{

class CNESEmulator
{
public:
    void ProcessUserInput(const CFinalInput& input, int);
};

}
}

#endif // __URDE_CNESEMULATOR_HPP__
