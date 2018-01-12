#ifndef __RSTL_HPP__
#define __RSTL_HPP__

#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdlib>
#include "optional.hpp"
#include "logvisor/logvisor.hpp"

namespace rstl
{

#ifndef NDEBUG
static logvisor::Module Log("rstl");
#endif

template <typename T>
using optional_object = std::experimental::optional<T>;

/**
 * @brief Base vector backed by statically-allocated array
 */
template <class T, size_t N>
class _reserved_vector_base
{
protected:
    union alignas(T) storage_t
    {
        struct {} _dummy;
        T _value;
        storage_t() : _dummy() {}
        ~storage_t() {}
    };
    explicit _reserved_vector_base(size_t _init_sz) : x0_size(_init_sz) {}
    size_t x0_size;
    storage_t x4_data[N];
    T& _value(std::ptrdiff_t idx) { return x4_data[idx]._value; }
    const T& _value(std::ptrdiff_t idx) const { return x4_data[idx]._value; }
    template <typename Tp>
    static void destroy(Tp& t, std::enable_if_t<std::is_destructible<Tp>::value &&
        !std::is_trivially_destructible<Tp>::value>* = 0) { t.Tp::~Tp(); }
    template <typename Tp>
    static void destroy(Tp& t, std::enable_if_t<!std::is_destructible<Tp>::value ||
        std::is_trivially_destructible<Tp>::value>* = 0) {}

public:
    class const_iterator
    {
        friend class _reserved_vector_base;
    protected:
        const T* m_val;
        explicit const_iterator(const T* val) : m_val(val) {}
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        const T& operator*() const { return *m_val; }
        const T* operator->() const { return m_val; }
        const_iterator& operator++() { ++m_val; return *this; }
        const_iterator& operator--() { --m_val; return *this; }
        const_iterator operator++(int) { auto ret = *this; ++m_val; return ret; }
        const_iterator operator--(int) { auto ret = *this; --m_val; return ret; }
        bool operator!=(const const_iterator& other) const { return m_val != other.m_val; }
        bool operator==(const const_iterator& other) const { return m_val == other.m_val; }
        const_iterator operator+(std::ptrdiff_t i) const { return const_iterator(m_val + i); }
        const_iterator operator-(std::ptrdiff_t i) const { return const_iterator(m_val - i); }
        const_iterator& operator+=(std::ptrdiff_t i) { m_val += i; return *this; }
        const_iterator& operator-=(std::ptrdiff_t i) { m_val -= i; return *this; }
        std::ptrdiff_t operator-(const const_iterator& it) const { return m_val - it.m_val; }
        bool operator>(const const_iterator& it) const { return m_val > it.m_val; }
        bool operator<(const const_iterator& it) const { return m_val < it.m_val; }
        bool operator>=(const const_iterator& it) const { return m_val >= it.m_val; }
        bool operator<=(const const_iterator& it) const { return m_val <= it.m_val; }
        const T& operator[](std::ptrdiff_t i) const { return m_val[i]; }
    };

    class iterator : public const_iterator
    {
        friend class _reserved_vector_base;
        explicit iterator(T* val) : const_iterator(val) {}
    public:
        T& operator*() const { return *const_cast<T*>(const_iterator::m_val); }
        T* operator->() const { return const_cast<T*>(const_iterator::m_val); }
        iterator& operator++() { ++const_iterator::m_val; return *this; }
        iterator& operator--() { --const_iterator::m_val; return *this; }
        iterator operator++(int) { auto ret = *this; ++const_iterator::m_val; return ret; }
        iterator operator--(int) { auto ret = *this; --const_iterator::m_val; return ret; }
        iterator operator+(std::ptrdiff_t i) const { return iterator(const_cast<T*>(const_iterator::m_val) + i); }
        iterator operator-(std::ptrdiff_t i) const { return iterator(const_cast<T*>(const_iterator::m_val) - i); }
        iterator& operator+=(std::ptrdiff_t i) { const_iterator::m_val += i; return *this; }
        iterator& operator-=(std::ptrdiff_t i) { const_iterator::m_val -= i; return *this; }
        std::ptrdiff_t operator-(const iterator& it) const { return const_iterator::m_val - it.m_val; }
        T& operator[](std::ptrdiff_t i) const { return const_cast<T*>(const_iterator::m_val)[i]; }
    };

