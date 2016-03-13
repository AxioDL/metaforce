#ifndef __URDE_CGUIFUNCTIONDEF_HPP__
#define __URDE_CGUIFUNCTIONDEF_HPP__

#include "CGuiFuncParm.hpp"
#include <array>

namespace urde
{

class CGuiFunctionDef
{
    int x0_funcId;
    unsigned x4_parmCount = 0;
    std::array<CGuiFuncParm, 8> x8_parms;
    bool x48_flag;
public:
    const CGuiFuncParm& GetParm(int parm)
    {
        return x8_parms[parm];
    }

    CGuiFunctionDef(int funcId, bool,
                    const CGuiFuncParm& a, const CGuiFuncParm& b,
                    const CGuiFuncParm& c, const CGuiFuncParm& d);
    CGuiFunctionDef(int funcId, bool,
                    const CGuiFuncParm& a, const CGuiFuncParm& b);
    CGuiFunctionDef(int funcId, bool);
};

}

#endif // __URDE_CGUIFUNCTIONDEF_HPP__
