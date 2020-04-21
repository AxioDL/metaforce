#pragma once

#include <memory>
#include <optional>
#include <string>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/GuiSys/CGuiObject.hpp"

#include <boo/IWindow.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CGuiFrame;
class CGuiTextSupport;
struct CFinalInput;
class CSimplePool;

enum class ETraversalMode { ChildrenAndSiblings = 0, Children = 1, Single = 2 };

enum class EGuiTextureClampModeHorz { NoClamp = 0, Right = 1, Left = 2, Center = 3 };

enum class EGuiTextureClampModeVert { NoClamp = 0, Top = 1, Bottom = 2, Center = 3 };

class CGuiWidget : public CGuiObject {
  friend class CGuiFrame;

public:
  enum class EGuiModelDrawFlags { Shadeless = 0, Opaque = 1, Alpha = 2, Additive = 3, AlphaAdditiveOverdraw = 4 };
  struct CGuiWidgetParms {
    CGuiFrame* x0_frame;
    bool x4_useAnimController;
    s16 x6_selfId;
    s16 x8_parentId;
    bool xa_defaultVisible;
    bool xb_defaultActive;
    bool xc_cullFaces;
    bool xd_g;
    bool xe_h;
    zeus::CColor x10_color;
    EGuiModelDrawFlags x14_drawFlags;
    std::string m_name;
    CGuiWidgetParms(CGuiFrame* frame, bool useAnimController, s16 selfId, s16 parentId, bool defaultVisible,
                    bool defaultActive, bool cullFaces, const zeus::CColor& color, EGuiModelDrawFlags drawFlags, bool g,
                    bool h, std::string&& name)
    : x0_frame(frame)
    , x4_useAnimController(useAnimController)
    , x6_selfId(selfId)
    , x8_parentId(parentId)
    , xa_defaultVisible(defaultVisible)
    , xb_defaultActive(defaultActive)
    , xc_cullFaces(cullFaces)
    , xd_g(g)
    , xe_h(h)
    , x10_color(color)
    , x14_drawFlags(drawFlags)
    , m_name(std::move(name)) {}
  };

protected:
  s16 x70_selfId;
  s16 x72_parentId;
  zeus::CTransform m_initTransform;
  zeus::CTransform x74_transform;
  zeus::CColor m_initColor;
  zeus::CColor xa4_color;
  zeus::CColor xa8_color2;
  EGuiModelDrawFlags xac_drawFlags;
  CGuiFrame* xb0_frame;
  s16 xb4_workerId = -1;
  bool xb6_24_pg : 1;
  bool xb6_25_isVisible : 1;
  bool xb6_26_isActive : 1;
  bool xb6_27_isSelectable : 1 = true;
  bool xb6_28_eventLock : 1 = false;
  bool xb6_29_cullFaces : 1;
  bool xb6_30_depthGreater : 1 = false;
  bool xb6_31_depthTest : 1 = true;
  bool xb7_24_depthWrite : 1 = false;
  bool xb7_25_ : 1 = true;
  bool m_mouseActive : 1 = false;

  std::optional<boo::SScrollDelta> m_lastScroll;
  boo::SScrollDelta m_integerScroll;

  std::string m_name;

public:
  explicit CGuiWidget(const CGuiWidgetParms& parms);

  static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in);
  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& drawParms) override;
  void Initialize() override;

  virtual void Reset(ETraversalMode mode);
  virtual void ProcessUserInput(const CFinalInput& input);
  virtual void Touch();
  virtual bool GetIsVisible() const;
  virtual bool GetIsActive() const;
  virtual bool GetMouseActive() const;
  virtual FourCC GetWidgetTypeID() const { return FOURCC('BWIG'); }
  virtual bool AddWorkerWidget(CGuiWidget* worker);
  virtual bool GetIsFinishedLoadingWidgetSpecific();
  virtual void OnVisibleChange();
  virtual void OnActiveChange();

  s16 GetSelfId() const { return x70_selfId; }
  s16 GetParentId() const { return x72_parentId; }
  s16 GetWorkerId() const { return xb4_workerId; }
  const zeus::CTransform& GetTransform() const { return x74_transform; }
  zeus::CTransform& GetTransform() { return x74_transform; }
  const zeus::CVector3f& GetIdlePosition() const { return x74_transform.origin; }
  void SetTransform(const zeus::CTransform& xf);
  const zeus::CColor& GetIntermediateColor() const { return xa4_color; }
  const zeus::CColor& GetGeometryColor() const { return xa8_color2; }
  void SetIdlePosition(const zeus::CVector3f& pos, bool reapply);
  void ReapplyXform();
  virtual void SetIsVisible(bool visible);
  void SetIsActive(bool active);
  bool GetIsSelectable() const { return xb6_27_isSelectable; }
  void SetIsSelectable(bool selectable) { xb6_27_isSelectable = selectable; }
  void SetMouseActive(bool mouseActive) { m_mouseActive = mouseActive; }

  void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms);
  void AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd);
  void SetVisibility(bool visible, ETraversalMode mode);
  void RecalcWidgetColor(ETraversalMode mode);
  void SetColor(const zeus::CColor& color);
  void InitializeRGBAFactor();
  CGuiWidget* FindWidget(s16 id);
  bool GetIsFinishedLoading();
  void DispatchInitialize();
  void SetDepthGreater(bool depthGreater) { xb6_30_depthGreater = depthGreater; }
  void SetDepthTest(bool depthTest) { xb6_31_depthTest = depthTest; }
  void SetDepthWrite(bool depthWrite) { xb7_24_depthWrite = depthWrite; }

  CGuiFrame* GetGuiFrame() const { return xb0_frame; }
};

} // namespace urde
