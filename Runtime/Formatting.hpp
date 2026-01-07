#pragma once

#include "Runtime/RetroTypes.hpp"

#include <fmt/format.h> // IWYU pragma: export
#include <fmt/xchar.h>

#include <zeus/CMatrix3f.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

#define FMT_CUSTOM_FORMATTER(Type, str, ...)                                                                           \
  template <typename CharT>                                                                                            \
  struct fmt::formatter<Type, CharT> : fmt::formatter<std::basic_string_view<CharT>, CharT> {                          \
    template <typename FormatContext>                                                                                  \
    auto format(const Type& obj, FormatContext& ctx) const -> decltype(ctx.out()) {                                    \
      if constexpr (std::is_same_v<CharT, char>) {                                                                     \
        return fmt::format_to(ctx.out(), str __VA_OPT__(, ) __VA_ARGS__);                                              \
      } else if constexpr (std::is_same_v<CharT, char8_t>) {                                                           \
        return fmt::format_to(ctx.out(), u8##str __VA_OPT__(, ) __VA_ARGS__);                                          \
      } else if constexpr (std::is_same_v<CharT, char16_t>) {                                                          \
        return fmt::format_to(ctx.out(), u##str __VA_OPT__(, ) __VA_ARGS__);                                           \
      } else if constexpr (std::is_same_v<CharT, char32_t>) {                                                          \
        return fmt::format_to(ctx.out(), U##str __VA_OPT__(, ) __VA_ARGS__);                                           \
      } else if constexpr (std::is_same_v<CharT, wchar_t>) {                                                           \
        return fmt::format_to(ctx.out(), L##str __VA_OPT__(, ) __VA_ARGS__);                                           \
      } else {                                                                                                         \
        static_assert(!sizeof(CharT), "Unsupported character type for formatter");                                     \
      }                                                                                                                \
    }                                                                                                                  \
  }

FMT_CUSTOM_FORMATTER(metaforce::CAssetId, "{:08X}", obj.Value());
FMT_CUSTOM_FORMATTER(metaforce::FourCC, "{:c}{:c}{:c}{:c}", obj.getChars()[0], obj.getChars()[1], obj.getChars()[2],
                     obj.getChars()[3]);
FMT_CUSTOM_FORMATTER(metaforce::SObjectTag, "{} {}", obj.type, obj.id);
FMT_CUSTOM_FORMATTER(metaforce::TEditorId, "{:08X}", obj.id);
static_assert(sizeof(metaforce::kUniqueIdType) == sizeof(u16),
              "TUniqueId size does not match expected size! Update TUniqueId format string!");
FMT_CUSTOM_FORMATTER(metaforce::TUniqueId, "{:04X}", obj.id);

FMT_CUSTOM_FORMATTER(zeus::CVector3f, "({} {} {})", obj.x(), obj.y(), obj.z());
FMT_CUSTOM_FORMATTER(zeus::CVector2f, "({} {})", obj.x(), obj.y());
FMT_CUSTOM_FORMATTER(zeus::CMatrix3f,
                     "\n({} {} {})"
                     "\n({} {} {})"
                     "\n({} {} {})",
                     obj[0][0], obj[1][0], obj[2][0], obj[0][1], obj[1][1], obj[2][1], obj[0][2], obj[1][2], obj[2][2]);
FMT_CUSTOM_FORMATTER(zeus::CMatrix4f,
                     "\n({} {} {} {})"
                     "\n({} {} {} {})"
                     "\n({} {} {} {})"
                     "\n({} {} {} {})",
                     obj[0][0], obj[1][0], obj[2][0], obj[3][0], obj[0][1], obj[1][1], obj[2][1], obj[3][1], obj[0][2],
                     obj[1][2], obj[2][2], obj[3][2], obj[0][3], obj[1][3], obj[2][3], obj[3][3]);
FMT_CUSTOM_FORMATTER(zeus::CTransform,
                     "\n({} {} {} {})"
                     "\n({} {} {} {})"
                     "\n({} {} {} {})",
                     obj.basis[0][0], obj.basis[1][0], obj.basis[2][0], obj.origin[0], obj.basis[0][1], obj.basis[1][1],
                     obj.basis[2][1], obj.origin[1], obj.basis[0][2], obj.basis[1][2], obj.basis[2][2], obj.origin[2]);