    class const_reverse_iterator
    {
        friend class _reserved_vector_base;
    protected:
        const T* m_val;
        explicit const_reverse_iterator(const T* val) : m_val(val) {}
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        const T& operator*() const { return *m_val; }
        const T* operator->() const { return m_val; }
        const_reverse_iterator& operator++() { --m_val; return *this; }
        const_reverse_iterator& operator--() { ++m_val; return *this; }
        const_reverse_iterator operator++(int) { auto ret = *this; --m_val; return ret; }
        const_reverse_iterator operator--(int) { auto ret = *this; ++m_val; return ret; }
        bool operator!=(const const_reverse_iterator& other) const { return m_val != other.m_val; }
        bool operator==(const const_reverse_iterator& other) const { return m_val == other.m_val; }
        const_reverse_iterator operator+(std::ptrdiff_t i) const { return const_reverse_iterator(m_val - i); }
        const_reverse_iterator operator-(std::ptrdiff_t i) const { return const_reverse_iterator(m_val + i); }
        const_reverse_iterator& operator+=(std::ptrdiff_t i) { m_val -= i; return *this; }
        const_reverse_iterator& operator-=(std::ptrdiff_t i) { m_val += i; return *this; }
        std::ptrdiff_t operator-(const const_reverse_iterator& it) const { return it.m_val - m_val; }
        bool operator>(const const_iterator& it) const { return it.m_val > m_val; }
        bool operator<(const const_iterator& it) const { return it.m_val < m_val; }
        bool operator>=(const const_iterator& it) const { return it.m_val >= m_val; }
        bool operator<=(const const_iterator& it) const { return it.m_val <= m_val; }
        const T& operator[](std::ptrdiff_t i) const { return m_val[-i]; }
    };

    class reverse_iterator : public const_reverse_iterator
    {
        friend class _reserved_vector_base;
        explicit reverse_iterator(T* val) : const_reverse_iterator(val) {}
    public:
        T& operator*() const { return *const_cast<T*>(const_reverse_iterator::m_val); }
        T* operator->() const { return const_cast<T*>(const_reverse_iterator::m_val); }
        reverse_iterator& operator++() { --const_reverse_iterator::m_val; return *this; }
        reverse_iterator& operator--() { ++const_reverse_iterator::m_val; return *this; }
        reverse_iterator operator++(int) { auto ret = *this; --const_reverse_iterator::m_val; return ret; }
        reverse_iterator operator--(int) { auto ret = *this; ++const_reverse_iterator::m_val; return ret; }
        reverse_iterator operator+(std::ptrdiff_t i) const
        { return reverse_iterator(const_cast<T*>(const_reverse_iterator::m_val) - i); }
        reverse_iterator operator-(std::ptrdiff_t i) const
        { return reverse_iterator(const_cast<T*>(const_reverse_iterator::m_val) + i); }
        reverse_iterator& operator+=(std::ptrdiff_t i) { const_reverse_iterator::m_val -= i; return *this; }
        reverse_iterator& operator-=(std::ptrdiff_t i) { const_reverse_iterator::m_val += i; return *this; }
        std::ptrdiff_t operator-(const reverse_iterator& it) const { return it.m_val - const_reverse_iterator::m_val; }
        T& operator[](std::ptrdiff_t i) const { return const_cast<T*>(const_reverse_iterator::m_val)[-i]; }
    };

