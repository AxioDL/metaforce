#include <cstdint>

#include "Runtime/RetroTypes.hpp"
#include <zeus/CVector2f.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// Use 32-bit index type for boo
#define ImDrawIdx uint32_t

enum ImUserTextureID : uint8_t {
  ImGuiUserTextureID_Atlas,
  ImGuiUserTextureID_MetaforceIcon,
  ImGuiUserTextureID_MAX,
};
struct ImUserTexture {
  ImUserTextureID textureId = ImGuiUserTextureID_MAX;
  metaforce::SObjectTag objectTag{};

  ImUserTexture() noexcept = default;
  ImUserTexture(long /* null */) noexcept {}
  ImUserTexture(ImUserTextureID textureId) noexcept : textureId(textureId) {}
  ImUserTexture(metaforce::SObjectTag objectTag) noexcept : objectTag(objectTag) {}
  bool operator==(const ImUserTexture& rhs) const { return textureId == rhs.textureId && objectTag == rhs.objectTag; }
  operator intptr_t() const { return (intptr_t(textureId) << 56) | objectTag.id.Value(); }
};
#define ImTextureID ImUserTexture

#define IM_VEC2_CLASS_EXTRA                                                                                            \
  ImVec2(const zeus::CVector2f& v) {                                                                                   \
    x = v.x();                                                                                                         \
    y = v.y();                                                                                                         \
  }                                                                                                                    \
  operator zeus::CVector2f() const { return zeus::CVector2f{x, y}; }
