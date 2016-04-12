#ifndef __PSHAG_CBOOLPOINODE_HPP__
#define __PSHAG_CBOOLPOINODE_HPP__

#include "CPOINode.hpp"

namespace urde
{
class IAnimSourceInfo;

class CBoolPOINode : public CPOINode
{
    bool x38_val = false;
public:
    CBoolPOINode();
    CBoolPOINode(CInputStream& in);
    bool GetValue() const {return x38_val;}
    static u32 _getPOIList(const CCharAnimTime& time,
                           CBoolPOINode* listOut,
                           u32 capacity, u32 iterator, u32 unk1,
                           const std::vector<CBoolPOINode>& stream,
                           const CCharAnimTime& curTime,
                           const IAnimSourceInfo& animInfo, u32 passedCount);
    static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                               const CCharAnimTime& startTime);
};

}

#endif // __PSHAG_CBOOLPOINODE_HPP__
