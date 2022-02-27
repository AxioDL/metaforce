#include <cstdint>

#include <zeus/CVector2f.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO

#ifdef NDEBUG
#define IMGUI_DISABLE_DEMO_WINDOWS
#endif

#define IM_VEC2_CLASS_EXTRA                                                                                            \
  ImVec2(const zeus::CVector2f& v) {                                                                                   \
    x = v.x();                                                                                                         \
    y = v.y();                                                                                                         \
  }                                                                                                                    \
  operator zeus::CVector2f() const { return zeus::CVector2f{x, y}; }
