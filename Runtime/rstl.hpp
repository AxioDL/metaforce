#ifndef __RSTL_HPP__
#define __RSTL_HPP__

#include <vector>
#include <algorithm>
#include <stdlib.h>
#include "optional.hpp"

namespace rstl
{

template <typename T>
using optional_object = std::experimental::optional<T>;

/**
 * @brief Vector reserved on construction
 */
template <class T, size_t N>
class reserved_vector : public std::vector<T>
{
public:
    reserved_vector() { this->reserve(N); }
    reserved_vector(size_t n, const T& val) : std::vector<T>(n, val) {}
};

template <class T, size_t N>
class prereserved_vector
{
    size_t x0_size = 1;
    T x4_data[N];

public:
    void set_size(size_t n)
    {
        if (n <= N)
            x0_size = n;
    }

    void set_data(const T* data) { memmove(x4_data, data, sizeof(T) * x0_size); }

    size_t size() const { return x0_size; }

    T& back() const { return x4_data[(x0_size == 0) ? 0 : x0_size - 1]; }
    T& front() const { return x4_data[0]; }
    T& operator[](size_t idx) { return x4_data[idx]; }
    const T& operator[](size_t idx) const { return x4_data[idx]; }
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
