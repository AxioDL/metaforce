#pragma once

#include "IOStreams.hpp"

namespace urde
{

class CHUDMemoParms
{
    float x0_dispTime = 0.f;
    bool x4_clearMemoWindow = false;
    bool x5_fadeOutOnly = false;
    bool x6_hintMemo = false;

public:
    CHUDMemoParms() = default;
    CHUDMemoParms(float dispTime, bool clearMemoWindow, bool fadeOutOnly, bool hintMemo)
        : x0_dispTime(dispTime), x4_clearMemoWindow(clearMemoWindow),
          x5_fadeOutOnly(fadeOutOnly), x6_hintMemo(hintMemo) {}
    CHUDMemoParms(CInputStream& in)
    {
        x0_dispTime = in.readFloatBig();
        x4_clearMemoWindow = in.readBool();
    }

    float GetDisplayTime() const { return x0_dispTime; }
    bool IsClearMemoWindow() const { return x4_clearMemoWindow; }
    bool IsFadeOutOnly() const { return x5_fadeOutOnly; }
    bool IsHintMemo() const { return x6_hintMemo; }
};

}

