#ifndef _CGUISYS
#define _CGUISYS

#include "types.h"

#include "rstl/rc_ptr.hpp"
#include "rstl/single_ptr.hpp"

#include "GuiSys/CGuiTextSupport.hpp"

class IFactory;
class CSimplePool;
class CTextExecuteBuffer;
class CTextParser;
class CGuiWidget;

class CGuiSys;
extern CGuiSys* gGuiSystem;

class CGuiSys {

public:
  enum EUsageMode {
    kUM_Zero,
    kUM_One,
    kUM_Two,
  };

  CGuiSys(IFactory*, CSimplePool*, EUsageMode);
  ~CGuiSys();

  static void SetGlobalGuiSys(CGuiSys* ptr) {
    gGuiSystem = ptr;
    spGuiSys = ptr;
  }

  static CGuiSys* GetGlobalGuiSys() { return spGuiSys; }
#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02
  static CGuiWidget* CreateWidgetInGame(uint type, CInputStream& in, CGuiFrame* parent, CSimplePool* sp);
#endif

  void AddFactories(EUsageMode mode);

  bool GetIsUsedInGame() const { return x8_mode == kUM_Zero; }
  EUsageMode GetUsageMode() const { return x8_mode; }

private:
  IFactory* x0_resFactory;
  CSimplePool* x4_resStore;
  EUsageMode x8_mode;
  rstl::single_ptr< CTextExecuteBuffer > xc_textExecuteBuffer;
  rstl::single_ptr< CTextParser > x10_textParser;
  static CGuiSys* spGuiSys;
};

CHECK_SIZEOF(CGuiSys, 0x14);

#endif // _CGUISYS
