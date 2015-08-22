#ifndef __RSTL_HPP__
#define __RSTL_HPP__

#include <vector>
#include <stdlib.h>

namespace rstl
{

/**
 * @brief Vector reserved on construction
 */
template <class T, size_t N>
class reserved_vector : public std::vector<T>
{
public:
    reserved_vector() {this->reserve(N);}
};

/**
 * @brief Simple data/refcount pair
 */
class CRefData
{
    void* m_ptr;
    int m_refCount;
public:
    CRefData() : m_ptr(nullptr), m_refCount(0xffffff) {}
    CRefData(void* ptr) : m_ptr(ptr), m_refCount(0) {}

    void* GetPtr() const {return m_ptr;}
    int AddRef() {return ++m_refCount;}
    int DelRef() {return --m_refCount;}

    static CRefData sNull;
};

}

#endif // __RSTL_HPP__
