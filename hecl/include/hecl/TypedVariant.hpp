#pragma once

#include <cassert>
#include <type_traits>
#include <variant>

#include <athena/DNA.hpp>

/*
 * The TypedVariant system is a type-safe union implementation capable of selecting
 * a participating field type based on an enumeration constant. As an extension of
 * std::variant, the usage pattern is similar to that of std::visit. A key difference
 * with TypedVariant is the monostate is implicitly supplied as the first argument.
 * The monostate will cause the visit implementation to function as a no-op, returning
 * a specified default value.
 */

/* Example user code:

enum class ChunkType {
  Invalid = 0,
  TYP1 = SBIG('TYP1'),
  TYP2 = SBIG('TYP2'),
  TYP3 = SBIG('TYP3'),
};

struct TYP1 : TypedRecord<ChunkType::TYP1> {
  static void PrintMe() { std::cout << "TYP1" << std::endl; }
};
struct TYP2 : TypedRecord<ChunkType::TYP2> {
  static void PrintMe() { std::cout << "TYP2" << std::endl; }
};
struct TYP3 : TypedRecord<ChunkType::TYP3> {
  static void PrintMe() { std::cout << "TYP3" << std::endl; }
};

using ChunkVariant = TypedVariant<TYP1, TYP2, TYP3>;

int main(int argc, char** argv) {
  volatile ChunkType tp = ChunkType::TYP2;
  auto var = ChunkVariant::Build(tp);
  var.visit([](auto& arg) { arg.PrintMe(); });
  return 0;
}

 */

namespace hecl {

template<auto _Type>
struct TypedRecord {
  using TypedType = std::integral_constant<decltype(_Type), _Type>;
  static constexpr auto variant_type() { return _Type; }
  static constexpr bool is_monostate() { return false; }
};

template<typename _Enum>
struct TypedMonostate : TypedRecord<_Enum(0)> {
  static constexpr bool is_monostate() { return true; }
  bool operator==(const TypedMonostate& other) const { return true; }
  bool operator!=(const TypedMonostate& other) const { return false; }
};

template<typename... _Types>
class _TypedVariant : public std::variant<_Types...> {
public:
  using base = std::variant<_Types...>;
  using EnumType = typename std::variant_alternative_t<0, std::variant<_Types...>>::TypedType::value_type;

private:
  template<typename _Type, typename _Variant>
  struct _Match;

  template<typename _Type, typename _First, typename... _Rest>
  struct _Match<_Type, std::variant<_First, _Rest...>>
      : _Match<_Type, std::variant<_Rest...>> {};

  template<typename _First, typename... _Rest>
  struct _Match<typename _First::TypedType, std::variant<_First, _Rest...>>
  { using type = _First; };

public:
  template<auto _Type>
  using Match = typename _Match<std::integral_constant<decltype(_Type), _Type>, std::variant<_Types...>>::type;

private:
  template<typename _First, typename... _Rest>
  struct _Builder {
    template<typename... _Args>
    static constexpr _TypedVariant _Build(EnumType tp, _Args&&... args) {
      //static_assert(std::is_constructible_v<_First, _Args...>,
      //              "All variant types must be constructible with the same parameters.");
      if (_First::TypedType::value == tp) {
        if constexpr (std::is_constructible_v<_First, _Args...>) {
          return {_First(std::forward<_Args>(args)...)};
        } else {
          assert(false && "Variant not constructible with supplied args.");
          return {};
        }
      }
      else if constexpr (sizeof...(_Rest) > 0)
        return _Builder<_Rest...>::template _Build<_Args...>(tp, std::forward<_Args>(args)...);
      return {};
    }
    template<typename... _Args>
    static constexpr _TypedVariant _BuildSkip(EnumType tp, _Args&&... args) {
      /* This prevents selecting the monostate explicitly (so constructor arguments aren't passed) */
      if constexpr (sizeof...(_Rest) > 0)
        return _Builder<_Rest...>::template _Build<_Args...>(tp, std::forward<_Args>(args)...);
      return {};
    }
  };

public:
  template<typename... _Args>
  static constexpr _TypedVariant Build(EnumType tp, _Args&&... args) {
    return _TypedVariant::_Builder<_Types...>::template _BuildSkip<_Args...>(tp, std::forward<_Args>(args)...);
  }

  template<typename _Return, typename _Visitor>
  constexpr auto visit(_Visitor&& visitor, _Return&& def) {
    return std::visit([&](auto& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!T::is_monostate())
        return visitor(arg);
      return def;
    }, static_cast<base&>(*this));
  }

  template<typename _Visitor>
  constexpr void visit(_Visitor&& visitor) {
    std::visit([&](auto& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!T::is_monostate())
        visitor(arg);
    }, static_cast<base&>(*this));
  }

  template<typename _Return, typename _Visitor>
  constexpr auto visit(_Visitor&& visitor, _Return&& def) const {
    return std::visit([&](const auto& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!T::is_monostate())
        return visitor(arg);
      return def;
    }, static_cast<const base&>(*this));
  }

  template<typename _Visitor>
  constexpr void visit(_Visitor&& visitor) const {
    std::visit([&](const auto& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (!T::is_monostate())
        visitor(arg);
    }, static_cast<const base&>(*this));
  }

  template<typename T>
  constexpr T& get() { return std::get<T>(*this); }

  template<typename T>
  constexpr const T& get() const { return std::get<T>(*this); }

  template<typename T>
  constexpr std::add_pointer_t<T> get_if() noexcept { return std::get_if<T>(this); }

  template<typename T>
  constexpr std::add_pointer_t<const T> get_if() const noexcept { return std::get_if<T>(this); }

  template<typename T>
  constexpr bool holds_alternative() const noexcept { return std::holds_alternative<T>(*this); }

  constexpr EnumType variant_type() const {
    return std::visit([](const auto& arg) {
      return arg.variant_type();
    }, static_cast<const base&>(*this));
  }

  constexpr explicit operator bool() const noexcept {
    return !std::holds_alternative<typename std::variant_alternative_t<0, std::variant<_Types...>>>(*this);
  }
};