    size_t size() const noexcept { return x0_size; }
    bool empty() const noexcept { return x0_size == 0; }
    constexpr size_t capacity() const noexcept { return N; }
    const T* data() const noexcept { return std::addressof(_value(0)); }
    T* data() noexcept { return std::addressof(_value(0)); }

    T& back() { return _value(x0_size - 1); }
    T& front() { return _value(0); }
    const T& back() const { return _value(x0_size - 1); }
    const T& front() const { return _value(0); }

    const_iterator begin() const noexcept { return const_iterator(std::addressof(_value(0))); }
    const_iterator end() const noexcept { return const_iterator(std::addressof(_value(x0_size))); }
    iterator begin() noexcept { return iterator(std::addressof(_value(0))); }
    iterator end() noexcept { return iterator(std::addressof(_value(x0_size))); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    const_reverse_iterator rbegin() const noexcept
    { return const_reverse_iterator(std::addressof(_value(x0_size - 1))); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(std::addressof(_value(-1))); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(std::addressof(_value(x0_size - 1))); }
    reverse_iterator rend() noexcept { return reverse_iterator(std::addressof(_value(-1))); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    T& operator[](size_t idx) { return _value(idx); }
    const T& operator[](size_t idx) const { return _value(idx); }

protected:
    static iterator _const_cast_iterator(const const_iterator& it) { return iterator(const_cast<T*>(it.m_val)); }
};

/**
 * @brief Vector backed by statically-allocated array with uninitialized storage
 */
template <class T, size_t N>
class reserved_vector : public _reserved_vector_base<T, N>
{
public:
    using base = _reserved_vector_base<T, N>;
    using iterator = typename base::iterator;
    using const_iterator = typename base::const_iterator;
    reserved_vector() : base(0) {}

    reserved_vector(const reserved_vector& other) : base(other.x0_size)
    {
        for (size_t i=0 ; i<base::x0_size ; ++i)
            ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(other.base::_value(i));
    }

    reserved_vector& operator=(const reserved_vector& other)
    {
        size_t i = 0;
        if (other.base::x0_size > base::x0_size)
        {
            for (; i<base::x0_size ; ++i)
                base::_value(i) = other.base::_value(i);
            for (; i<other.base::x0_size ; ++i)
                ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(other.base::_value(i));
        }
        else if (other.base::x0_size < base::x0_size)
        {
            for (; i<other.base::x0_size ; ++i)
                base::_value(i) = other.base::_value(i);
            if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
                for (; i<base::x0_size ; ++i)
                    base::destroy(base::_value(i));
        }
        else
        {
            for (; i<other.base::x0_size ; ++i)
                base::_value(i) = other.base::_value(i);
        }
        base::x0_size = other.base::x0_size;
        return *this;
    }

    reserved_vector(reserved_vector&& other) : base(other.x0_size)
    {
        for (size_t i=0 ; i<base::x0_size ; ++i)
            ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(std::forward<T>(other.base::_value(i)));
    }

    reserved_vector& operator=(reserved_vector&& other)
    {
        size_t i = 0;
        if (other.base::x0_size > base::x0_size)
        {
            for (; i<base::x0_size ; ++i)
                base::_value(i) = std::forward<T>(other.base::_value(i));
            for (; i<other.base::x0_size ; ++i)
                ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(std::forward<T>(other.base::_value(i)));
        }
        else if (other.base::x0_size < base::x0_size)
        {
            for (; i<other.base::x0_size ; ++i)
                base::_value(i) = std::forward<T>(other.base::_value(i));
            if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
                for (; i<base::x0_size ; ++i)
                    base::destroy(base::_value(i));
        }
        else
        {
            for (; i<other.base::x0_size ; ++i)
                base::_value(i) = std::forward<T>(other.base::_value(i));
        }
        base::x0_size = other.base::x0_size;
        return *this;
    }

    ~reserved_vector()
    {
        if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
            for (size_t i=0 ; i<base::x0_size ; ++i)
                base::destroy(base::_value(i));
    }

    void push_back(const T& d)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "push_back() called on full rstl::reserved_vector.");
#endif
        ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size)))) T(d);
        ++base::x0_size;
    }

    void push_back(T&& d)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "push_back() called on full rstl::reserved_vector.");
