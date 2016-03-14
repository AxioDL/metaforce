#ifndef __URDE_CGUIMESSAGE_HPP__
#define __URDE_CGUIMESSAGE_HPP__

#include "RetroTypes.hpp"
#include "CGuiFuncParm.hpp"

namespace urde
{

class CGuiMessage
{
public:
    enum class Type
    {
        Eighteen = 18,
        Nineteen = 19
    };
private:
    Type x4_type;
    CGuiFuncParm x8_a;
    CGuiFuncParm x10_b;
public:
    CGuiMessage(Type type, uintptr_t a, uintptr_t b)
    : x4_type(type), x8_a(a), x10_b(b) {}
    uintptr_t GetInt(u32 val) const
    {
        if (val == 0x60EF4DB0)
            return x8_a.x4_int;
        else
            return x10_b.x4_int;
    }

};

}

#endif // __URDE_CGUIMESSAGE_HPP__
