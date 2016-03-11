#ifndef __URDE_CGUIFUNCPARM_HPP__
#define __URDE_CGUIFUNCPARM_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CGuiFuncParm
{
public:
    enum class Type
    {
        Null = -1,
        Int = 0,
        Float = 1,
        IntrusivePointer = 2
    };

private:
    Type x0_type = Type::Null;
    union
    {
        u32 x4_int;
        float x4_float;
        void* x4_ptr = nullptr;
    };

public:
    CGuiFuncParm() = default;
    CGuiFuncParm(int arg) : x0_type(Type::Int), x4_int(arg) {}
    CGuiFuncParm(float arg) : x0_type(Type::Float), x4_float(arg) {}
    CGuiFuncParm(void* arg) : x0_type(Type::IntrusivePointer), x4_ptr(arg) {}
    ~CGuiFuncParm()
    {
        if (x0_type == Type::IntrusivePointer)
            delete static_cast<u8*>(x4_ptr);
    }
};

}

#endif // __URDE_CGUIFUNCPARM_HPP__
