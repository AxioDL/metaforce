#ifndef __RETRO_TEMPLATES_HPP__
#define __RETRO_TEMPLATES_HPP__

#include <utility>
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
    static u8 m_allocspace[sizeof(T)];
    static uint32_t m_refCount;
public:
    static T* GetAllocSpace() {return (T*)m_allocspace;}
    static u32& ReferenceCount() {return m_refCount;}
    T* operator->() const {return (T*)m_allocspace;}

    void* operator new(size_t) = delete;
    void operator delete(void*) = delete;

    template<typename U = T>
    TOneStatic(typename std::enable_if<!std::is_default_constructible<U>::value>::type* = 0)
    {++ReferenceCount();}
    template<typename U = T>
    TOneStatic(typename std::enable_if<std::is_default_constructible<U>::value>::type* = 0)
    {++ReferenceCount(); new (GetAllocSpace()) T();}

    template<typename... Args> TOneStatic(Args&&... args)
    {++ReferenceCount(); new (GetAllocSpace()) T(std::forward<Args>(args)...);}

    ~TOneStatic() {--ReferenceCount();}

    template<typename... Args> void reset(Args&&... args)
    {new (GetAllocSpace()) T(std::forward<Args>(args)...);}
};
template<class T> u8 TOneStatic<T>::m_allocspace[sizeof(T)];
template<class T> u32 TOneStatic<T>::m_refCount;

}
}

#endif // __RETRO_TEMPLATES_HPP__
