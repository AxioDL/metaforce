#ifndef __URDE_CGUIWIDGET_HPP__
#define __URDE_CGUIWIDGET_HPP__

#include "IOStreams.hpp"
#include "CGuiObject.hpp"

namespace urde
{
class CGuiAnimBase;
class CGuiFrame;
class CGuiMessage;

enum class EGuiAnimBehListID
{
};

class CGuiWidget : public CGuiObject
{
public:
    struct CGuiWidgetParms
    {
    };
    static void LoadWidgetFnMap();
private:
public:
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
    virtual void GetIsActive() const;
    virtual void TextSupport();
    virtual void GetTextSupport() const;
    virtual void ModifyRGBA(CGuiWidget* widget);
    virtual void AddAnim(EGuiAnimBehListID, CGuiAnimBase*);
    virtual void AddChildWidget(CGuiWidget* widget, bool, bool);
    virtual void RemoveChildWidget(CGuiWidget* widget, bool);
    virtual void GetWidgetTypeID() const;
    virtual void AddWorkerWidget(CGuiWidget* worker);
    virtual void GetFinishedLoadingWidgetSpecific() const;
    virtual void OnVisible();
    virtual void OnInvisible();
    virtual void OnActivate(bool);
    virtual void OnDeActivate();
    virtual void DoRegisterEventHandler();
    virtual void DoUnregisterEventHandler();

    bool MAF_StartAnimationSet(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SendMessage(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_PauseAnim(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_ResumeAnim(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SetState(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SetStateOfWidget(CGuiFunctionDef* def, CGuiControllerInfo* info);
};

}

#endif // __URDE_CGUIWIDGET_HPP__
