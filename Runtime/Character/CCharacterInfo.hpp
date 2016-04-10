#ifndef __PSHAG_CCHARACTERINFO_HPP__
#define __PSHAG_CCHARACTERINFO_HPP__

#include "IOStreams.hpp"
#include "CPASDatabase.hpp"

namespace urde
{

class CCharacterInfo
{
    u16 x0_tableCount;
    std::string x4_name;
    TResId x14_cmdl;
    TResId x18_cskr;
    TResId x1c_cinf;
    std::vector<std::pair<u32, std::pair<std::string, std::string>>> x20_animInfo;
    CPASDatabase x30_pasDatabase;

public:
    CCharacterInfo(CInputStream& in);
};

}

#endif // __PSHAG_CCHARACTERINFO_HPP__
