#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include "helpers.hpp"

namespace HECLHelpers
{

bool IsRegularFile(const std::string& path)
{
    struct stat theStat;
    if (stat(path.c_str(), &theStat))
        return false;
    if (!S_ISREG(theStat.st_mode))
        return false;
    return true;
}

bool IsDirectoryFile(const std::string& path)
{
    struct stat theStat;
    if (stat(path.c_str(), &theStat))
        return false;
    if (!S_ISDIR(theStat.st_mode))
        return false;
    return true;
}

bool ContainsMagic(const std::string& path, const char* magicBuf,
                   size_t magicLen, size_t magicOff)
{
    if (!IsRegularFile(path))
        return false;

    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return false;

    char* readBuf[magicLen];
    fseek(fp, magicOff, SEEK_SET);
    size_t readLen = fread(readBuf, 1, magicLen, fp);
    fclose(fp);

    if (readLen < magicLen)
        return false;
    if (memcmp(readBuf, magicBuf, magicLen))
        return false;
    return true;
}

}
