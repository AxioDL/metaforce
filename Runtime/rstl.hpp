#pragma once

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <vector>
#include <iterator>

#ifndef NDEBUG
#include <logvisor/logvisor.hpp>
#endif

namespace rstl {

#ifndef NDEBUG
static logvisor::Module Log("rstl");
#endif

/**
 * @brief Base vector backed by statically-allocated array
 */
template <class T>
class _reserved_vector_base {
public:
  class const_iterator {
    friend class _reserved_vector_base;

  protected:
    const T* m_val;

  public:
    const_iterator() : m_val(nullptr) {}
    explicit const_iterator(const T* val) : m_val(val) {}
    using value_type = T;
    using element_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
#ifdef __cpp_lib_concepts
    using iterator_category = std::contiguous_iterator_tag;
#else
    using iterator_category = std::random_access_iterator_tag;
#endif

    const T& operator*() const { return *m_val; }
    const T* operator->() const { return m_val; }
    const_iterator& operator++() {
      ++m_val;
      return *this;
    }
    const_iterator& operator--() {
      --m_val;
      return *this;
    }
    const_iterator operator++(int) {
      auto ret = *this;
      ++m_val;
      return ret;
    }
    const_iterator operator--(int) {
      auto ret = *this;
      --m_val;
      return ret;
    }
    bool operator!=(const const_iterator& other) const { return m_val != other.m_val; }
    bool operator==(const const_iterator& other) const { return m_val == other.m_val; }
    const_iterator operator+(std::ptrdiff_t i) const { return const_iterator(m_val + i); }
    const_iterator operator-(std::ptrdiff_t i) const { return const_iterator(m_val - i); }
    const_iterator& operator+=(std::ptrdiff_t i) {
      m_val += i;
      return *this;
    }
    const_iterator& operator-=(std::ptrdiff_t i) {
      m_val -= i;
      return *this;
    }
    std::ptrdiff_t operator-(const const_iterator& it) const { return m_val - it.m_val; }
    bool operator>(const const_iterator& it) const { return m_val > it.m_val; }
    bool operator<(const const_iterator& it) const { return m_val < it.m_val; }
    bool operator>=(const const_iterator& it) const { return m_val >= it.m_val; }
    bool operator<=(const const_iterator& it) const { return m_val <= it.m_val; }
    const T& operator[](std::ptrdiff_t i) const { return m_val[i]; }
  };

  class iterator : public const_iterator {
    friend class _reserved_vector_base;

  public:
    iterator() : const_iterator() {}
    explicit iterator(T* val) : const_iterator(val) {}
    T& operator*() const { return *const_cast<T*>(const_iterator::m_val); }
    T* operator->() const { return const_cast<T*>(const_iterator::m_val); }
    iterator& operator++() {
      ++const_iterator::m_val;
      return *this;
    }
    iterator& operator--() {
      --const_iterator::m_val;
      return *this;
    }
    iterator operator++(int) {
      auto ret = *this;
      ++const_iterator::m_val;
      return ret;
    }
    iterator operator--(int) {
      auto ret = *this;
      --const_iterator::m_val;
      return ret;
    }
    iterator operator+(std::ptrdiff_t i) const { return iterator(const_cast<T*>(const_iterator::m_val) + i); }
    iterator operator-(std::ptrdiff_t i) const { return iterator(const_cast<T*>(const_iterator::m_val) - i); }
    iterator& operator+=(std::ptrdiff_t i) {
      const_iterator::m_val += i;
      return *this;
    }
    iterator& operator-=(std::ptrdiff_t i) {
      const_iterator::m_val -= i;
      return *this;
    }
    std::ptrdiff_t operator-(const iterator& it) const { return const_iterator::m_val - it.m_val; }
    bool operator>(const iterator& it) const { return const_iterator::m_val > it.m_val; }
    bool operator<(const iterator& it) const { return const_iterator::m_val < it.m_val; }
    bool operator>=(const iterator& it) const { return const_iterator::m_val >= it.m_val; }
    bool operator<=(const iterator& it) const { return const_iterator::m_val <= it.m_val; }
    T& operator[](std::ptrdiff_t i) const { return const_cast<T*>(const_iterator::m_val)[i]; }
  };

  using reverse_iterator = decltype(std::make_reverse_iterator(iterator{}));
  using const_reverse_iterator = decltype(std::make_reverse_iterator(const_iterator{}));

protected:
  static iterator _const_cast_iterator(const const_iterator& it) { return iterator(const_cast<T*>(it.m_val)); }
};

/**
 * @brief Vector backed by statically-allocated array with uninitialized storage
 */
template <class T, size_t N>
class reserved_vector : public _reserved_vector_base<T> {
  using base = _reserved_vector_base<T>;

public:
  using value_type = T;

