#ifndef __URDE_CGUIRANDOMVAR_HPP__
#define __URDE_CGUIRANDOMVAR_HPP__

namespace urde
{

class CGuiRandomVar
{
    enum class Type
    {
        Zero,
        One
    } x0_type;
    float x4_num;

    float GenNum() const
    {
        switch (x0_type)
        {
        case Type::Zero:
        case Type::One:
            return x4_num;
        default: break;
        }
        return 0.f;
    }
};

}

#endif // __URDE_CGUIRANDOMVAR_HPP__
