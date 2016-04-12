#include "CSkinBank.hpp"

namespace urde
{

CSkinBank::CSkinBank(CInputStream& in)
{
    u32 boneCount = in.readUint32Big();
    x0_segments.reserve(boneCount);
    for (u32 i=0 ; i<boneCount ; ++i)
        x0_segments.emplace_back(in);
}

}