  using pointer = value_type*;
  using const_pointer = const value_type*;

  using reference = value_type&;
  using const_reference = const value_type&;

  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  using iterator = typename base::iterator;
  using const_iterator = typename base::const_iterator;
  using reverse_iterator = typename base::reverse_iterator;
  using const_reverse_iterator = typename base::const_reverse_iterator;

private:
  union alignas(T) storage_t {
    struct {
    } _dummy;
    T _value;
    storage_t() : _dummy() {}
    ~storage_t() {}
  };
  size_t x0_size;
  storage_t x4_data[N];
  T& _value(std::ptrdiff_t idx) { return x4_data[idx]._value; }
  const T& _value(std::ptrdiff_t idx) const { return x4_data[idx]._value; }
  template <typename Tp>
  static void
  destroy(Tp& t, std::enable_if_t<std::is_destructible_v<Tp> && !std::is_trivially_destructible_v<Tp>>* = nullptr) {
    t.Tp::~Tp();
  }
  template <typename Tp>
  static void
  destroy(Tp& t, std::enable_if_t<!std::is_destructible_v<Tp> || std::is_trivially_destructible_v<Tp>>* = nullptr) {}

public:
  constexpr reserved_vector() noexcept(std::is_nothrow_constructible_v<T>) : x0_size(0) {}

  template <size_t LN>
  constexpr reserved_vector(const T (&l)[LN]) noexcept(std::is_nothrow_copy_constructible_v<T>) : x0_size(LN) {
    static_assert(LN <= N, "initializer array too large for reserved_vector");
    for (size_t i = 0; i < LN; ++i) {
      ::new (static_cast<void*>(std::addressof(_value(i)))) T(l[i]);
    }
  }

  reserved_vector(const reserved_vector& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
  : x0_size(other.x0_size) {
    for (size_t i = 0; i < x0_size; ++i) {
      ::new (static_cast<void*>(std::addressof(_value(i)))) T(other._value(i));
    }
  }

  reserved_vector& operator=(const reserved_vector& other) noexcept(std::is_nothrow_copy_assignable_v<T>) {
    size_t i = 0;
    if (other.x0_size > x0_size) {
      for (; i < x0_size; ++i) {
        _value(i) = other._value(i);
      }
      for (; i < other.x0_size; ++i) {
        ::new (static_cast<void*>(std::addressof(_value(i)))) T(other._value(i));
      }
    } else if (other.x0_size < x0_size) {
      for (; i < other.x0_size; ++i) {
        _value(i) = other._value(i);
      }

      if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
        for (; i < x0_size; ++i) {
          destroy(_value(i));
        }
      }
    } else {
      for (; i < other.x0_size; ++i) {
        _value(i) = other._value(i);
      }
    }

    x0_size = other.x0_size;
    return *this;
  }

  reserved_vector(reserved_vector&& other) noexcept(std::is_nothrow_move_constructible_v<T>) : x0_size(other.x0_size) {
    for (size_t i = 0; i < x0_size; ++i) {
      ::new (static_cast<void*>(std::addressof(_value(i)))) T(std::forward<T>(other._value(i)));
    }
  }

  reserved_vector& operator=(reserved_vector&& other) noexcept(
      std::is_nothrow_move_assignable_v<T>&& std::is_nothrow_move_constructible_v<T>) {
    size_t i = 0;
    if (other.x0_size > x0_size) {
      for (; i < x0_size; ++i) {
        _value(i) = std::forward<T>(other._value(i));
      }
      for (; i < other.x0_size; ++i) {
        ::new (static_cast<void*>(std::addressof(_value(i)))) T(std::forward<T>(other._value(i)));
      }
    } else if (other.x0_size < x0_size) {
      for (; i < other.x0_size; ++i) {
        _value(i) = std::forward<T>(other._value(i));
      }

      if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
        for (; i < x0_size; ++i) {
          destroy(_value(i));
        }
      }
    } else {
      for (; i < other.x0_size; ++i) {
        _value(i) = std::forward<T>(other._value(i));
      }
    }

    x0_size = other.x0_size;
    return *this;
  }

