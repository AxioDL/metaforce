#ifndef __URDE_CGUITABLEGROUP_HPP__
#define __URDE_CGUITABLEGROUP_HPP__

#include "CGuiCompoundWidget.hpp"

namespace urde
{

struct CCellPos
{
    u32 x0_ = 0;
    u32 x4_ = 0;
    bool x8_ = false;
};

class CGuiVirtualBox
{
    int x0_;
    int x4_;
    int x8_;
    int xc_;
    int x10_;
    int x14_;
    int x18_;
    int x1c_ = 0;
    std::unique_ptr<CCellPos[]> x20_;
    std::unique_ptr<CCellPos[]> x24_;
    std::unique_ptr<CCellPos[]> x28_;
    std::unique_ptr<CCellPos[]> x2c_;
public:
    CGuiVirtualBox(int a,int b,int c,int d,int e,int f)
    : x0_(a), x4_(b), x8_(c), xc_(d), x10_(e), x14_(f), x18_(std::max(x10_, x14_)),
      x20_(new CCellPos[c*d]),
      x24_(new CCellPos[c*d]),
      x28_(new CCellPos[x18_]),
      x2c_(new CCellPos[x18_]) {}

    void GetDifference(int,int,int&,int&);
    bool ShiftHorizontal(int,bool);
    bool ShiftVertical(int,bool);
    void ResolveTransitionCells();
    void FillCellsInBox(CCellPos* cells);
};

class CGuiTableGroup : public CGuiCompoundWidget
{
public:
    enum class ETableGroupModes
    {
    };
private:
    std::function<void(const CGuiTableGroup*)> xd4_doMenuAdvance;
    std::function<void(const CGuiTableGroup*)> xec_doMenuCancel;
    std::function<void(const CGuiTableGroup*)> x104_doMenuSelChange;
    int xf8_;
    int xfc_;
    int x100_[3][2];
    bool x118_;
    bool x119_;
    float x11c_;
    float x120_;
    bool x124_;
    float x128_;
    bool x12c_ = false;
    ETableGroupModes x130_modes;
    CGuiVirtualBox x134_box;
public:
    CGuiTableGroup(const CGuiWidgetParms& parms, int, int, ETableGroupModes modes,
                   int, int, bool, bool, float, float, bool, float, CGuiVirtualBox&& box);
    FourCC GetWidgetTypeID() const {return FOURCC('TBGP');}
    static void LoadWidgetFnMap();

    bool MAF_SelectNextColumn(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevColumn(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectNextRow(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevRow(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectNextColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectNextRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectColumnAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectRowAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_InitializeTable(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_MenuAdvance(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_MenuCancel(CGuiFunctionDef* def, CGuiControllerInfo* info);

    void SetMenuAdvanceCallback(std::function<void(const CGuiTableGroup*)>&& cb)
    {
        xd4_doMenuAdvance = std::move(cb);
    }

    void SetMenuCancelCallback(std::function<void(const CGuiTableGroup*)>&& cb)
    {
        xec_doMenuCancel = std::move(cb);
    }

    void SetMenuSelectionChangeCallback(std::function<void(const CGuiTableGroup*)>&& cb)
    {
        x104_doMenuSelChange = std::move(cb);
    }

    static CGuiTableGroup* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUITABLEGROUP_HPP__
