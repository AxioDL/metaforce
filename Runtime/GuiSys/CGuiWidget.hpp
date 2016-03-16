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
class CGuiTextSupport;

enum class EGuiAnimBehListID
{
    NegOne = -1,
    Zero = 0,
    One = 1,
    Two = 2,
    EGuiAnimBehListIDMAX = 13
};

enum class ETraversalMode
{
    ChildrenAndSiblings = 0,
    Children = 1,
    Single = 2
};

enum class EGuiAnimInitMode
{
    One = 1,
    Two = 2,
    Three = 3,
    Five = 5
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
        bool x4_useAnimController;
        s16 x6_selfId;
        s16 x8_parentId;
        bool xa_defaultVisible;
        bool xb_defaultActive;
        bool xc_f;
        bool xd_g;
        bool xe_h;
        zeus::CColor x10_color;
        EGuiModelDrawFlags x14_drawFlags;
        CGuiWidgetParms(CGuiFrame* frame, bool useAnimController, s16 selfId, s16 parentId,
                        bool defaultVisible, bool defaultActive,
                        bool f, const zeus::CColor& color, EGuiModelDrawFlags drawFlags, bool g, bool h)
        : x0_frame(frame), x4_useAnimController(useAnimController), x6_selfId(selfId),
          x8_parentId(parentId), xa_defaultVisible(defaultVisible), xb_defaultActive(defaultActive),
          xc_f(f), xd_g(g), xe_h(h), x10_color(color), x14_drawFlags(drawFlags) {}
    };
    static void LoadWidgetFnMap();
    virtual FourCC GetWidgetTypeID() const {return FOURCC('BWIG');}
protected:
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
    std::unordered_map<int, std::unique_ptr<std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>>> xcc_triggerMap;
    std::unordered_map<int, std::unique_ptr<std::vector<std::unique_ptr<CGuiFunctionDef>>>> xe0_functionMap;
    s16 xf4_workerId = -1;
    bool xf6_24_pg : 1;
    bool xf6_25_isVisible : 1;
    bool xf6_26_isActive : 1;
    bool xf6_27_ : 1;
    bool xf6_28_eventLock : 1;
    bool xf6_29_pf : 1;
    bool xf6_30_ : 1;
    bool xf6_31_ : 1;
    bool xf7_24_ : 1;
    bool xf7_25_ : 1;

public:
    CGuiWidget(const CGuiWidgetParms& parms);

    static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in, bool);
    static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, bool);

    virtual bool Message(const CGuiMessage& msg);
    virtual void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms);
    virtual void ParseMessages(CInputStream& in, const CGuiWidgetParms& parms);
    virtual void ParseAnimations(CInputStream& in, const CGuiWidgetParms& parms);
    virtual std::vector<TResId> GetTextureAssets() const;
    virtual std::vector<TResId> GetModelAssets() const;
    virtual std::vector<TResId> GetFontAssets() const;
    virtual void Initialize();
    virtual void Touch() const;
    virtual bool GetIsVisible() const;
    virtual bool GetIsActive() const;
    virtual CGuiTextSupport* TextSupport();
    virtual CGuiTextSupport* GetTextSupport() const;
    virtual void ModifyRGBA(CGuiWidget* widget);
    virtual void AddAnim(EGuiAnimBehListID, CGuiAnimBase*);
    virtual void AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd);
    virtual CGuiWidget* RemoveChildWidget(CGuiWidget* widget, bool makeWorldLocal);
    virtual bool AddWorkerWidget(CGuiWidget* worker);
    virtual bool GetIsFinishedLoadingWidgetSpecific() const;
    virtual void OnVisible();
    virtual void OnInvisible();
    virtual void OnActivate(bool);
    virtual void OnDeActivate();
    virtual bool DoRegisterEventHandler();
    virtual bool DoUnregisterEventHandler();

    s16 GetSelfId() const {return x7c_selfId;}
    s16 GetParentId() const {return x7e_parentId;}
    s16 GetWorkerId() const {return xf4_workerId;}
    const zeus::CTransform& GetTransform() const {return x80_transform;}
    std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>* FindTriggerList(int id);
    void AddTrigger(std::unique_ptr<CGuiLogicalEventTrigger>&& trigger);
    std::vector<std::unique_ptr<CGuiFunctionDef>>* FindFunctionDefList(int id);
    void AddFunctionDef(s32 id, std::unique_ptr<CGuiFunctionDef>&& def);
    const zeus::CVector3f& GetIdlePosition() const {return x80_transform.m_origin;}
    void SetIdlePosition(const zeus::CVector3f& pos, bool reapply);
    void ReapplyXform();
    void SetIsVisible(bool);
    void SetIsActive(bool, bool);
    void EnsureHasAnimController();

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
