#ifndef __RETRO_CBASICS_HPP__
#define __RETRO_CBASICS_HPP__

#include <stdint.h>
#include <stdlib.h>

#include <Athena/IStreamReader.hpp>
#include <Athena/IStreamWriter.hpp>
using CInputStream = Athena::io::IStreamReader;
using COutputStream = Athena::io::IStreamWriter;

template<class T>
class TOneStatic
{
    static T m_allocspace;
    static uint32_t m_refCount;
public:
    static T* GetAllocSpace() {return &m_allocspace;}
    static uint32_t& ReferenceCount() {return m_refCount;}
    void* operator new(size_t) {++ReferenceCount(); return GetAllocSpace();}
    void operator delete(void*) {--ReferenceCount();}
};
template<class T> T TOneStatic<T>::m_allocspace;
template<class T> uint32_t TOneStatic<T>::m_refCount;

class CBasics
{
public:
    static void Init();
    static const char* Stringize(const char* fmt, ...);
};

#endif // __RETRO_CBASICS_HPP__
