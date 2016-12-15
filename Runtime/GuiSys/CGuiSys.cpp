#include "CGuiSys.hpp"
#include "CGuiWidget.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiBackground.hpp"
#include "CGuiLight.hpp"
#include "CGuiCamera.hpp"
#include "CGuiGroup.hpp"
#include "CGuiStaticImage.hpp"
#include "CGuiPane.hpp"
#include "CAuiImagePane.hpp"
#include "CAuiMeter.hpp"
#include "CGuiModel.hpp"
#include "CGuiTableGroup.hpp"
#include "CGuiSliderGroup.hpp"
#include "CGuiTextPane.hpp"
#include "CAuiEnergyBarT01.hpp"
#include "CTextParser.hpp"
#include "CSimplePool.hpp"
#include "CTextExecuteBuffer.hpp"

namespace urde
{

CGuiSys* g_GuiSys = nullptr;
CTextExecuteBuffer* g_TextExecuteBuf = nullptr;
CTextParser* g_TextParser = nullptr;

CGuiWidget* CGuiSys::CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* frame)
{
    switch (type)
    {
    case SBIG('BWIG'):
        return CGuiWidget::Create(frame, in, false);
    case SBIG('HWIG'):
        return CGuiHeadWidget::Create(frame, in, false);
    case SBIG('BGND'):
        return CGuiBackground::Create(frame, in, false);
    case SBIG('LITE'):
        return CGuiLight::Create(frame, in, false);
    case SBIG('CAMR'):
        return CGuiCamera::Create(frame, in, false);
    case SBIG('GRUP'):
        return CGuiGroup::Create(frame, in, false);
    case SBIG('IMAG'):
        return CGuiStaticImage::Create(frame, in, false);
    case SBIG('PANE'):
        return CGuiPane::Create(frame, in, false);
    case SBIG('IMGP'):
        return CAuiImagePane::Create(frame, in, false);
    case SBIG('METR'):
        return CAuiMeter::Create(frame, in, false);
    case SBIG('MODL'):
        return CGuiModel::Create(frame, in, false);
    case SBIG('TBGP'):
        return CGuiTableGroup::Create(frame, in, false);
    case SBIG('SLGP'):
        return CGuiSliderGroup::Create(frame, in, false);
    case SBIG('TXPN'):
        return CGuiTextPane::Create(frame, in, false);
    case SBIG('ENRG'):
        return CAuiEnergyBarT01::Create(frame, in, false);
    default: break;
    }
    return nullptr;
}

CGuiSys::CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode)
: x0_resFactory(resFactory), x4_resStore(resStore), x8_mode(mode),
  xc_textExecuteBuf(new CTextExecuteBuffer()),
  x10_textParser(new CTextParser(resStore))
{
    g_TextExecuteBuf = xc_textExecuteBuf.get();
    g_TextParser = x10_textParser.get();
}

}
