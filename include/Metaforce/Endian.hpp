#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

template < typename T >
  requires(sizeof(T) == sizeof(uint16_t) && std::is_arithmetic_v< T >)
inline constexpr T bswap(T val) noexcept {
  auto bits = std::bit_cast< uint16_t >(val);
#if defined(__GNUC__) || defined(__clang__)
  bits = __builtin_bswap16(bits);
#elif defined(_WIN32)
  bits = _byteswap_ushort(bits);
#else
  bits = static_cast< uint16_t >((bits << 8) | (bits >> 8));
#endif
  return std::bit_cast< T >(bits);
}

template < typename T >
  requires(sizeof(T) == sizeof(uint32_t) && std::is_arithmetic_v< T >)
inline constexpr T bswap(T val) noexcept {
  auto bits = std::bit_cast< uint32_t >(val);
#if defined(__GNUC__) || defined(__clang__)
  bits = __builtin_bswap32(bits);
#elif defined(_WIN32)
  bits = _byteswap_ulong(bits);
#else
  bits = ((bits & 0x000000ffU) << 24) | ((bits & 0x0000ff00U) << 8) | ((bits & 0x00ff0000U) >> 8) |
         ((bits & 0xff000000U) >> 24);
#endif
  return std::bit_cast< T >(bits);
}

template < typename T >
  requires(sizeof(T) == sizeof(uint64_t) && std::is_arithmetic_v< T >)
inline constexpr T bswap(T val) noexcept {
  auto bits = std::bit_cast< uint64_t >(val);
#if defined(__GNUC__) || defined(__clang__)
  bits = __builtin_bswap64(bits);
#elif defined(_WIN32)
  bits = _byteswap_uint64(bits);
#else
  bits = ((bits & 0x00000000000000ffULL) << 56) | ((bits & 0x000000000000ff00ULL) << 40) |
         ((bits & 0x0000000000ff0000ULL) << 24) | ((bits & 0x00000000ff000000ULL) << 8) |
         ((bits & 0x000000ff00000000ULL) >> 8) | ((bits & 0x0000ff0000000000ULL) >> 24) |
         ((bits & 0x00ff000000000000ULL) >> 40) | ((bits & 0xff00000000000000ULL) >> 56);
#endif
  return std::bit_cast< T >(bits);
}

template < typename T >
  requires(std::is_trivially_copyable_v< T >)
inline T unaligned_load(const void* ptr) noexcept {
  T value;
  std::memcpy(&value, ptr, sizeof(value));
  return value;
}

template < typename T >
  requires(std::is_integral_v< T > && !std::is_same_v< T, bool >)
inline T read_bits(const void* ptr, const std::endian e = std::endian::big) noexcept {
  using U = std::make_unsigned_t< T >;
  U val = unaligned_load< U >(ptr);
  if constexpr (sizeof(U) > 1) {
    if (e != std::endian::native) {
      val = bswap(val);
    }
  }
  return std::bit_cast< T >(val);
}

template < size_t N >
struct uint_of_size;
template <>
struct uint_of_size< 1 > {
  using type = uint8_t;
};
template <>
struct uint_of_size< 2 > {
  using type = uint16_t;
};
template <>
struct uint_of_size< 4 > {
  using type = uint32_t;
};
template <>
struct uint_of_size< 8 > {
  using type = uint64_t;
};
template < size_t N >
using uint_of_size_t = uint_of_size< N >::type;

template < typename T >
  requires(std::is_floating_point_v< T > && requires { typename uint_of_size_t< sizeof(T) >; })
inline T read_bits(const void* ptr, const std::endian e = std::endian::big) noexcept {
  using U = uint_of_size_t< sizeof(T) >;
  return std::bit_cast< T >(read_bits< U >(ptr, e));
}
