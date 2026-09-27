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
    CGuiFrame* mFrame;
    bool mUseAnimController;
    short mSelfId;
    short mParentId;
    bool mDefaultVisible;
    bool mDefaultActive;
    bool mCullFaces;
    bool mG;
    bool mH;
    CColor mColor;
    EGuiModelDrawFlags mDrawFlags;
  };

  CGuiWidget(const CGuiWidgetParms& parms);
  ~CGuiWidget();
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& drawParms) const override;
  void Initialize() override {}
  virtual void ProcessUserInput(const CFinalInput& input);
  virtual void Touch() const {}
  virtual bool GetIsVisible() const { return mIsVisible; }
  virtual bool GetIsActive() const;
  virtual FourCC GetWidgetTypeID() const { return 'BWIG'; }
  virtual bool AddWorkerWidget(CGuiWidget* worker) { return false; }
  virtual bool GetIsFinishedLoadingWidgetSpecific() const { return true; };
  bool GetIsFinishedLoading() const;
  virtual void OnVisible();
  virtual void OnActivate();
  short GetWidgetID() const { return mSelfId; }
  short GetWorkerId() const { return mWorkerId; }

  bool GetIsAlwaysDepthRead() const { return mDepthTest; }
  bool GetIsAlwaysDepthWrite() const { return mDepthWrite; }
  bool GetIsDepthBackwards() const { return mDepthGreater; }
  void SetIsDepthBackwards(bool depthGreater) { mDepthGreater = depthGreater; }
  void SetIsAlwaysDepthWrite(bool depthWrite) { mDepthWrite = depthWrite; }
  void SetDepthTest(bool depthTest) { mDepthTest = depthTest; }
  void SetIsSelectable(bool selectable) { mIsSelectable = selectable; }
  bool GetIsSelectable() const { return mIsSelectable; }
  void SetIsVisible(bool visible);
  void SetIsActive(bool active);
  bool GetIsBackfaceCullingOn() const { return mCullFaces; }
  void SetColor(const CColor& color);
  const CColor& GetColor() const { return mColor; }
  const CColor& GetModifiedColor() const { return mColor2; }
  const EGuiModelDrawFlags GetDrawFlags() const { return mDrawFlags; }
  void SetVisibility(bool visible, ETraversalMode mode);
  void InitializeRecursive();
  void RecalcWidgetColor(ETraversalMode mode);
  void ReapplyXform();
  CVector3f GetIdlePosition() const;
  const CTransform4f& GetIdleXform() const { return mTransform; }
  void SetIdleXform(const CTransform4f& xf, bool reapply = true) {
    mTransform = xf;
    if (reapply) {
      ReapplyXform();
    }
  }
  void AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd);
  CGuiWidget* FindWidget(short id);
  void ReadUnusedThing(CInputStream& in);
  void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms);

  static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in);

  const CTransform4f& GetTransform() const { return mTransform; }
  CGuiFrame* GetParentFrame() const { return mFrame; }

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
  static const short InvalidWidgetId() { return gkInvalidWidgetId; }

  static const short gkDummyWidgetID;

protected:
  static const short gkInvalidWidgetId;
  short mSelfId;
  short mParentId;
  CTransform4f mTransform;
  CColor mColor;
  CColor mColor2;
  EGuiModelDrawFlags mDrawFlags;
  CGuiFrame* mFrame;
  short mWorkerId;
  bool mPg : 1;
  bool mIsVisible : 1;
  bool mIsActive : 1;
  bool mIsSelectable : 1;
  bool mEventLock : 1;
  bool mCullFaces : 1;
  bool mDepthGreater : 1;
  bool mDepthTest : 1;
  bool mDepthWrite : 1;
  bool xb7_25_ : 1;
};
CHECK_SIZEOF(CGuiWidget, 0xb8)

#if VERSION >= VERSION_GM8P_00
CGuiWidget* FGuiWidgetFactoryInGame(uint type, CGuiFrame* parent, CInputStream& in, CSimplePool* sp,
                                    uint version);
#else
CGuiWidget* FGuiWidgetFactoryInGame(uint type, CGuiFrame* parent, CInputStream& in, CSimplePool* sp);
#endif

#endif // _CGUIWIDGET
