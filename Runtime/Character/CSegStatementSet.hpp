#ifndef __URDE_CSEGSTATEMENTSET_HPP__
#define __URDE_CSEGSTATEMENTSET_HPP__

#include "CAnimPerSegmentData.hpp"

namespace urde
{
class CSegIdList;
class CCharLayoutInfo;

class CSegStatementSet
{
public:
    /* Used to be a pointer to arbitrary subclass-provided storage,
     * now it's a self-stored array */
    CAnimPerSegmentData x4_segData[100];
    void Add(const CSegIdList& list, const CCharLayoutInfo& layout,
             const CSegStatementSet& other, float weight);
};

}

#endif // __URDE_CSEGSTATEMENTSET_HPP__
