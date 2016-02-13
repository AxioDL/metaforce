#ifndef __PSHAG_CFACTORYMGR_HPP__
#define __PSHAG_CFACTORYMGR_HPP__

#include <unordered_map>
#include "RetroTypes.hpp"

namespace pshag
{
class SObjectTag;
class CVParamTransfer;
class CInputStream;

typedef void(*CFactoryFnReturn)(const SObjectTag&, CInputStream&, const CVParamTransfer&);

class CFactoryMgr
{
    std::unordered_map<u32, CFactoryFnReturn> m_factories;
public:
    MakeObjectFromMemory(const SObjectTag&, void*, int, bool, const CVParamTransfer&);
    void AddFactory(FourCC key, CFactoryFnReturn func)
    {
        m_factories[key] = func;
    }
};

}

#endif // __PSHAG_CFACTORYMGR_HPP__
