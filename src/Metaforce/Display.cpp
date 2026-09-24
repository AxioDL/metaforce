#include "Metaforce/Display.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"

#include <SDL3/SDL_video.h>
#include <dolphin/gx/GXAurora.h>

#include <cmath>

namespace metaforce {
namespace {
SDL_Window* sWindow = nullptr;
bool sLockAspect = false;
float sAspectScale = 1.f;
AuroraViewportPolicy sViewportPolicy = AURORA_VIEWPORT_FIT;

void LogicalSize(unsigned int& width, unsigned int& height) {
  const auto& mode = CGraphics::GetRenderMode();
  width = mode.fbWidth ? mode.fbWidth : 640;
  height = mode.efbHeight ? mode.efbHeight : 448;
}
} // namespace

void ConfigureDisplay(SDL_Window* window, bool originalAspect) {
  sWindow = window;
  sLockAspect = originalAspect;
  sAspectScale = 1.f;
  AuroraSetViewportPolicy(sViewportPolicy);
  UpdateDisplayPolicy();
}

void UpdateDisplayPolicy() {
  int width = 0;
  int height = 0;
  if (!sWindow || !SDL_GetWindowSizeInPixels(sWindow, &width, &height) || width <= 0 ||
      height <= 0) {
    return;
  }
  uint logicalWidth, logicalHeight;
  LogicalSize(logicalWidth, logicalHeight);
  const auto policy =
      !sLockAspect && DisplayAspectScale(width, height, logicalWidth, logicalHeight) > 1.f
          ? AURORA_VIEWPORT_STRETCH
          : AURORA_VIEWPORT_FIT;
  if (policy != sViewportPolicy) {
    sViewportPolicy = policy;
    AuroraSetViewportPolicy(policy);
  }
}

void UpdateDisplayAspect() {
  uint width, height, logicalWidth, logicalHeight;
  AuroraGetRenderSize(&width, &height);
  LogicalSize(logicalWidth, logicalHeight);
  sAspectScale = sViewportPolicy == AURORA_VIEWPORT_FIT
                     ? 1.f
                     : DisplayAspectScale(width, height, logicalWidth, logicalHeight);
}

float GetDisplayAspectScale() { return sAspectScale; }

float AdjustDisplayAspect(float aspect) { return aspect * sAspectScale; }

float ScreenToUiX(float x, float width) {
  const float center = width * 0.5f;
  return center + (x - center) * sAspectScale;
}

float UiToScreenX(float x, float width) {
  const float center = width * 0.5f;
  return center + (x - center) / sAspectScale;
}

void AdjustUiProjection() {
  if (sAspectScale == 1.f) {
    return;
  }
  const auto& projection = CGraphics::GetProjectionState();
  const float center = (projection.GetLeft() + projection.GetRight()) * 0.5f;
  const float halfWidth = (projection.GetRight() - projection.GetLeft()) * 0.5f * sAspectScale;
  CGraphics::SetProjectionState(CGraphics::CProjectionState(
      projection.IsPerspective(), center - halfWidth, center + halfWidth, projection.GetTop(),
      projection.GetBottom(), projection.GetNear(), projection.GetFar()));
}

void RenderUiTexture(const CTexture& texture, int x, int y, int width, int height,
                     const CColor& color) {
  const auto& viewport = CGraphics::GetViewport();
  const int left = static_cast< int >(
      std::lround(UiToScreenX(static_cast< float >(x), static_cast< float >(viewport.mWidth))));
  const int right = static_cast< int >(std::lround(
      UiToScreenX(static_cast< float >(x + width), static_cast< float >(viewport.mWidth))));
  CGraphics::Render2D(texture, left, y, right - left, height, color);
}
} // namespace metaforce
