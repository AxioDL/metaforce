#ifndef __URDE_CINT32POINODE_HPP__
#define __URDE_CINT32POINODE_HPP__

#include "CPOINode.hpp"

namespace urde
{
class IAnimSourceInfo;

class CInt32POINode : public CPOINode
{
    s32 x38_val;
    std::string x3c_locatorName;
public:
    CInt32POINode();
    CInt32POINode(CInputStream& in);
    s32 GetValue() const {return x38_val;}
    const std::string& GetLocatorName() const {return x3c_locatorName;}

    static CInt32POINode CopyNodeMinusStartTime(const CInt32POINode& node,
                                                const CCharAnimTime& startTime);
};

}

#endif // __URDE_CINT32POINODE_HPP__
