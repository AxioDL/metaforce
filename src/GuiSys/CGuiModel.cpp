#include "GuiSys/CGuiModel.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiSys.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"

CGuiModel* CGuiModel::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);

  CAssetId model = in.Get< CAssetId >();
  in.Get< uint >();
  uint lightMask = in.Get< uint >();

  CGuiModel* ret = rs_new CGuiModel(parms, sp, model, lightMask, true);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

CGuiModel::CGuiModel(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId modelId, uint lightMask,
                     bool flag)
: CGuiWidget(parms)
, mModelId(modelId)
, mLightMask(lightMask) {
  const CGuiSys* gs = CGuiSys::GetGlobalGuiSys();
  if (flag && mModelId != kInvalidAssetId && gs->GetUsageMode() != CGuiSys::kUM_Two) {
    mModel = sp->GetObj(SObjectTag('CMDL', modelId));
    mModel->Lock();
  }
}

CGuiModel::~CGuiModel() {}

void CGuiModel::Draw(const CGuiWidgetDrawParms& parms) const {
  CGraphics::SetModelMatrix(GetWorldTransform());
  if (!mModel) {
    return;
  }
  if (!GetIsFinishedLoading()) {
    return;
  }
  CModel* const model = mModel->GetObject();
  if (!model) {
    return;
  }
  if (GetIsVisible()) {
    CColor col = GetModifiedColor().WithAlphaModulatedBy(parms.GetAlpha());
    GetParentFrame()->EnableLights(mLightMask);

    const bool cullChanged = GetIsBackfaceCullingOn();
    if (cullChanged) {
      CGraphics::SetCullMode(kCM_Front);
    }

    CModelFlags flags = CModelFlags::Normal();
    bool doDraw = true;
    switch (GetDrawFlags()) {
    case kGMDF_Shadeless:
      flags = CModelFlags::Normal();
      break;
    case kGMDF_Opaque:
      flags = CModelFlags::ColorModulate(col);
      break;
    case kGMDF_Alpha:
      flags = CModelFlags::AlphaBlended(col).DepthCompareUpdate(GetIsAlwaysDepthRead(),
                                                              GetIsAlwaysDepthWrite());
      break;
    case kGMDF_Additive:
      flags = CModelFlags::Additive(col).DepthCompareUpdate(GetIsAlwaysDepthRead(),
                                                          GetIsAlwaysDepthWrite());
      break;
    case kGMDF_AlphaAdditiveOverdraw:
      flags = CModelFlags::AlphaBlended(col).DepthCompareUpdate(GetIsAlwaysDepthRead(), false);
      if (GetIsDepthBackwards()) {
        flags = flags.DepthBackwards();
      }
      model->Draw(flags);
      flags = CModelFlags::AdditiveRGB(col).DepthCompareUpdate(GetIsAlwaysDepthRead(),
             GetIsAlwaysDepthWrite());
      if (GetIsDepthBackwards()) {
        flags = flags.DepthBackwards();
      }
      model->Draw(flags);
      doDraw = false;
      break;
    default:
      doDraw = false;
      break;
    }

    if (doDraw) {
      if (GetIsDepthBackwards()) {
        flags = flags.DepthBackwards();
      }
      model->Draw(flags);
    }

    if (cullChanged) {
      CGraphics::SetCullMode(kCM_None);
    }
    GetParentFrame()->DisableLights();
  }

  CGuiWidget::Draw(parms);
}

void CGuiModel::Touch() const {
  if (mModel && mModel->GetObject()) {
    mModel->GetObject()->Touch(0);
  }
}

bool CGuiModel::GetIsFinishedLoadingWidgetSpecific() const {
  if (!mModel.valid()) {
    return true;
  }
  mModel->TryCache();

  if (const CModel* model = mModel->GetObject()) {
    model->Touch(0);
    return model->IsLoaded(0);
  }

  return false;
}

rstl::vector< CAssetId > CGuiModel::GetModelAssets() const {
  return rstl::vector< CAssetId >(1, mModelId);
}

FourCC CGuiModel::GetWidgetTypeID() const { return 'MODL'; }
