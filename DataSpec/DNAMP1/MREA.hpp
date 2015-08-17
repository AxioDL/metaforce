#ifndef __DNAMP1_MREA_HPP__
#define __DNAMP1_MREA_HPP__

#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

struct MREA : BigDNA
{
    DECL_DNA
    Value<atUint32> magic;

};

}
}

#endif
