#ifndef CRUMBLEGENERATOR_HPP
#define CRUMBLEGENERATOR_HPP

#include "CRumbleVoice.hpp"

namespace urde
{
class CRumbleGenerator
{
public:
    CRumbleGenerator();
    void Update(float);
    void HardStopAll();
};
}

#endif // CRUMBLEGENERATOR_HPP
