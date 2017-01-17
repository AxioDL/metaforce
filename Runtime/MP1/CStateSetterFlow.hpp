#ifndef __URDE_MP1_CSTATESETTERFLOW_HPP__
#define __URDE_MP1_CSTATESETTERFLOW_HPP__

#include "CIOWin.hpp"

namespace urde
{
namespace MP1
{

class CStateSetterFlow : public CIOWin
{
public:
    CStateSetterFlow();
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
};

}
}

#endif // __URDE_MP1_CSTATESETTERFLOW_HPP__