#endif
        ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size)))) T(std::forward<T>(d));
        ++base::x0_size;
    }

    template<class... _Args>
    void emplace_back(_Args&&... args)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "emplace_back() called on full rstl::reserved_vector.");
#endif
        ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size)))) T(std::forward<_Args>(args)...);
        ++base::x0_size;
    }

    void pop_back()
    {
#ifndef NDEBUG
        if (base::x0_size == 0)
            Log.report(logvisor::Fatal, "pop_back() called on empty rstl::reserved_vector.");
#endif
        --base::x0_size;
        base::destroy(base::_value(base::x0_size));
    }

    iterator insert(const_iterator pos, const T& value)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "insert() called on full rstl::reserved_vector.");
#endif
        auto target_it = base::_const_cast_iterator(pos);
        if (pos == base::cend())
        {
            ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size)))) T(value);
        }
        else
        {
            ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size))))
                T(std::forward<T>(base::_value(base::x0_size - 1)));
            for (auto it = base::end() - 1; it != target_it; --it)
                *it = std::forward<T>(*(it - 1));
            *target_it = value;
        }
        ++base::x0_size;
        return target_it;
    }

    iterator insert(const_iterator pos, T&& value)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "insert() called on full rstl::reserved_vector.");
#endif
        auto target_it = base::_const_cast_iterator(pos);
        if (pos == base::cend())
        {
            ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size)))) T(std::forward<T>(value));
        }
        else
        {
            ::new (static_cast<void*>(std::addressof(base::_value(base::x0_size))))
                T(std::forward<T>(base::_value(base::x0_size - 1)));
            for (auto it = base::end() - 1; it != target_it; --it)
                *it = std::forward<T>(*(it - 1));
            *target_it = std::forward<T>(value);
        }
        ++base::x0_size;
        return target_it;
    }

    void resize(size_t size)
    {
#ifndef NDEBUG
        if (size > N)
            Log.report(logvisor::Fatal, "resized() call overflows rstl::reserved_vector.");
#endif
        if (size > base::x0_size)
        {
            for (size_t i = base::x0_size; i < size; ++i)
                ::new (static_cast<void*>(std::addressof(base::_value(i)))) T();
            base::x0_size = size;
        }
        else if (size < base::x0_size)
        {
            if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
                for (size_t i = size; i < base::x0_size; ++i)
                    base::destroy(base::_value(i));
            base::x0_size = size;
        }
    }

    void resize(size_t size, const T& value)
    {
#ifndef NDEBUG
        if (size > N)
            Log.report(logvisor::Fatal, "resized() call overflows rstl::reserved_vector.");
#endif
        if (size > base::x0_size)
        {
            for (size_t i = base::x0_size; i < size; ++i)
                ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(value);
            base::x0_size = size;
        }
        else if (size < base::x0_size)
        {
            if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
                for (size_t i = size; i < base::x0_size; ++i)
                    base::destroy(base::_value(i));
            base::x0_size = size;
        }
    }

    iterator erase(const_iterator pos)
    {
#ifndef NDEBUG
        if (base::x0_size == 0)
            Log.report(logvisor::Fatal, "erase() called on empty rstl::reserved_vector.");
#endif
        for (auto it = base::_const_cast_iterator(pos) + 1; it != base::end(); ++it)
            *(it - 1) = std::forward<T>(*it);
        --base::x0_size;
        base::destroy(base::_value(base::x0_size));
        return base::_const_cast_iterator(pos);
    }

    void clear()
    {
        if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
            for (auto it = base::begin(); it != base::end(); ++it)
                base::destroy(*it);
        base::x0_size = 0;
    }
};

