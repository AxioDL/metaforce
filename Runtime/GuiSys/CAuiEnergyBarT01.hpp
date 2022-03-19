#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CEnergyBarShader.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CSimplePool;

class CAuiEnergyBarT01 : public CGuiWidget {
public:
  using FCoordFunc = std::pair<zeus::CVector3f, zeus::CVector3f> (*)(float t);
  enum class ESetMode { Normal, Wrapped, Insta };

private:
  CAssetId xb8_txtrId;
  TLockedToken<CTexture> xbc_tex; // Used to be optional
  zeus::CColor xcc_emptyColor;
  zeus::CColor xd0_filledColor;
  zeus::CColor xd4_shadowColor;
  FCoordFunc xd8_coordFunc = nullptr;
  float xdc_tesselation = 1.f;
  float xe0_maxEnergy = 0.f;
  float xe4_filledSpeed = 1000.f;
  float xe8_shadowSpeed = 1000.f;
  float xec_shadowDrainDelay = 0.f;
  bool xf0_alwaysResetDelayTimer = false;
  bool xf1_wrapping = false;
  float xf4_setEnergy = 0.f;
  float xf8_filledEnergy = 0.f;
  float xfc_shadowEnergy = 0.f;
  float x100_shadowDrainDelayTimer = 0.f;

public:
  CAuiEnergyBarT01(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId txtrId);
  FourCC GetWidgetTypeID() const override { return FOURCC('ENRG'); }
  static std::pair<zeus::CVector3f, zeus::CVector3f> DownloadBarCoordFunc(float t);
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& drawParms) override;
  float GetActualFraction() const { return xe0_maxEnergy == 0.f ? 0.f : xf4_setEnergy / xe0_maxEnergy; }
  float GetSetEnergy() const { return xf4_setEnergy; }
  float GetMaxEnergy() const { return xe0_maxEnergy; }
  float GetFilledEnergy() const { return xf8_filledEnergy; }
  void SetCurrEnergy(float e, ESetMode mode);
  void SetCoordFunc(FCoordFunc func) { xd8_coordFunc = func; }
  void SetEmptyColor(const zeus::CColor& c) { xcc_emptyColor = c; }
  void SetFilledColor(const zeus::CColor& c) { xd0_filledColor = c; }
  void SetShadowColor(const zeus::CColor& c) { xd4_shadowColor = c; }
  void SetMaxEnergy(float maxEnergy);
  void ResetMaxEnergy() { SetMaxEnergy(xdc_tesselation); }
  void SetTesselation(float t) { xdc_tesselation = t; }
  void SetIsAlwaysResetTimer(bool b) { xf0_alwaysResetDelayTimer = b; }
  void SetFilledDrainSpeed(float s) { xe4_filledSpeed = s; }
  void SetShadowDrainSpeed(float s) { xe8_shadowSpeed = s; }
  void SetShadowDrainDelay(float d) { xec_shadowDrainDelay = d; }
  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace metaforce
