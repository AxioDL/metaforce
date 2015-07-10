#ifndef __DNAMP1_STRG_HPP__
#define __DNAMP1_STRG_HPP__

#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

extern const HECL::FourCC ENGLfcc;
extern const HECL::FourCC FRENfcc;
extern const HECL::FourCC GERMfcc;
extern const HECL::FourCC SPANfcc;
extern const HECL::FourCC ITALfcc;
extern const HECL::FourCC JAPNfcc;

struct STRG : BigDNA
{
    DECL_EXPLICIT_DNA
    atUint32 version;
    atUint32 langCount;
    atUint32 strCount;

    struct Language
    {
        HECL::FourCC lang;
        std::vector<std::wstring> strings;
    };
    std::vector<Language> langs;
};

}
}

#endif // __DNAMP1_STRG_HPP__
