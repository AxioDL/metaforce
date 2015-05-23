#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>

namespace HECLHelpers
{
bool IsRegularFile(const std::string& path);
bool IsDirectoryFile(const std::string& path);
bool ContainsMagic(const std::string& path, const char* magicBuf,
                   size_t magicLen, size_t magicOff=0);

bool IsBlenderFile(const std::string& path);

}

#endif // HELPERS_HPP
