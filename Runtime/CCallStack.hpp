#ifndef __URDE_CCALLSTACK_HPP__
#define __URDE_CCALLSTACK_HPP__

namespace urde
{

class CCallStack
{
    const char* x0_fileAndLineStr;
    const char* x4_typeStr;
public:
    CCallStack(const char* fileAndLineStr, const char* typeStr)
    : x0_fileAndLineStr(fileAndLineStr), x4_typeStr(typeStr) {}
    const char* GetFileAndLineText() const {return x0_fileAndLineStr;}
    const char* GetTypeText() const {return x4_typeStr;}
};

}

#endif // __URDE_CCALLSTACK_HPP__