/**
 * @brief Vector backed by statically-allocated array with default-initialized elements
 */
template <class T, size_t N>
class prereserved_vector : public _reserved_vector_base<T, N>
{
    void _init()
    {
        for (auto& i : base::x4_data)
            ::new (static_cast<void*>(std::addressof(i._value))) T();
    }
    void _deinit()
    {
        if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
            for (auto& i : base::x4_data)
                base::destroy(i._value);
    }
public:
    using base = _reserved_vector_base<T, N>;
    using iterator = typename base::iterator;
    using const_iterator = typename base::const_iterator;
    prereserved_vector() : base(1) { _init(); }

    prereserved_vector(const prereserved_vector& other) : base(other.x0_size)
    {
        for (size_t i=0 ; i<N ; ++i)
            ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(other.base::_value(i));
    }

    prereserved_vector& operator=(const prereserved_vector& other)
    {
        for (size_t i=0 ; i<N ; ++i)
            base::_value(i) = other.base::_value(i);
        base::x0_size = other.base::x0_size;
        return *this;
    }

    prereserved_vector(prereserved_vector&& other) : base(other.x0_size)
    {
        for (size_t i=0 ; i<N ; ++i)
            ::new (static_cast<void*>(std::addressof(base::_value(i)))) T(std::forward<T>(other.base::_value(i)));
    }

    prereserved_vector& operator=(prereserved_vector&& other)
    {
        for (size_t i=0 ; i<N ; ++i)
            base::_value(i) = std::forward<T>(other.base::_value(i));
        base::x0_size = other.base::x0_size;
        return *this;
    }

    ~prereserved_vector() { _deinit(); }

    void set_size(size_t n)
    {
        if (n <= N)
            base::x0_size = n;
    }

    void set_data(const T* data) { memmove(base::x4_data, data, sizeof(T) * base::x0_size); }

    void push_back(const T& d)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "push_back() called on full rstl::prereserved_vector.");
#endif
        base::_value(base::x0_size) = d;
        ++base::x0_size;
    }

    void push_back(T&& d)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "push_back() called on full rstl::prereserved_vector.");
#endif
        base::_value(base::x0_size) = std::forward<T>(d);
        ++base::x0_size;
    }

    template<class... _Args>
    void emplace_back(_Args&&... args)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "emplace_back() called on full rstl::prereserved_vector.");
#endif
        base::_value(base::x0_size) = T(std::forward<_Args>(args)...);
        ++base::x0_size;
    }

    void pop_back()
    {
#ifndef NDEBUG
        if (base::x0_size == 0)
            Log.report(logvisor::Fatal, "pop_back() called on empty rstl::prereserved_vector.");
#endif
        --base::x0_size;
    }

    iterator insert(const_iterator pos, const T& value)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "insert() called on full rstl::reserved_vector.");
#endif
        auto target_it = base::_const_cast_iterator(pos);
        if (pos == base::cend())
        {
            *target_it = value;
        }
        else
        {
            for (auto it = base::end(); it != target_it; --it)
                *it = std::forward<T>(*(it - 1));
            *target_it = value;
        }
        ++base::x0_size;
        return target_it;
    }

    iterator insert(const_iterator pos, T&& value)
    {
#ifndef NDEBUG
        if (base::x0_size == N)
            Log.report(logvisor::Fatal, "insert() called on full rstl::reserved_vector.");
#endif
        auto target_it = base::_const_cast_iterator(pos);
        if (pos == base::cend())
        {
            *target_it = std::forward<T>(value);
        }
        else
        {
            for (auto it = base::end(); it != target_it; --it)
                *it = std::forward<T>(*(it - 1));
            *target_it = std::forward<T>(value);
        }
        ++base::x0_size;
        return target_it;
    }

    void resize(size_t size)
    {
#ifndef NDEBUG
        if (size > N)
            Log.report(logvisor::Fatal, "resized() call overflows rstl::prereserved_vector.");
#endif
        base::x0_size = size;
    }

    void resize(size_t size, const T& value)
    {
#ifndef NDEBUG
        if (size > N)
            Log.report(logvisor::Fatal, "resized() call overflows rstl::prereserved_vector.");
#endif
        if (size > base::x0_size)
        {
            for (size_t i = base::x0_size; i < size; ++i)
                base::_value(i) = T(value);
            base::x0_size = size;
        }
        else if (size < base::x0_size)
        {
            base::x0_size = size;
        }
    }

    iterator erase(const_iterator pos)
    {
#ifndef NDEBUG
        if (base::x0_size == 0)
            Log.report(logvisor::Fatal, "erase() called on empty rstl::prereserved_vector.");
#endif
        for (auto it = base::_const_cast_iterator(pos) + 1; it != base::end(); ++it)
            *(it - 1) = std::forward<T>(*it);
        --base::x0_size;
        return base::_const_cast_iterator(pos);
    }

    void clear() { base::x0_size = 0; }
};