  ~reserved_vector() {
    if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
      for (size_t i = 0; i < x0_size; ++i) {
        destroy(_value(i));
      }
    }
  }

  void push_back(const T& d) {
#ifndef NDEBUG
    if (x0_size == N) {
      Log.report(logvisor::Fatal, FMT_STRING("push_back() called on full rstl::reserved_vector."));
    }
#endif

    ::new (static_cast<void*>(std::addressof(_value(x0_size)))) T(d);
    ++x0_size;
  }

  void push_back(T&& d) {
#ifndef NDEBUG
    if (x0_size == N) {
      Log.report(logvisor::Fatal, FMT_STRING("push_back() called on full rstl::reserved_vector."));
    }
#endif

    ::new (static_cast<void*>(std::addressof(_value(x0_size)))) T(std::forward<T>(d));
    ++x0_size;
  }

  template <class... _Args>
  T& emplace_back(_Args&&... args) {
#ifndef NDEBUG
    if (x0_size == N) {
      Log.report(logvisor::Fatal, FMT_STRING("emplace_back() called on full rstl::reserved_vector."));
    }
#endif

    T& element = _value(x0_size);
    ::new (static_cast<void*>(std::addressof(element))) T(std::forward<_Args>(args)...);

    ++x0_size;
    return element;
  }

  void pop_back() {
#ifndef NDEBUG
    if (x0_size == 0) {
      Log.report(logvisor::Fatal, FMT_STRING("pop_back() called on empty rstl::reserved_vector."));
    }
#endif

    --x0_size;
    destroy(_value(x0_size));
  }

  iterator insert(const_iterator pos, const T& value) {
#ifndef NDEBUG
    if (x0_size == N) {
      Log.report(logvisor::Fatal, FMT_STRING("insert() called on full rstl::reserved_vector."));
    }
#endif

    auto target_it = base::_const_cast_iterator(pos);
    if (pos == cend()) {
      ::new (static_cast<void*>(std::addressof(_value(x0_size)))) T(value);
    } else {
      ::new (static_cast<void*>(std::addressof(_value(x0_size)))) T(std::forward<T>(_value(x0_size - 1)));
      for (auto it = end() - 1; it != target_it; --it) {
        *it = std::forward<T>(*(it - 1));
      }
      *target_it = value;
    }
    ++x0_size;
    return target_it;
  }

  iterator insert(const_iterator pos, T&& value) {
#ifndef NDEBUG
    if (x0_size == N)
      Log.report(logvisor::Fatal, FMT_STRING("insert() called on full rstl::reserved_vector."));
#endif
    auto target_it = base::_const_cast_iterator(pos);
    if (pos == cend()) {
      ::new (static_cast<void*>(std::addressof(_value(x0_size)))) T(std::forward<T>(value));
    } else {
      ::new (static_cast<void*>(std::addressof(_value(x0_size)))) T(std::forward<T>(_value(x0_size - 1)));
      for (auto it = end() - 1; it != target_it; --it) {
        *it = std::forward<T>(*(it - 1));
      }
      *target_it = std::forward<T>(value);
    }
    ++x0_size;
    return target_it;
  }

  void resize(size_t size) {
#ifndef NDEBUG
    if (size > N) {
      Log.report(logvisor::Fatal, FMT_STRING("resize() call overflows rstl::reserved_vector."));
    }
#endif

    if (size > x0_size) {
      for (size_t i = x0_size; i < size; ++i) {
        ::new (static_cast<void*>(std::addressof(_value(i)))) T();
      }
      x0_size = size;
    } else if (size < x0_size) {
      if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
        for (size_t i = size; i < x0_size; ++i) {
          destroy(_value(i));
        }
      }
      x0_size = size;
    }
  }

  void resize(size_t size, const T& value) {
#ifndef NDEBUG
    if (size > N) {
      Log.report(logvisor::Fatal, FMT_STRING("resize() call overflows rstl::reserved_vector."));
    }
#endif

    if (size > x0_size) {
      for (size_t i = x0_size; i < size; ++i) {
        ::new (static_cast<void*>(std::addressof(_value(i)))) T(value);
      }
      x0_size = size;
    } else if (size < x0_size) {
      if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
        for (size_t i = size; i < x0_size; ++i) {
          destroy(_value(i));
        }
      }
      x0_size = size;
    }
  }

  iterator erase(const_iterator pos) {
#ifndef NDEBUG
    if (x0_size == 0) {
      Log.report(logvisor::Fatal, FMT_STRING("erase() called on empty rstl::reserved_vector."));
    }
#endif

    for (auto it = base::_const_cast_iterator(pos) + 1; it != end(); ++it) {
      *(it - 1) = std::forward<T>(*it);
    }
    --x0_size;
    destroy(_value(x0_size));
    return base::_const_cast_iterator(pos);
  }

  void pop_front() {
    if (x0_size != 0) {
      erase(begin());
    }
  }

  void clear() {
    if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
      for (auto it = begin(); it != end(); ++it) {
        destroy(*it);
      }
    }
    x0_size = 0;
  }

  [[nodiscard]] size_t size() const noexcept { return x0_size; }
  [[nodiscard]] bool empty() const noexcept { return x0_size == 0; }
  [[nodiscard]] constexpr size_t capacity() const noexcept { return N; }
  [[nodiscard]] const T* data() const noexcept { return std::addressof(_value(0)); }
  [[nodiscard]] T* data() noexcept { return std::addressof(_value(0)); }

  [[nodiscard]] T& back() { return _value(x0_size - 1); }
  [[nodiscard]] T& front() { return _value(0); }
  [[nodiscard]] const T& back() const { return _value(x0_size - 1); }
  [[nodiscard]] const T& front() const { return _value(0); }

  [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(std::addressof(_value(0))); }
  [[nodiscard]] const_iterator end() const noexcept { return const_iterator(std::addressof(_value(x0_size))); }
  [[nodiscard]] iterator begin() noexcept { return iterator(std::addressof(_value(0))); }
  [[nodiscard]] iterator end() noexcept { return iterator(std::addressof(_value(x0_size))); }
  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
  [[nodiscard]] const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }
  [[nodiscard]] reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
  [[nodiscard]] reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  [[nodiscard]] const_reverse_iterator crend() const noexcept { return rend(); }

  [[nodiscard]] T& operator[](size_t idx) {
#ifndef NDEBUG
    if (idx >= x0_size) {
      Log.report(logvisor::Fatal, FMT_STRING("out of bounds access on reserved_vector."));
    }
#endif
    return _value(idx);
  }
  [[nodiscard]] const T& operator[](size_t idx) const {
#ifndef NDEBUG
    if (idx >= x0_size) {
      Log.report(logvisor::Fatal, FMT_STRING("out of bounds access on reserved_vector."));
    }
#endif
    return _value(idx);
  }
};

