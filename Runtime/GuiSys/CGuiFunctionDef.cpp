#include "CGuiFunctionDef.hpp"

namespace urde
{

CGuiFunctionDef::CGuiFunctionDef(int funcId, bool flag,
                                 const CGuiFuncParm& a, const CGuiFuncParm& b,
                                 const CGuiFuncParm& c, const CGuiFuncParm& d)
: x0_funcId(funcId), x4_parmCount(4), x48_flag(flag)
{
    x8_parms[0] = a;
    x8_parms[1] = b;
    x8_parms[2] = c;
    x8_parms[3] = d;
}

CGuiFunctionDef::CGuiFunctionDef(int funcId, bool flag,
                                 const CGuiFuncParm& a, const CGuiFuncParm& b)
: x0_funcId(funcId), x4_parmCount(2), x48_flag(flag)
{
    x8_parms[0] = a;
    x8_parms[1] = b;
}

CGuiFunctionDef::CGuiFunctionDef(int funcId, bool flag)
: x0_funcId(funcId), x48_flag(flag)
{
}

}
