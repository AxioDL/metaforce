#include "Runtime/GuiSys/CGuiModel.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"

namespace metaforce {

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId modelId, u32 lightMask, bool flag)
: CGuiWidget(parms), xc8_modelId(modelId), xcc_lightMask(lightMask) {
  if (!flag || !modelId.IsValid() || parms.x0_frame->GetGuiSys().GetUsageMode() == CGuiSys::EUsageMode::Two) {
    return;
  }
  xb8_model = sp->GetObj({SBIG('CMDL'), modelId});
}

bool CGuiModel::GetIsFinishedLoadingWidgetSpecific() {
  if (!xb8_model) {
    return true;
  }
  if (!xb8_model.IsLoaded()) {
    return false;
  }
  xb8_model->Touch(0);
  return xb8_model->IsLoaded(0);
}

void CGuiModel::Touch() {
  if (CModel* const model = xb8_model.GetObj()) {
    model->Touch(0);
  }
}

void CGuiModel::Draw(const CGuiWidgetDrawParms& parms) {
  CGraphics::SetModelMatrix(x34_worldXF);
  if (!xb8_model) {
    return;
  }
  if (!GetIsFinishedLoading()) {
    return;
  }
  CModel* const model = xb8_model.GetObj();
  if (model == nullptr) {
    return;
  }

  if (GetIsVisible()) {
    SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format("CGuiModel::Draw {}", m_name).c_str(), zeus::skCyan);
    zeus::CColor moduCol = xa8_color2;
    moduCol.a() *= parms.x0_alphaMod;
    xb0_frame->EnableLights(xcc_lightMask);
    if (xb6_29_cullFaces) {
      CGraphics::SetCullMode(ERglCullMode::Front);
    }

    switch (xac_drawFlags) {
    case EGuiModelDrawFlags::Shadeless: {
      constexpr CModelFlags flags(0, 0, 3, zeus::skWhite);
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::Opaque: {
      CModelFlags flags(1, 0, 3, moduCol);
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::Alpha: {
      CModelFlags flags(5, 0, (u32(xb7_24_depthWrite) << 1) | u32(xb6_31_depthTest), moduCol);
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::Additive: {
      CModelFlags flags(7, 0, (u32(xb7_24_depthWrite) << 1) | u32(xb6_31_depthTest), moduCol);
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::AlphaAdditiveOverdraw: {
      const CModelFlags flags(5, 0, (u32(xb6_30_depthGreater) << 4) | u32(xb6_31_depthTest), moduCol);
      model->Draw(flags);

      const CModelFlags overdrawFlags(
          8, 0, (u32(xb6_30_depthGreater) << 4) | (u32(xb7_24_depthWrite) << 1) | u32(xb6_31_depthTest), moduCol);
      model->Draw(overdrawFlags);
      break;
    }
    default:
      break;
    }

    if (xb6_29_cullFaces) {
      CGraphics::SetCullMode(ERglCullMode::None);
    }
    xb0_frame->DisableLights();
  }

  CGuiWidget::Draw(parms);
}

bool CGuiModel::TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const {
  if (!xb8_model || !xb8_model.IsLoaded()) {
    return false;
  }
  return xb8_model->GetAABB().projectedPointTest(vp * x34_worldXF.toMatrix4f(), point);
}

std::shared_ptr<CGuiWidget> CGuiModel::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);

  auto model = in.Get<CAssetId>();
  in.ReadLong();
  u32 lightMask = in.ReadLong();

  std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiModel>(parms, sp, model, lightMask, true);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace metaforce
