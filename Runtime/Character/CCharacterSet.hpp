#ifndef __URDE_CCHARACTERSET_HPP__
#define __URDE_CCHARACTERSET_HPP__

#include "IOStreams.hpp"
#include "CCharacterInfo.hpp"

namespace urde
{

class CCharacterSet
{
    u16 x0_version;
    std::map<u32, CCharacterInfo> x4_characters;
public:
    CCharacterSet(CInputStream& in);
    const std::map<u32, CCharacterInfo>& GetCharacterInfoMap() const {return x4_characters;}
};

}

#endif // __URDE_CCHARACTERSET_HPP__
