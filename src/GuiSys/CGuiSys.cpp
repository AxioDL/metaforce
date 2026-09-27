#include "GuiSys/CGuiSys.hpp"

#include "GuiSys/CGuiTextSupport.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "Kyoto/Text/CTextParser.hpp"

CGuiSys* CGuiSys::spGuiSys = nullptr;

CGuiSys::CGuiSys(IFactory* factory, CSimplePool* pool, EUsageMode mode)
: mResFactory(factory), mResStore(pool), mMode(mode) {
  AddFactories(mMode);
  mTextExecuteBuffer = rs_new CTextExecuteBuffer();
  mTextParser = rs_new CTextParser(*pool);

  CGuiTextSupport::Initialize(mTextExecuteBuffer.get(), mTextParser.get());
}

CGuiSys::~CGuiSys() {}

#if VERSION < VERSION_GM8P_00
CGuiWidget* CGuiSys::CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* parent, CSimplePool* sp) {
  return FGuiWidgetFactoryInGame(type, parent, in, sp);
}
#endif

void CGuiSys::AddFactories(EUsageMode mode) {}
