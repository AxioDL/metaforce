#include "CFaceplateDecoration.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{
namespace MP1
{

CFaceplateDecoration::CFaceplateDecoration(CStateManager& stateMgr) {}

void CFaceplateDecoration::Update(float dt, CStateManager& stateMgr)
{
    ResId txtrId = stateMgr.GetPlayer().GetVisorSteam().GetTextureId();
    if (txtrId == -1)
    {
        if (xc_ready)
        {
            x4_tex.Unlock();
            x0_id = txtrId;
            if (m_texFilter)
                m_texFilter = std::experimental::nullopt;
        }
    }

    if (x0_id != txtrId && txtrId != -1)
    {
        if (m_texFilter)
            m_texFilter = std::experimental::nullopt;
        x0_id = txtrId;
        x4_tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), txtrId});
        xc_ready = true;
        x4_tex.Lock();
    }

    if (!m_texFilter && x4_tex.IsLoaded())
        m_texFilter.emplace(CCameraFilterPass::EFilterType::Blend, x4_tex);
}

void CFaceplateDecoration::Draw(CStateManager& stateMgr)
{
    if (xc_ready && m_texFilter)
    {
        zeus::CColor color = zeus::CColor::skWhite;
        color.a = stateMgr.GetPlayer().GetVisorSteam().GetAlpha();
        m_texFilter->draw(color, 1.f);
    }
}

}
}
