#ifndef __URDE_MP1_CPREFRONTEND_HPP__
#define __URDE_MP1_CPREFRONTEND_HPP__

#include "CIOWin.hpp"

namespace urde
{
namespace MP1
{

class CPreFrontEnd : public CIOWin
{
public:
    CPreFrontEnd();
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
};

}
}

#endif // __URDE_MP1_CPREFRONTEND_HPP__
