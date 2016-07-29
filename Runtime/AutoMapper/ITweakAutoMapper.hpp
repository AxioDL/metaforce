#ifndef __URDE_ITWEAKAUTOMAPPER_HPP__
#define __URDE_ITWEAKAUTOMAPPER_HPP__

#include "zeus/CVector3f.hpp"
#include "ITweak.hpp"

namespace urde
{
class ITweakAutoMapper : public ITweak
{
public:
    virtual ~ITweakAutoMapper() {}
    virtual const zeus::CVector3f& GetDoorCenter() const=0;
};
}

#endif // __URDE_ITWEAKAUTOMAPPER_HPP__