template<typename... _Types>
using TypedVariant = _TypedVariant<TypedMonostate<typename std::variant_alternative_t
    <0, std::variant<_Types...>>::TypedType::value_type>, _Types...>;

/* DNA support below here */

template<auto _Type>
struct TypedRecordBigDNA : BigDNA, TypedRecord<_Type> {};

template<typename... _Types>
struct TypedVariantBigDNA : BigDNA, TypedVariant<_Types...> {
  AT_DECL_EXPLICIT_DNA_YAML
  template<typename... _Args>
  static constexpr TypedVariantBigDNA Build(typename TypedVariant<_Types...>::EnumType tp, _Args&&... args) {
    return TypedVariantBigDNA(TypedVariant<_Types...>::Build(tp, std::forward<_Args>(args)...));
  }
  TypedVariantBigDNA() = default;
private:
  TypedVariantBigDNA(TypedVariant<_Types...> var) : TypedVariant<_Types...>(std::move(var)) {}
};

#define AT_SPECIALIZE_TYPED_VARIANT_BIGDNA(...)                                                                        \
  template <>                                                                                                          \
  template <>                                                                                                          \
  inline void hecl::TypedVariantBigDNA<__VA_ARGS__>::Enumerate<athena::io::DNA<athena::Endian::Big>::Read>(            \
      typename Read::StreamT & r) {                                                                                    \
    EnumType variant_type = {};                                                                                        \
    Do<athena::io::DNA<athena::Endian::Big>::Read>(athena::io::PropId("variant_type"sv), variant_type, r);             \
    static_cast<TypedVariant<__VA_ARGS__>&>(*this) = Build(variant_type);                                              \
    visit([&](auto& var) { var.read(r); });                                                                            \
  }                                                                                                                    \
                                                                                                                       \
  template <>                                                                                                          \
  template <>                                                                                                          \
  inline void hecl::TypedVariantBigDNA<__VA_ARGS__>::Enumerate<athena::io::DNA<athena::Endian::Big>::Write>(           \
      typename Write::StreamT & w) {                                                                                   \
    visit([&](auto& var) {                                                                                             \
      using T = std::decay_t<decltype(var)>;                                                                           \
      EnumType variant_type = T::variant_type();                                                                       \
      Do<athena::io::DNA<athena::Endian::Big>::Write>(athena::io::PropId("variant_type"sv), variant_type, w);          \
      var.write(w);                                                                                                    \
    });                                                                                                                \
  }                                                                                                                    \
                                                                                                                       \
  template <>                                                                                                          \
  template <>                                                                                                          \
  inline void hecl::TypedVariantBigDNA<__VA_ARGS__>::Enumerate<athena::io::DNA<athena::Endian::Big>::BinarySize>(      \
      typename BinarySize::StreamT & sz) {                                                                             \
    visit([&](auto& var) {                                                                                             \
      using T = std::decay_t<decltype(var)>;                                                                           \
      EnumType variant_type = T::variant_type();                                                                       \
      Do<athena::io::DNA<athena::Endian::Big>::BinarySize>(athena::io::PropId("variant_type"sv), variant_type, sz);    \
      var.binarySize(sz);                                                                                              \
    });                                                                                                                \
  }                                                                                                                    \
  template <>                                                                                                          \
  inline std::string_view hecl::TypedVariantBigDNA<__VA_ARGS__>::DNAType() {                                           \
    return "hecl::TypedVariantBigDNA<" #__VA_ARGS__ ">"sv;                                                             \
  }

#define AT_SPECIALIZE_TYPED_VARIANT_BIGDNA_YAML(...)                                                                   \
  AT_SPECIALIZE_TYPED_VARIANT_BIGDNA(__VA_ARGS__)                                                                      \
  template <>                                                                                                          \
  template <>                                                                                                          \
  inline void hecl::TypedVariantBigDNA<__VA_ARGS__>::Enumerate<athena::io::DNA<athena::Endian::Big>::ReadYaml>(        \
      typename ReadYaml::StreamT & r) {                                                                                \
    EnumType variant_type = {};                                                                                        \
    Do<athena::io::DNA<athena::Endian::Big>::ReadYaml>(athena::io::PropId("variant_type"sv), variant_type, r);         \
    static_cast<TypedVariant<__VA_ARGS__>&>(*this) = Build(variant_type);                                              \
    visit([&](auto& var) { var.read(r); });                                                                            \
  }                                                                                                                    \
                                                                                                                       \
  template <>                                                                                                          \
  template <>                                                                                                          \
  inline void hecl::TypedVariantBigDNA<__VA_ARGS__>::Enumerate<athena::io::DNA<athena::Endian::Big>::WriteYaml>(       \
      typename WriteYaml::StreamT & w) {                                                                               \
    visit([&](auto& var) {                                                                                             \
      using T = std::decay_t<decltype(var)>;                                                                           \
      EnumType variant_type = T::variant_type();                                                                       \
      Do<athena::io::DNA<athena::Endian::Big>::WriteYaml>(athena::io::PropId("variant_type"sv), variant_type, w);      \
      var.write(w);                                                                                                    \
    });                                                                                                                \
  }

}
