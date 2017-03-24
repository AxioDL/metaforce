#include "CSamusHud.hpp"

namespace urde
{
namespace MP1
{

CSamusHud* CSamusHud::g_SamusHud = nullptr;

CSamusHud::CSamusHud(CStateManager& stateMgr)
{

}

bool CSamusHud::CheckLoadComplete(CStateManager& stateMgr)
{
    return false;
}

void CSamusHud::OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr)
{

}

void CSamusHud::Touch()
{

}

void CSamusHud::DisplayHudMemo(const std::u16string& text, const SHudMemoInfo& info)
{

}

void CSamusHud::_DeferHintMemo(ResId strg, u32 timePeriods, const SHudMemoInfo& info)
{

}

}
}
