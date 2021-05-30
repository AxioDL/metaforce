#include <cstdint>

#include <zeus/CVector2f.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// Use 32-bit index type for boo
#define ImDrawIdx uint32_t

enum ImUserTextureID {
  ImGuiUserTextureID_Atlas,
  ImGuiUserTextureID_MetaforceIcon,
  ImGuiUserTextureID_MAX,
};
#define ImTextureID ImUserTextureID

#define IM_VEC2_CLASS_EXTRA                                                                                            \
  ImVec2(const zeus::CVector2f& v) {                                                                                   \
    x = v.x();                                                                                                         \
    y = v.y();                                                                                                         \
  }                                                                                                                    \
  operator zeus::CVector2f() const { return zeus::CVector2f{x, y}; }
