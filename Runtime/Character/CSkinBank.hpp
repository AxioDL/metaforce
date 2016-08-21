#ifndef __URDE_CSKINBANK_HPP__
#define __URDE_CSKINBANK_HPP__

#include "IOStreams.hpp"
#include "CSegId.hpp"

namespace urde
{
class CPoseAsTransforms;

class CSkinBank
{
    std::vector<CSegId> x0_segments;
public:
    CSkinBank(CInputStream& in);
    void GetBankTransforms(std::vector<const zeus::CTransform*>& out,
                           const CPoseAsTransforms& pose) const;
};

}

#endif // __URDE_CSKINBANK_HPP__
