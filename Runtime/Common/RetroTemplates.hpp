#ifndef __RETRO_TEMPLATES_HPP__
#define __RETRO_TEMPLATES_HPP__

#include "GCNTypes.hpp"

namespace Retro
{
namespace Common
{

/**
 * @brief Inheritable singleton static-allocator
 */
template<class T>
class TOneStatic
{
    static T m_allocspace;
    static uint32_t m_refCount;
public:
    static T* GetAllocSpace() {return &m_allocspace;}
    static u32& ReferenceCount() {return m_refCount;}
    void* operator new(size_t) {++ReferenceCount(); return GetAllocSpace();}
    void operator delete(void*) {--ReferenceCount();}
};
template<class T> T TOneStatic<T>::m_allocspace;
template<class T> u32 TOneStatic<T>::m_refCount;

}
}

#endif // __RETRO_TEMPLATES_HPP__
