#ifndef __PSHAG_CBOOLPOINODE_HPP__
#define __PSHAG_CBOOLPOINODE_HPP__

#include "CPOINode.hpp"

namespace urde
{

class CBoolPOINode : public CPOINode
{
    bool x38_val = false;
public:
    CBoolPOINode();
    CBoolPOINode(CInputStream& in);
    bool GetValue() const {return x38_val;}
};

}

#endif // __PSHAG_CBOOLPOINODE_HPP__
