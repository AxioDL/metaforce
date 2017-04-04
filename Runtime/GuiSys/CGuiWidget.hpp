#ifndef __URDE_CGUIWIDGET_HPP__
#define __URDE_CGUIWIDGET_HPP__

#include "IOStreams.hpp"
#include "CGuiObject.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
class CGuiFrame;
class CGuiTextSupport;
class CFinalInput;
class CSimplePool;

enum class ETraversalMode
{
    ChildrenAndSiblings = 0,
    Children = 1,
    Single = 2
};

enum class EGuiTextureClampModeHorz
{
    NoClamp = 0,
    Right = 1,
    Left = 2,
    Center = 3
};

enum class EGuiTextureClampModeVert
{
    NoClamp = 0,
    Top = 1,
    Bottom = 2,
    Center = 3
};

class CGuiWidget : public CGuiObject
{
    friend class CGuiFrame;
public:
    enum class EGuiModelDrawFlags
    {
        Shadeless = 0,
        Opaque = 1,
        Alpha = 2,
        Additive = 3,
        AlphaAdditiveOverdraw = 4
    };
    struct CGuiWidgetParms
    {
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
        CGuiWidgetParms(CGuiFrame* frame, bool useAnimController, s16 selfId, s16 parentId,
                        bool defaultVisible, bool defaultActive, bool cullFaces,
                        const zeus::CColor& color, EGuiModelDrawFlags drawFlags,
                        bool g, bool h)
        : x0_frame(frame), x4_useAnimController(useAnimController), x6_selfId(selfId),
          x8_parentId(parentId), xa_defaultVisible(defaultVisible), xb_defaultActive(defaultActive),
          xc_cullFaces(cullFaces), xd_g(g), xe_h(h), x10_color(color), x14_drawFlags(drawFlags) {}
    };
protected:
    s16 x70_selfId;
    s16 x72_parentId;
    zeus::CTransform x74_transform;
    zeus::CColor xa4_color;
    zeus::CColor xa8_color2;
    EGuiModelDrawFlags xac_drawFlags;
    CGuiFrame* xb0_frame;
    s16 xb4_workerId = -1;
    bool xb6_24_pg : 1;
    bool xb6_25_isVisible : 1;
    bool xb6_26_isActive : 1;
    bool xb6_27_isSelectable : 1;
    bool xb6_28_eventLock : 1;
    bool xb6_29_cullFaces : 1;
    bool xb6_30_ : 1;
    bool xb6_31_depthTest : 1;
    bool xb7_24_depthWrite : 1;
    bool xb7_25_ : 1;

public:
    CGuiWidget(const CGuiWidgetParms& parms);

    static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in);
    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    virtual void Update(float dt);
    virtual void Draw(const CGuiWidgetDrawParms& drawParms) const;
    virtual void Initialize();
    virtual void ProcessUserInput(const CFinalInput& input);
    virtual void Touch() const;
    virtual bool GetIsVisible() const;
    virtual bool GetIsActive() const;
    virtual FourCC GetWidgetTypeID() const { return FOURCC('BWIG'); }
    virtual bool AddWorkerWidget(CGuiWidget* worker);
    virtual bool GetIsFinishedLoadingWidgetSpecific() const;
    virtual void OnVisibleChange();
    virtual void OnActiveChange();

    s16 GetSelfId() const {return x70_selfId;}
    s16 GetParentId() const {return x72_parentId;}
    s16 GetWorkerId() const {return xb4_workerId;}
    const zeus::CTransform& GetTransform() const {return x74_transform;}
    zeus::CTransform& GetTransform() {return x74_transform;}
    const zeus::CVector3f& GetIdlePosition() const {return x74_transform.origin;}
    void SetTransform(const zeus::CTransform& xf);
    const zeus::CColor& GetGeometryColor() const { return xa8_color2; }
    void SetIdlePosition(const zeus::CVector3f& pos, bool reapply);
    void ReapplyXform();
    void SetIsVisible(bool);
    void SetIsActive(bool);
    bool GetIsSelectable() const { return xb6_27_isSelectable; }
    void SetIsSelectable(bool v) { xb6_27_isSelectable = v; }

    void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms);
    void AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd);
    void SetVisibility(bool, ETraversalMode);
    void RecalcWidgetColor(ETraversalMode);
    void SetColor(const zeus::CColor& color);
    void InitializeRGBAFactor();
    CGuiWidget* FindWidget(s16 id);
    bool GetIsFinishedLoading() const;
    void DispatchInitialize();
    void SetDepthTest(bool v) { xb6_31_depthTest = v; }
    void SetDepthWrite(bool v) { xb7_24_depthWrite = v; }

    CGuiFrame* GetGuiFrame() const { return xb0_frame; }
};

}

#endif // __URDE_CGUIWIDGET_HPP__
