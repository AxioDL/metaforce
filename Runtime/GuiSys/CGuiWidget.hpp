#ifndef __URDE_CGUIWIDGET_HPP__
#define __URDE_CGUIWIDGET_HPP__

#include "IOStreams.hpp"
#include "CGuiObject.hpp"
#include "CGuiFunctionDef.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
class CGuiAnimBase;
class CGuiFrame;
class CGuiMessage;
class CGuiAnimController;
class CGuiLogicalEventTrigger;

enum class EGuiAnimBehListID
{
};

enum class ETraversalMode
{
    Recursive = 1,
    NonRecursive = 2
};

enum class EGuiAnimInitMode
{
};

enum class EGuiTextureClampModeHorz
{
};

enum class EGuiTextureClampModeVert
{
};

class CGuiWidget : public CGuiObject
{
    friend class CGuiFrame;
public:
    enum class EGuiModelDrawFlags
    {
        Two = 2
    };
    struct CGuiWidgetParms
    {
        CGuiFrame* x0_frame;
        bool x4_a;
        s16 x6_selfId;
        s16 x8_parentId;
        bool xa_d;
        bool xb_defaultActive;
        bool xc_f;
        bool xd_g;
        bool xe_h;
        zeus::CColor x10_color;
        EGuiModelDrawFlags x14_drawFlags;
        CGuiWidgetParms(CGuiFrame* frame, bool a, s16 selfId, s16 parentId, bool d, bool defaultActive,
                        bool f, const zeus::CColor& color, EGuiModelDrawFlags drawFlags, bool g, bool h)
        : x0_frame(frame), x4_a(a), x6_selfId(selfId), x8_parentId(parentId), xa_d(d),
          xb_defaultActive(defaultActive), xc_f(f), xd_g(g), xe_h(h), x10_color(color),
          x14_drawFlags(drawFlags) {}
    };
    static void LoadWidgetFnMap();
    virtual FourCC GetWidgetTypeID() const {return hecl::FOURCC('BWIG');}
private:
    s16 x7c_selfId;
    s16 x7e_parentId;
    zeus::CTransform x80_transform;
    std::unique_ptr<CGuiAnimController> xb0_animController;
    zeus::CColor xb4_ = zeus::CColor::skWhite;
    zeus::CColor xb8_ = zeus::CColor::skClear;
    zeus::CColor xbc_color;
    zeus::CColor xc0_color2;
    EGuiModelDrawFlags xc4_drawFlags;
    CGuiFrame* xc8_frame;
    std::unordered_map<int, std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>> xcc_functionMap;
    u32 xe4_ = 0;
    u32 xe8_ = 0;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    s16 xf4_workerId = -1;
    bool xf6_24_pg : 1;
    bool xf6_25_pd : 1;
    bool xf6_26_isActive : 1;
    bool xf6_27_ : 1;
    bool xf6_28_ : 1;
    bool xf6_29_pf : 1;
    bool xf6_30_ : 1;
    bool xf6_31_ : 1;
    bool xf7_24_ : 1;
    bool xf7_25_ : 1;

public:
    CGuiWidget(const CGuiWidgetParms& parms);

    static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in, bool);
    static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, bool);

    virtual void Message(const CGuiMessage& msg);
    virtual void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms);
    virtual void ParseMessages(CInputStream& in, const CGuiWidgetParms& parms);
    virtual void ParseAnimations(CInputStream& in, const CGuiWidgetParms& parms);
    virtual void GetTextureAssets() const;
    virtual void GetModelAssets() const;
    virtual void GetFontAssets() const;
    virtual void GetKFAMAssets() const;
    virtual void Initialize();
    virtual void Touch() const;
    virtual void GetIsVisible() const;
    virtual bool GetIsActive() const;
    virtual void TextSupport();
    virtual void GetTextSupport() const;
    virtual void ModifyRGBA(CGuiWidget* widget);
    virtual void AddAnim(EGuiAnimBehListID, CGuiAnimBase*);
    virtual void AddChildWidget(CGuiWidget* widget, bool, bool);
    virtual void RemoveChildWidget(CGuiWidget* widget, bool);
    virtual bool AddWorkerWidget(CGuiWidget* worker);
    virtual void GetFinishedLoadingWidgetSpecific() const;
    virtual void OnVisible();
    virtual void OnInvisible();
    virtual void OnActivate(bool);
    virtual void OnDeActivate();
    virtual bool DoRegisterEventHandler();
    virtual bool DoUnregisterEventHandler();

    void AddFunctionDef(u32, CGuiFunctionDef* def);
    void FindFunctionDefList(int);
    zeus::CVector3f GetIdlePosition() const;
    void SetIdlePosition(const zeus::CVector3f& pos);
    void ReapplyXform();
    void SetIsVisible(bool);
    void SetIsActive(bool, bool);
    void EnsureHasAnimController();
    std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>* FindTriggerList(int);
    void AddTrigger(std::unique_ptr<CGuiLogicalEventTrigger>&& trigger);

    void BroadcastMessage(int, CGuiControllerInfo* info);
    void LockEvents(bool);
    void UnregisterEventHandler();
    void UnregisterEventHandler(ETraversalMode);
    void RegisterEventHandler();
    void RegisterEventHandler(ETraversalMode);
    void ResetAllAnimUpdateState();
    void SetVisibility(bool, ETraversalMode);
    void SetAnimUpdateState(EGuiAnimBehListID, bool);
    void SetAnimUpdateState(EGuiAnimBehListID, bool, ETraversalMode);
    void GetBranchAnimLen(EGuiAnimBehListID, float&);
    void GetBranchAnimLen(EGuiAnimBehListID, float&, ETraversalMode);
    void IsAllAnimsDone(EGuiAnimBehListID, bool&);
    void IsAllAnimsDone(EGuiAnimBehListID, bool&, ETraversalMode);
    void InitializeAnimControllers(EGuiAnimBehListID, float, bool, EGuiAnimInitMode);
    void InitializeAnimControllers(EGuiAnimBehListID, float, bool, EGuiAnimInitMode, ETraversalMode);
    void RecalcWidgetColor(ETraversalMode);
    void SetColor(const zeus::CColor& color);
    void RecalculateAllRGBA();
    void InitializeRGBAFactor();
    CGuiWidget* FindWidget(s16 id);
    bool GetIsFinishedLoading() const;
    void InitializeRecursive();

    bool MAF_StartAnimationSet(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SendMessage(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_PauseAnim(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_ResumeAnim(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SetState(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SetStateOfWidget(CGuiFunctionDef* def, CGuiControllerInfo* info);
};

}

#endif // __URDE_CGUIWIDGET_HPP__
