#ifndef __URDE_CWORLDTRANSMANAGER_HPP__
#define __URDE_CWORLDTRANSMANAGER_HPP__

#include "RetroTypes.hpp"
#include "CRandom16.hpp"
#include "Character/CModelData.hpp"
#include "GuiSys/CGuiTextSupport.hpp"

namespace urde
{
class CSimplePool;
class CStringTable;

class CWorldTransManager
{
public:
    enum class ETransType
    {
        Blank,
        LiftScene,
        Text
    };

    struct CWorldTransModelData
    {
        CModelData x1c_;
        CModelData x68_;
        CModelData xb4_;
        CModelData x100_;
        CToken x14c_;
        CToken x158_;
        CToken x164_;
        std::vector<u32> x1a0_;
        std::unique_ptr<u8> x1b0_;
    };

private:
    float x0_ = 0.f;
    std::unique_ptr<CWorldTransModelData> x4_modelData;
    std::unique_ptr<CGuiTextSupport> x8_textData;
    TLockedToken<CStringTable> xc_strTable;
    u8 x14_ = 0;
    float x18_;
    CRandom16 x20_ = CRandom16(99);
    u16 x24_ = 1189;
    u32 x28_ = 0;
    u8 x2c_ = 127;
    u8 x2d_ = 64;
    ETransType x30_type = ETransType::Blank;
    float x38_ = 0.f;
    bool x40_;
    union
    {
        struct
        {
            bool x44_24_ : 1;
            bool x44_25_ : 1;
            bool x44_26_ : 1;
            bool x44_27_ : 1;
            bool x44_28_ : 1;
        };
        u8 dummy = 0;
    };
public:
    CWorldTransManager() : x44_24_(true) {}

    void DrawFirstPass() const {}
    void DrawSecondPass() const {}
    void DrawAllModels() const {}
    void UpdateLights(float) {}
    void UpdateEnabled(float) {}
    void UpdateDisabled(float) {}
    void Update(float) {}
    void DrawEnabled() const;
    void DrawDisabled() const;
    void sub_80209280() const {}
    void Draw() const
    {
        if (x30_type == ETransType::Blank)
            DrawDisabled();
        else if (x30_type == ETransType::LiftScene)
            DrawEnabled();
        else if (x30_type == ETransType::Text)
            sub_80209280();
    }

    void StartTransition()
    {
        x0_ = 0.f;
        x18_ = 0.f;
        x44_24_ = false;
        x44_28_ = true;
    }

    void StartTextTransition(ResId fontId, ResId stringId, bool, bool, float, float, float);

    void EndTransition() {}
    void PleaseStopSoon() {}
    bool IsTransitionEnabled() const { return false; }
    void DisableTransition() {}
    void TouchModels() {}
    void EnableTransition() {}
};

}

#endif // __URDE_CWORLDTRANSMANAGER_HPP__
