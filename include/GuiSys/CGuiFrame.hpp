#ifndef _CGUIFRAME
#define _CGUIFRAME

#include "GuiSys/CGuiWidgetIdDB.hpp"
#include "rstl/string.hpp"

class CFinalInput;
class CGuiSys;
class CGuiWidget;
class CGuiCamera;
class CGuiLight;
class CGuiHeadWidget;
class CGuiWidgetDrawParms;
class CSimplePool;
class CGuiFrame {
public:
  CGuiFrame(uint id, CGuiSys& sys, int a, int b, int c, CSimplePool* sp);
  ~CGuiFrame();
  static CGuiFrame* CreateFrame(uint id, CGuiSys& sys, CInputStream& in, CSimplePool* sp);
#if VERSION >= VERSION_GM8P_00
  int LoadWidgetsInGame(CInputStream& in, CSimplePool* sp, uint version);
#else
  int LoadWidgetsInGame(CInputStream& in, CSimplePool* sp);
#endif
  void Initialize();
  void Touch() const;
  void SortDrawOrder();
  CGuiLight* GetFrameLight(int idx);
  void Update(float dt);
  void Draw(const CGuiWidgetDrawParms& parms) const;
  void ProcessUserInput(const CFinalInput& input);
  CGuiWidget* FindWidget(const short id) const;
  CGuiWidget* FindWidget(const char* name) const;
  CGuiWidget* FindWidget(const rstl::string& name) const;
  bool GetIsFinishedLoading() const;

  CGuiCamera* GetFrameCamera() const { return mCamera; }
  void SetFrameCamera(CGuiCamera* camera);
  void AddLight(CGuiLight* light);
  void RemoveLight(CGuiLight* light);
  void SetHeadWidget(CGuiHeadWidget* widget);
  void RemoveWidgetFromDrawList(CGuiWidget* widget);

  short AddWidgetToIDDB(const rstl::string& name) { return mDb.AddWidget(name); }
  CGuiWidgetIdDB& WidgetIdDB() { return mDb; }

  CGuiSys& GetGuiSys() const { return mGuiSys; }

  void EnableLights(uint mask) const;
  void DisableLights() const;

private:
  uint mId;
  uint x4_;
  CGuiSys& mGuiSys;
  CGuiHeadWidget* mHeadWidget;
  CGuiWidget* mRootWidget;
  CGuiCamera* mCamera;
  CGuiWidgetIdDB mDb;
  rstl::vector< CGuiWidget* > mWidgets;
  rstl::vector< CGuiLight* > mLights;
  int mA;
  int mB;
  int mC;
  mutable bool mLoaded : 1;
};
CHECK_SIZEOF(CGuiFrame, 0x5c);

#endif // _CGUIFRAME
