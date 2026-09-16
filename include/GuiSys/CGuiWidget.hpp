#ifndef _CGUIWIDGET
#define _CGUIWIDGET

#include "GuiSys/CGuiObject.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/string.hpp"

class CFinalInput;
class CGuiFrame;
class CInputStream;
class CSimplePool;
enum ETraversalMode {
  kTM_ChildrenAndSiblings = 0,
  kTM_Children = 1,
  kTM_Single = 2,
};

class CGuiWidget : public CGuiObject {
public:
  enum EGuiModelDrawFlags {
    kGMDF_Shadeless = 0,
    kGMDF_Opaque = 1,
    kGMDF_Alpha = 2,
    kGMDF_Additive = 3,
    kGMDF_AlphaAdditiveOverdraw = 4
  };
  class CGuiWidgetParms {
  public:
    CGuiWidgetParms(CGuiFrame* frame, bool useAnimController, const short selfId,
                    const short parentId, bool defaultVisible, bool defaultActive, bool cullFaces,
                    const CColor& color, EGuiModelDrawFlags drawFlags, bool g, bool h);
    CGuiFrame* x0_frame;
    bool x4_useAnimController;
    short x6_selfId;
    short x8_parentId;
    bool xa_defaultVisible;
    bool xb_defaultActive;
    bool xc_cullFaces;
    bool xd_g;
    bool xe_h;
    CColor x10_color;
    EGuiModelDrawFlags x14_drawFlags;
  };

  CGuiWidget(const CGuiWidgetParms& parms);
  ~CGuiWidget();
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& drawParms) const override;
  void Initialize() override {}
  virtual void ProcessUserInput(const CFinalInput& input);
  virtual void Touch() const {}
  virtual bool GetIsVisible() const { return xb6_25_isVisible; }
  virtual bool GetIsActive() const;
  virtual FourCC GetWidgetTypeID() const { return 'BWIG'; }
  virtual bool AddWorkerWidget(CGuiWidget* worker) { return false; }
  virtual bool GetIsFinishedLoadingWidgetSpecific() const { return true; };
  bool GetIsFinishedLoading() const;
  virtual void OnVisible();
  virtual void OnActivate();
  short GetWidgetID() const { return x70_selfId; }
  short GetWorkerId() const { return xb4_workerId; }

  bool GetIsAlwaysDepthRead() const { return xb6_31_depthTest; }
  bool GetIsAlwaysDepthWrite() const { return xb7_24_depthWrite; }
  bool GetIsDepthBackwards() const { return xb6_30_depthGreater; }
  void SetIsDepthBackwards(bool depthGreater) { xb6_30_depthGreater = depthGreater; }
  void SetIsAlwaysDepthWrite(bool depthWrite) { xb7_24_depthWrite = depthWrite; }
  void SetDepthTest(bool depthTest) { xb6_31_depthTest = depthTest; }
  void SetIsSelectable(bool selectable) { xb6_27_isSelectable = selectable; }
  bool GetIsSelectable() const { return xb6_27_isSelectable; }
  void SetIsVisible(bool visible);
  void SetIsActive(bool active);
  bool GetIsBackfaceCullingOn() const { return xb6_29_cullFaces; }
  void SetColor(const CColor& color);
  const CColor& GetColor() const { return xa4_color; }
  const CColor& GetModifiedColor() const { return xa8_color2; }
  const EGuiModelDrawFlags GetDrawFlags() const { return xac_drawFlags; }
  void SetVisibility(bool visible, ETraversalMode mode);
  void InitializeRecursive();
  void RecalcWidgetColor(ETraversalMode mode);
  void ReapplyXform();
  CVector3f GetIdlePosition() const;
  const CTransform4f& GetIdleXform() const { return x74_transform; }
  void SetIdleXform(const CTransform4f& xf, bool reapply = true) {
    x74_transform = xf;
    if (reapply) {
      ReapplyXform();
    }
  }
  void AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd);
  CGuiWidget* FindWidget(short id);
  void ReadUnusedThing(CInputStream& in);
  void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms);

  static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in);

  const CTransform4f& GetTransform() const { return x74_transform; }
  CGuiFrame* GetParentFrame() const { return xb0_frame; }

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
  static const short InvalidWidgetId() { return gkInvalidWidgetId; }

  static const short gkDummyWidgetID;

protected:
  static const short gkInvalidWidgetId;
  short x70_selfId;
  short x72_parentId;
  CTransform4f x74_transform;
  CColor xa4_color;
  CColor xa8_color2;
  EGuiModelDrawFlags xac_drawFlags;
  CGuiFrame* xb0_frame;
  short xb4_workerId;
  bool xb6_24_pg : 1;
  bool xb6_25_isVisible : 1;
  bool xb6_26_isActive : 1;
  bool xb6_27_isSelectable : 1;
  bool xb6_28_eventLock : 1;
  bool xb6_29_cullFaces : 1;
  bool xb6_30_depthGreater : 1;
  bool xb6_31_depthTest : 1;
  bool xb7_24_depthWrite : 1;
  bool xb7_25_ : 1;
};
CHECK_SIZEOF(CGuiWidget, 0xb8)

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
CGuiWidget* FGuiWidgetFactoryInGame(uint type, CGuiFrame* parent, CInputStream& in, CSimplePool* sp,
                                    uint version);
#else
CGuiWidget* FGuiWidgetFactoryInGame(uint type, CGuiFrame* parent, CInputStream& in, CSimplePool* sp);
#endif

#endif // _CGUIWIDGET
