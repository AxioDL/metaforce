#ifndef __RETRO_CCALLSTACK_HPP__
#define __RETRO_CCALLSTACK_HPP__

namespace Retro
{

class CCallStack
{
    const char* x0_fileAndLineStr;
    const char* x4_typeStr;
public:
    CCallStack(char const* fileAndLineStr, char const* typeStr)
    : x0_fileAndLineStr(fileAndLineStr), x4_typeStr(typeStr) {}
    const char* GetFileAndLineText() const {return x0_fileAndLineStr;}
    const char* GetTypeText() const {return x4_typeStr;}
};

}

#endif // __RETRO_CCALLSTACK_HPP__