/**
 * @brief Vector-style view backed by externally-allocated storage
 */
template <class T>
class prereserved_vector : public _reserved_vector_base<T> {
  size_t x0_size;
  T* x4_data;
  T& _value(std::ptrdiff_t idx) { return x4_data[idx]; }
  const T& _value(std::ptrdiff_t idx) const { return x4_data[idx]; }

public:
  using base = _reserved_vector_base<T>;
  using iterator = typename base::iterator;
  using const_iterator = typename base::const_iterator;
  using reverse_iterator = typename base::reverse_iterator;
  using const_reverse_iterator = typename base::const_reverse_iterator;
  prereserved_vector() : x0_size(0), x4_data(nullptr) {}
  prereserved_vector(size_t size, T* data) : x0_size(size), x4_data(data) {}

  void set_size(size_t n) { x0_size = n; }
  void set_data(T* data) { x4_data = data; }

  [[nodiscard]] size_t size() const noexcept { return x0_size; }
  [[nodiscard]] bool empty() const noexcept { return x0_size == 0; }
  [[nodiscard]] const T* data() const noexcept { return x4_data; }
  [[nodiscard]] T* data() noexcept { return x4_data; }

  [[nodiscard]] T& back() { return _value(x0_size - 1); }
  [[nodiscard]] T& front() { return _value(0); }
  [[nodiscard]] const T& back() const { return _value(x0_size - 1); }
  [[nodiscard]] const T& front() const { return _value(0); }

  [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(std::addressof(_value(0))); }
  [[nodiscard]] const_iterator end() const noexcept { return const_iterator(std::addressof(_value(x0_size))); }
  [[nodiscard]] iterator begin() noexcept { return iterator(std::addressof(_value(0))); }
  [[nodiscard]] iterator end() noexcept { return iterator(std::addressof(_value(x0_size))); }
  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
  [[nodiscard]] const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }
  [[nodiscard]] reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
  [[nodiscard]] reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  [[nodiscard]] const_reverse_iterator crend() const noexcept { return rend(); }

  [[nodiscard]] T& operator[](size_t idx) { return _value(idx); }
  [[nodiscard]] const T& operator[](size_t idx) const { return _value(idx); }
};

template <class ForwardIt, class T>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value) {
  first = std::lower_bound(first, last, value);
  return (!(first == last) && !(value < *first)) ? first : last;
}

template <class ForwardIt, class T, class GetKey>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, GetKey getkey) {
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

} // namespace rstl
