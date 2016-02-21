#ifndef CRUMBLEGENERATOR_HPP
#define CRUMBLEGENERATOR_HPP

#include "CRumbleVoice.hpp"

namespace pshag
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
