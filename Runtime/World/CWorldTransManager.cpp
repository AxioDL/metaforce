#include "CWorldTransManager.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

void CWorldTransManager::DrawEnabled() const
{
}

void CWorldTransManager::DrawDisabled() const
{
}

void CWorldTransManager::StartTextTransition(ResId fontId, ResId stringId, bool b1, bool b2,
                                             float chFadeTime, float chFadeRate, float f3)
{
    x40_ = b1;
    x38_ = f3;
    x44_25_ = false;
    x30_type = ETransType::Text;

    x4_modelData.reset();
    x44_27_ = b2;

    CGuiTextProperties props(false, true, EJustification::Center,
                             EVerticalJustification::Center, ETextDirection::Horizontal);
    x8_textData.reset(new CGuiTextSupport(fontId, props, zeus::CColor::skWhite,
                                          zeus::CColor::skBlack, zeus::CColor::skWhite,
                                          640, 448, g_SimplePool));

    x8_textData->SetTypeWriteEffectOptions(true, chFadeTime, chFadeRate);
    xc_strTable = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), stringId});
    x8_textData->SetText(L"");
    StartTransition();
}

}
