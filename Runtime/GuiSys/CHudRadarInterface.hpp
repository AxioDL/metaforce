#ifndef __URDE_CHUDRADARINTERFACE_HPP__
#define __URDE_CHUDRADARINTERFACE_HPP__

#include "CToken.hpp"
#include "Graphics/CTexture.hpp"
#include "Graphics/Shaders/CRadarPaintShader.hpp"

namespace urde
{
class CGuiFrame;
class CGuiWidget;
class CGuiCamera;
class CStateManager;

class CHudRadarInterface
{
    struct SRadarPaintDrawParms
    {
        zeus::CVector3f x0_playerPos;
        zeus::CTransform xc_preTranslate;
        zeus::CTransform x3c_postTranslate;
        float x6c_scopeRadius;
        float x70_scopeScalar;
        float x74_alpha;
        float x78_xyRadius;
        float x7c_zRadius;
        float x80_ZCloseRadius;
    };
    TLockedToken<CTexture> x0_txtrRadarPaint;
    zeus::CTransform xc_radarStuffXf;
    bool x3c_24_visibleGame : 1;
    bool x3c_25_visibleDebug : 1;
    CGuiWidget* x40_BaseWidget_RadarStuff;
    CGuiCamera* x44_camera;
    CRadarPaintShader m_paintShader;
    std::vector<CRadarPaintShader::Instance> m_paintInsts;
    void DoDrawRadarPaint(float radius, const zeus::CColor& color) const;
    void DrawRadarPaint(const zeus::CVector3f& enemyPos, float radius,
                        float alpha, const SRadarPaintDrawParms& parms) const;
public:
    CHudRadarInterface(CGuiFrame& baseHud, CStateManager& stateMgr);
    void SetIsVisibleGame(bool v);
    void Update(float dt, const CStateManager& mgr);
    void Draw(const CStateManager& mgr, float alpha) const;
};

}

#endif // __URDE_CHUDRADARINTERFACE_HPP__
