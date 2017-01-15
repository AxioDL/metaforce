#ifndef __URDE_CSTRINGEXTRAS_HPP__
#define __URDE_CSTRINGEXTRAS_HPP__

#include <string>
#include <string.h>

namespace urde
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

    static int IndexOfSubstring(const std::string& haystack, const std::string& needle)
    {
        std::string str = haystack;
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::string::size_type s = str.find(needle);
        if (s == std::string::npos)
            return -1;
        return s;
    }
};

}

#endif // __URDE_CSTRINGEXTRAS_HPP__
