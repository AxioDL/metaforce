#ifndef __URDE_CGUIANIMCONTROLLER_HPP__
#define __URDE_CGUIANIMCONTROLLER_HPP__

#include "CGuiWidget.hpp"
#include <array>

namespace urde
{

class CGuiAnimSet
{
    friend class CGuiAnimController;
    std::unordered_map<int, CGuiAnimBase*> x0_map;
    EGuiAnimBehListID x14_id;
    bool x18_update = false;
    bool x19_isDone = true;
    bool x1a_ = false;
    bool x1b_ = false;
    bool x1c_ = false;
    u32 x20_ = 0;
    u32 x24_ = 0;
    float x28_totalTime = 0.f;
    float x2c_ = 1.f;
public:
    CGuiAnimSet(EGuiAnimBehListID id) : x14_id(id) {}
    void AddAnim(CGuiAnimBase* anim);
    void RecalcTotalTime();
    void Init(CGuiAnimController* controller, float fval);
    void Update(CGuiAnimController* controller, bool flag);
};

class CGuiAnimController : public CGuiWidget
{
    friend class CGuiAnimSet;
    std::array<std::unique_ptr<CGuiAnimSet>, 13> xf8_sets;
    float x164_ = 0.f;
    CGuiWidget* x168_widget;
    zeus::CColor x16c_ = zeus::CColor::skClear;
    u32 x170_ = 0;

public:
    CGuiAnimController(const CGuiWidget::CGuiWidgetParms& parms, CGuiWidget* widget);
    CGuiAnimSet* FindAnimSet(EGuiAnimBehListID id) const;
    void AddAnimation(CGuiAnimBase* anim, EGuiAnimBehListID id);
    void ResetListUpdateState();
    void SetListUpdateState(EGuiAnimBehListID id, bool state);
    float GetAnimSetLength(EGuiAnimBehListID id) const;
    void IsAnimsDone(EGuiAnimBehListID id, bool& isDone) const;
    void InitTransform(CGuiWidget* widget, EGuiAnimBehListID id, float fval, bool flag,
                       EGuiAnimInitMode initMode);
    void FinishAnim();
    void Step(bool flag);
};

}

#endif // __URDE_CGUIANIMCONTROLLER_HPP__