template<class ForwardIt, class T>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value)
{
    first = std::lower_bound(first, last, value);
    return (!(first == last) && !(value < *first)) ? first : last;
}

template<class ForwardIt, class T, class GetKey>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, GetKey getkey)
{
    auto comp = [&](const auto& left, const T& right) { return getkey(left) < right; };
    first = std::lower_bound(first, last, value, comp);
    return (!(first == last) && !(value < getkey(*first))) ? first : last;
}

#if 0
template <typename _CharTp>
class basic_string
{
    struct COWData
    {
        uint32_t x0_capacity;
        uint32_t x4_refCount;
        _CharTp x8_data[];
    };

    const _CharTp* x0_ptr;
    COWData* x4_cow;
    uint32_t x8_size;

    void internal_allocate(int size)
    {
        x4_cow = reinterpret_cast<COWData*>(new uint8_t[size * sizeof(_CharTp) + 8]);
        x0_ptr = x4_cow->x8_data;
        x4_cow->x0_capacity = uint32_t(size);
        x4_cow->x4_refCount = 1;
    }

    static const _CharTp _EmptyString;

public:
    struct literal_t {};

    basic_string(literal_t, const _CharTp* data)
    {
        x0_ptr = data;
        x4_cow = nullptr;

        const _CharTp* it = data;
        while (*it)
            ++it;

        x8_size = uint32_t((it - data) / sizeof(_CharTp));
    }

    basic_string(const basic_string& str)
    {
        x0_ptr = str.x0_ptr;
        x4_cow = str.x4_cow;
        x8_size = str.x8_size;
        if (x4_cow)
            ++x4_cow->x4_refCount;
    }

    basic_string(const _CharTp* data, int size)
    {
        if (size <= 0 && !data)
        {
            x0_ptr = &_EmptyString;
            x4_cow = nullptr;
            x8_size = 0;
            return;
        }

        const _CharTp* it = data;
        uint32_t len = 0;
        while (*it)
        {
            if (size != -1 && len >= size)
                break;
            ++it;
            ++len;
        }

        internal_allocate(len + 1);
        x8_size = len;
        for (int i = 0; i < len; ++i)
            x4_cow->x8_data[i] = data[i];
        x4_cow->x8_data[len] = 0;
    }

    ~basic_string()
    {
        if (x4_cow && --x4_cow->x4_refCount == 0)
            delete[] x4_cow;
    }
};

template <>
const char basic_string<char>::_EmptyString = 0;
template <>
const wchar_t basic_string<wchar_t>::_EmptyString = 0;

typedef basic_string<wchar_t> wstring;
typedef basic_string<char> string;

wstring wstring_l(const wchar_t* data)
{
    return wstring(wstring::literal_t(), data);
}

string string_l(const char* data)
{
    return string(string::literal_t(), data);
}
#endif

}

#endif // __RSTL_HPP__
