#include <cstdint>

#include <zeus/CVector2f.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// To match imgui-sys
#define ImTextureID size_t

#define IM_VEC2_CLASS_EXTRA                                                                                            \
  ImVec2(const zeus::CVector2f& v) {                                                                                   \
    x = v.x();                                                                                                         \
    y = v.y();                                                                                                         \
  }                                                                                                                    \
  operator zeus::CVector2f() const { return zeus::CVector2f{x, y}; }
