#include "Runtime/GuiSys/CAuiEnergyBarT01.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"

namespace metaforce {

CAuiEnergyBarT01::CAuiEnergyBarT01(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId txtrId)
: CGuiWidget(parms), xb8_txtrId(txtrId) {
  if (g_GuiSys->GetUsageMode() != CGuiSys::EUsageMode::Two) {
    xbc_tex = sp->GetObj(SObjectTag{FOURCC('TXTR'), xb8_txtrId});
  }
}

std::pair<zeus::CVector3f, zeus::CVector3f> CAuiEnergyBarT01::DownloadBarCoordFunc(float t) {
  const float x = 12.5f * t - 6.25f;
  return {zeus::CVector3f{x, 0.f, -0.2f}, zeus::CVector3f{x, 0.f, 0.2f}};
}

void CAuiEnergyBarT01::Update(float dt) {
  if (x100_shadowDrainDelayTimer > 0.f) {
    x100_shadowDrainDelayTimer = std::max(x100_shadowDrainDelayTimer - dt, 0.f);
  }

  if (xf8_filledEnergy < xf4_setEnergy) {
    if (xf1_wrapping) {
      xf8_filledEnergy -= dt * xe4_filledSpeed;
      if (xf8_filledEnergy < 0.f) {
        xf8_filledEnergy = std::max(xf4_setEnergy, xf8_filledEnergy + xe0_maxEnergy);
        xf1_wrapping = false;
        xfc_shadowEnergy = xe0_maxEnergy;
      }
    } else {
      xf8_filledEnergy = std::min(xf4_setEnergy, xf8_filledEnergy + dt * xe4_filledSpeed);
    }
  } else if (xf8_filledEnergy > xf4_setEnergy) {
    if (xf1_wrapping) {
      xf8_filledEnergy += dt * xe4_filledSpeed;
      if (xf8_filledEnergy > xe0_maxEnergy) {
        xf8_filledEnergy = std::min(xf4_setEnergy, xf8_filledEnergy - xe0_maxEnergy);
        xf1_wrapping = false;
        xfc_shadowEnergy = xf8_filledEnergy;
      }
    } else {
      xf8_filledEnergy = std::max(xf4_setEnergy, xf8_filledEnergy - dt * xe4_filledSpeed);
    }
  }

  if (xfc_shadowEnergy < xf8_filledEnergy) {
    xfc_shadowEnergy = xf8_filledEnergy;
  } else if (xfc_shadowEnergy > xf8_filledEnergy && x100_shadowDrainDelayTimer == 0.f) {
    xfc_shadowEnergy = std::max(xf8_filledEnergy, xfc_shadowEnergy - dt * xe8_shadowSpeed);
  }

  CGuiWidget::Update(dt);
}

void CAuiEnergyBarT01::Draw(const CGuiWidgetDrawParms& drawParms) {
  CGraphics::SetModelMatrix(x34_worldXF);
  if (!xbc_tex || !xbc_tex.IsLoaded() || xd8_coordFunc == nullptr) {
    return;
  }

  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
  CGraphics::SetAmbientColor(zeus::skWhite);
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);

  const float filledT = xe0_maxEnergy > 0.f ? xf8_filledEnergy / xe0_maxEnergy : 0.f;
  const float shadowT = xe0_maxEnergy > 0.f ? xfc_shadowEnergy / xe0_maxEnergy : 0.f;

  zeus::CColor filledColor = xd0_filledColor;
  filledColor.a() *= drawParms.x0_alphaMod;
  filledColor *= xa8_color2;

  zeus::CColor shadowColor = xd4_shadowColor;
  shadowColor.a() *= drawParms.x0_alphaMod;
  shadowColor *= xa8_color2;

  zeus::CColor emptyColor = xcc_emptyColor;
  emptyColor.a() *= drawParms.x0_alphaMod;
  emptyColor *= xa8_color2;

  zeus::CColor useCol = emptyColor;
  for (u32 i = 0; i < 3; ++i) {
    float barOffT;
    if (i == 0) {
      barOffT = 0.f;
    } else if (i == 1) {
      barOffT = filledT;
    } else {
      barOffT = shadowT;
    }

    float barMaxT;
    if (i == 0) {
      barMaxT = filledT;
    } else if (i == 1) {
      barMaxT = shadowT;
    } else {
      barMaxT = 1.f;
    }

    if (i == 0) {
      useCol = filledColor;
    } else if (i == 1) {
      useCol = shadowColor;
    } else {
      useCol = emptyColor;
    }

    if (barOffT != barMaxT) {
      CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
      CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
      xbc_tex->Load(GX::TEXMAP0, EClampMode::Repeat);
      CGraphics::StreamBegin(GX::TRIANGLESTRIP);
      CGraphics::StreamColor(useCol);
      auto coords = xd8_coordFunc(barOffT);
      while (barOffT < barMaxT) {
        CGraphics::StreamTexcoord(barOffT, 0.f);
        CGraphics::StreamVertex(coords.first);
        CGraphics::StreamTexcoord(barOffT, 1.f);
        CGraphics::StreamVertex(coords.second);
        barOffT += xdc_tesselation;
        if (barOffT < barMaxT) {
          coords = xd8_coordFunc(barOffT);
        } else {
          coords = xd8_coordFunc(barMaxT);
          CGraphics::StreamTexcoord(barMaxT, 0.f);
          CGraphics::StreamVertex(coords.first);
          CGraphics::StreamTexcoord(barMaxT, 1.f);
          CGraphics::StreamVertex(coords.second);
        }
      }
      CGraphics::StreamEnd();
    }
  }
  CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
}

void CAuiEnergyBarT01::SetCurrEnergy(float e, ESetMode mode) {
  e = zeus::clamp(0.f, e, xe0_maxEnergy);
  if (e == xf4_setEnergy) {
    return;
  }
  if (xf0_alwaysResetDelayTimer || xf8_filledEnergy == xfc_shadowEnergy) {
    x100_shadowDrainDelayTimer = xec_shadowDrainDelay;
  }
  xf1_wrapping = mode == ESetMode::Wrapped;
  xf4_setEnergy = e;
  if (mode == ESetMode::Insta) {
    xf8_filledEnergy = xf4_setEnergy;
  }
}

void CAuiEnergyBarT01::SetMaxEnergy(float maxEnergy) {
  xe0_maxEnergy = maxEnergy;
  xf4_setEnergy = std::min(xe0_maxEnergy, xf4_setEnergy);
  xf8_filledEnergy = std::min(xe0_maxEnergy, xf8_filledEnergy);
  xfc_shadowEnergy = std::min(xe0_maxEnergy, xfc_shadowEnergy);
}

std::shared_ptr<CGuiWidget> CAuiEnergyBarT01::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  auto tex = in.Get<CAssetId>();
  std::shared_ptr<CGuiWidget> ret = std::make_shared<CAuiEnergyBarT01>(parms, sp, tex);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace metaforce
