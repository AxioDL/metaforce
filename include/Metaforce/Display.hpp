#pragma once

#include "types.h"

struct SDL_Window;
class CColor;
class CTexture;

namespace metaforce {

constexpr float DisplayAspectScale(uint width, uint height, uint logicalWidth, uint logicalHeight) {
  if (!width || !height || !logicalWidth || !logicalHeight) {
    return 1.f;
  }
  const float scale = (static_cast< float >(width) / static_cast< float >(height)) /
                      (static_cast< float >(logicalWidth) / static_cast< float >(logicalHeight));
  return scale > 1.f ? scale : 1.f;
}

void ConfigureDisplay(SDL_Window* window, bool originalAspect);
void UpdateDisplayPolicy();
void UpdateDisplayAspect();
float GetDisplayAspectScale();
float AdjustDisplayAspect(float aspect);
float ScreenToUiX(float x, float width);
float UiToScreenX(float x, float width);

void AdjustUiProjection();
void RenderUiTexture(const CTexture& texture, int x, int y, int width, int height,
                     const CColor& color);

} // namespace metaforce
