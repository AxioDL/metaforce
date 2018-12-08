#include "CGuiModel.hpp"
#include "CGuiFrame.hpp"
#include "CGuiSys.hpp"
#include "CSimplePool.hpp"
#include "CGuiWidgetDrawParms.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde {

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId modelId, u32 lightMask, bool flag)
: CGuiWidget(parms), xc8_modelId(modelId), xcc_lightMask(lightMask) {
  if (!flag || !modelId.IsValid() || parms.x0_frame->GetGuiSys().GetUsageMode() == CGuiSys::EUsageMode::Two)
    return;

  xb8_model = sp->GetObj({SBIG('CMDL'), modelId});
}

std::vector<CAssetId> CGuiModel::GetModelAssets() const { return {xc8_modelId}; }

bool CGuiModel::GetIsFinishedLoadingWidgetSpecific() const {
  if (!xb8_model)
    return true;
  const CModel* model = xb8_model.GetObj();
  if (!model)
    return false;
  model->GetInstance().Touch(0);
  return model->IsLoaded(0);
}

void CGuiModel::Touch() const {
  const CModel* model = xb8_model.GetObj();
  if (model)
    model->GetInstance().Touch(0);
}

void CGuiModel::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(x34_worldXF);
  if (!xb8_model)
    return;
  if (!GetIsFinishedLoading())
    return;
  const CModel* model = xb8_model.GetObj();
  if (!model)
    return;

  if (GetIsVisible()) {
    zeus::CColor moduCol = xa8_color2;
    moduCol.a() *= parms.x0_alphaMod;
    xb0_frame->EnableLights(xcc_lightMask, const_cast<CBooModel&>(model->GetInstance()));
    // if (xb6_29_cullFaces)
    //    CGraphics::SetCullMode(ERglCullMode::Front);

    switch (xac_drawFlags) {
    case EGuiModelDrawFlags::Shadeless: {
      CModelFlags flags(0, 0, 3, zeus::CColor::skWhite);
      flags.m_extendedShader = EExtendedShader::Flat;
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::Opaque: {
      CModelFlags flags(1, 0, 3, moduCol);
      flags.m_extendedShader = EExtendedShader::Lighting;
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::Alpha: {
      CModelFlags flags(5, 0, (u32(xb7_24_depthWrite) << 1) | u32(xb6_31_depthTest), moduCol);
      flags.m_noCull = !xb6_29_cullFaces;
      flags.m_noZWrite = !xb7_24_depthWrite;
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::Additive: {
      CModelFlags flags(7, 0, (u32(xb7_24_depthWrite) << 1) | u32(xb6_31_depthTest), moduCol);
      flags.m_noCull = !xb6_29_cullFaces;
      flags.m_noZWrite = !xb7_24_depthWrite;
      model->Draw(flags);
      break;
    }
    case EGuiModelDrawFlags::AlphaAdditiveOverdraw: {
      CModelFlags flags(5, 0, xb6_31_depthTest, moduCol);
      flags.m_noCull = !xb6_29_cullFaces;
      flags.m_noZWrite = !xb7_24_depthWrite;
      model->Draw(flags);

      flags.x0_blendMode = 7;
      flags.x1_matSetIdx = 0;
      flags.x2_flags = (u32(xb7_24_depthWrite) << 1) | u32(xb6_31_depthTest);
      flags.x4_color = moduCol;
      flags.m_noCull = !xb6_29_cullFaces;
      model->Draw(flags);
      break;
    }
    default:
      break;
    }

    // if (xb6_29_cullFaces)
    //    CGraphics::SetCullMode(ERglCullMode::None);
    xb0_frame->DisableLights();
  }

  CGuiWidget::Draw(parms);
}

std::shared_ptr<CGuiWidget> CGuiModel::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);

  CAssetId model = in.readUint32Big();
  in.readUint32Big();
  u32 lightMask = in.readUint32Big();

  std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiModel>(parms, sp, model, lightMask, true);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace urde
