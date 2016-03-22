#include "CGuiTableGroup.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"

namespace urde
{

typedef bool(CGuiTableGroup::*FMAF)(CGuiFunctionDef* def, CGuiControllerInfo* info);
static std::unordered_map<u32, FMAF> WidgetFnMap;

void CGuiTableGroup::LoadWidgetFnMap()
{
    WidgetFnMap.emplace(std::make_pair(21, &CGuiTableGroup::MAF_SelectNextColumn));
    WidgetFnMap.emplace(std::make_pair(22, &CGuiTableGroup::MAF_SelectPrevColumn));
    WidgetFnMap.emplace(std::make_pair(23, &CGuiTableGroup::MAF_SelectNextRow));
    WidgetFnMap.emplace(std::make_pair(24, &CGuiTableGroup::MAF_SelectPrevRow));
    WidgetFnMap.emplace(std::make_pair(25, &CGuiTableGroup::MAF_SelectNextColumnSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(26, &CGuiTableGroup::MAF_SelectPrevColumnSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(27, &CGuiTableGroup::MAF_SelectNextRowSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(28, &CGuiTableGroup::MAF_SelectPrevRowSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(29, &CGuiTableGroup::MAF_SelectColumnAtIndex));
    WidgetFnMap.emplace(std::make_pair(30, &CGuiTableGroup::MAF_SelectRowAtIndex));
    WidgetFnMap.emplace(std::make_pair(31, &CGuiTableGroup::MAF_InitializeTable));
    WidgetFnMap.emplace(std::make_pair(32, &CGuiTableGroup::MAF_MenuAdvance));
    WidgetFnMap.emplace(std::make_pair(33, &CGuiTableGroup::MAF_MenuCancel));
}

void CGuiVirtualBox::GetDifference(int a, int b, int& aOut, int& bOut)
{
    if (a < x0_)
    {
        aOut = a - x0_;
    }
    else
    {
        if (a > x0_ + x8_)
            aOut = a - x0_ + x8_;
        else
            aOut = 0;
    }
    if (b < x4_)
    {
        bOut = b - x4_;
    }
    else
    {
        if (b > x4_ + xc_)
            bOut = b - x4_ + xc_;
        else
            bOut = 0;
    }
}

bool CGuiVirtualBox::ShiftHorizontal(int a, bool b)
{
    bool ret = false;
    FillCellsInBox(x24_.get());

    int r31 = x4_ + a;
    if (r31 < 0)
    {
        ret = true;
        if (b)
            x4_ = x14_ - xc_;
        else
            x4_ = 0;
    }
    else
    {
        if (r31 + xc_ > x14_)
        {
            ret = true;
            if (b)
                x4_ = 0;
            else
                x4_ = x14_ - xc_;
        }
        else
            x4_ = r31;
    }

    FillCellsInBox(x20_.get());
    ResolveTransitionCells();
    return ret;
}

bool CGuiVirtualBox::ShiftVertical(int a, bool b)
{
    bool ret = false;
    FillCellsInBox(x24_.get());

    int r31 = x0_ + a;
    if (r31 < 0)
    {
        ret = true;
        if (b)
            x0_ = x10_ - x8_;
        else
            x0_ = 0;
    }
    else
    {
        if (r31 + x8_ > x10_)
        {
            ret = true;
            if (b)
                x0_ = 0;
            else
                x0_ = x10_ - x8_;
        }
        else
            x0_ = r31;
    }

    FillCellsInBox(x20_.get());
    ResolveTransitionCells();
    return ret;
}

void CGuiVirtualBox::ResolveTransitionCells()
{
    int prod = x8_ * xc_;
    x1c_ = 0;

    for (int i=0 ; i<prod ; ++i)
        x20_[i].x8_ = 0;

    for (int i=0 ; i<prod ; ++i)
    {
        CCellPos& c1 = x24_[i];
        bool needsAdd = true;
        for (int j=0 ; j<prod ; ++j)
        {
            CCellPos& c2 = x20_[i];
            if (c2.x8_)
                continue;
            if (c1.x0_ == c2.x0_ && c1.x4_ == c2.x4_)
            {
                c2.x8_ = true;
                needsAdd = false;
                break;
            }
        }
        if (needsAdd)
            x2c_[x1c_++] = c1;
    }

    for (int i=0, j=0 ; i<prod ; ++i)
    {
        CCellPos& c1 = x20_[i];
        if (c1.x8_)
            continue;
        x28_[j++] = c1;
    }
}

void CGuiVirtualBox::FillCellsInBox(CCellPos* cells)
{
    for (int i=0 ; i<x8_ ; ++i)
    {
        int val = x0_ + i;
        for (int j=0 ; j<xc_ ; ++j)
        {
            CCellPos& cell = cells[i*xc_+j];
            cell.x0_ = val;
            cell.x4_ = x4_ + j;
        }
    }
}

CGuiTableGroup::CGuiTableGroup(const CGuiWidgetParms& parms, int a, int b, ETableGroupModes modes,
                               int c, int d, bool e, bool f, float g, float h, bool i, float j,
                               CGuiVirtualBox&& box)
: CGuiCompoundWidget(parms),
  xf8_(a), xfc_(b),
  x118_(f), x119_(e),
  x11c_(h), x120_(g),
  x124_(i), x128_(j),
  x130_modes(modes),
  x134_box(std::move(box))
{
    x100_[0][0] = c;
    x100_[0][1] = d;
    x100_[1][0] = c;
    x100_[1][1] = d;
    x100_[2][0] = c;
    x100_[2][1] = d;
}

bool CGuiTableGroup::MAF_SelectNextColumn(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectPrevColumn(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectNextRow(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectPrevRow(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectNextColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectPrevColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectNextRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectPrevRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectColumnAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_SelectRowAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_InitializeTable(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_MenuAdvance(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

bool CGuiTableGroup::MAF_MenuCancel(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
    return false;
}

CGuiTableGroup* CGuiTableGroup::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    int a = in.readInt16Big();
    int b = in.readInt16Big();
    ETableGroupModes modes = ETableGroupModes(in.readUint32Big());
    int c = in.readInt16Big();
    int d = in.readInt16Big();
    bool e = in.readBool();
    bool f = in.readBool();
    float g = in.readFloatBig();
    float h = in.readFloatBig();
    bool i = in.readBool();
    float j = in.readFloatBig();

    int ba = in.readInt16Big();
    int bb = in.readInt16Big();
    int bc = in.readInt16Big();
    int bd = in.readInt16Big();

    CGuiVirtualBox box(ba, bb, bc, bd, a, b);

    CGuiTableGroup* ret = new CGuiTableGroup(parms, a, b, modes, c, d, e,
                                             f, g, h, i, j, std::move(box));
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
