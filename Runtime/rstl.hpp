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
    void* x0_ptr;
    int x4_refCount;
public:
    CRefData() : x0_ptr(nullptr), x4_refCount(0xffffff) {}
    CRefData(void* ptr) : x0_ptr(ptr), x4_refCount(0) {}

    void* GetPtr() const {return x0_ptr;}
    int AddRef() {return ++x4_refCount;}
    int DelRef() {return --x4_refCount;}

    static CRefData sNull;
};

template<class T>
class ncrc_ptr;

/**
 * @brief Reference-counted shared smart pointer
 */
template<class T>
class rc_ptr
{
    CRefData* m_aux;
    friend class ncrc_ptr<T>;
public:
    rc_ptr() : m_aux(&CRefData::sNull) {m_aux->AddRef();}
    rc_ptr(void* ptr) : m_aux(new CRefData(ptr)) {m_aux->AddRef();}
    rc_ptr(const rc_ptr<T>& other) : m_aux(other.m_aux) {m_aux->AddRef();}
    rc_ptr(rc_ptr<T>&& other) : m_aux(other.m_aux) {other.m_aux = nullptr;}
    rc_ptr(const ncrc_ptr<T>& other) : m_aux(other.m_aux) {m_aux->AddRef();}
    ~rc_ptr()
    {
        if (m_aux && !m_aux->DelRef())
        {
            delete static_cast<T*>(m_aux->GetPtr());
            delete m_aux;
        }
    }

    T* operator->() const {return static_cast<T*>(m_aux->GetPtr());}
    T& operator*() const {return *static_cast<T*>(m_aux->GetPtr());}
    T* get() const {return static_cast<T*>(m_aux->GetPtr());}
    operator bool() {return m_aux->GetPtr() != nullptr;}
};

/**
 * @brief Non-reference-counted shared smart pointer
 */
template<class T>
class ncrc_ptr
{
    CRefData* m_aux;
    friend class rc_ptr<T>;
public:
    ncrc_ptr(const rc_ptr<T>& other) : m_aux(other.m_aux) {}
};

}

#endif // __RSTL_HPP__
