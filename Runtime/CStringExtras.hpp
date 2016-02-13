#ifndef __PSHAG_CSTRINGEXTRAS_HPP__
#define __PSHAG_CSTRINGEXTRAS_HPP__

#include <string>
#include <string.h>

namespace pshag
{

class CStringExtras
{
public:
    static int CompareCaseInsensitive(const char* a, const char* b)
    {
#if _WIN32
        return _stricmp(a, b);
#else
        return strcasecmp(a, b);
#endif
    }
    static int CompareCaseInsensitive(const std::string& a, const std::string& b)
    {
        return CompareCaseInsensitive(a.c_str(), b.c_str());
    }
};

}

#endif // __PSHAG_CSTRINGEXTRAS_HPP__
