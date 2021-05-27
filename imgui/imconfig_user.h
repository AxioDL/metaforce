#include <cstdint>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// Use 32-bit index type for boo
#define ImDrawIdx uint32_t

enum ImUserTextureID {
  ImGuiUserTextureID_Atlas,
  ImGuiUserTextureID_MetaforceIcon,
  ImGuiUserTextureID_MAX,
};
#define ImTextureID ImUserTextureID
