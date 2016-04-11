#ifndef __PSHAG_CINT32POINODE_HPP__
#define __PSHAG_CINT32POINODE_HPP__

#include "CPOINode.hpp"

namespace urde
{

class CInt32POINode : public CPOINode
{
    u32 x38_val;
    std::string x3c_boneName;
public:
    CInt32POINode(CInputStream& in);
};

}

#endif // __PSHAG_CINT32POINODE_HPP__
