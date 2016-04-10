#include "CCharacterInfo.hpp"

namespace urde
{

static std::vector<std::pair<u32, std::pair<std::string, std::string>>>
MakeAnimInfoVector(CInputStream& in)
{
    std::vector<std::pair<u32, std::pair<std::string, std::string>>> ret;
    u32 animInfoCount = in.readUint32Big();
    ret.reserve(animInfoCount);
    for (u32 i=0 ; i<animInfoCount ; ++i)
    {
        u32 idx = in.readUint32Big();
        std::string a = in.readString();
        std::string b = in.readString();
        ret.emplace_back(idx, std::make_pair(a, b));
    }
    return ret;
}

CCharacterInfo::CCharacterInfo(CInputStream& in)
: x0_tableCount(in.readUint16Big()),
  x4_name(in.readString()),
  x14_cmdl(in.readUint32Big()),
  x18_cskr(in.readUint32Big()),
  x1c_cinf(in.readUint32Big()),
  x20_animInfo(MakeAnimInfoVector(in)),
  x30_pasDatabase(in)
{}

}
