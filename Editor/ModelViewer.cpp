#include "ModelViewer.hpp"

namespace URDE
{

void ModelViewer::View::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    Specter::View::resized(root, sub);
    m_scissorRect = sub;
}

void ModelViewer::View::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    gfxQ->setScissor(m_scissorRect);
    m_mv.m_lineRenderer->Reset();
    m_mv.m_lineRenderer->AddVertex({-0.5f, 0.f, -0.5f}, Zeus::CColor::skBlue, 1.f);
    m_mv.m_lineRenderer->AddVertex({-0.5f, 0.f, 0.5f}, Zeus::CColor::skBlue, 1.f);
    m_mv.m_lineRenderer->AddVertex({0.5f, 10.f, 0.5f}, Zeus::CColor::skRed, 3.f);
    m_mv.m_lineRenderer->AddVertex({0.5f, 0.f, -0.5f}, Zeus::CColor::skBlue, 1.f);
    m_mv.m_lineRenderer->Render();
    gfxQ->setScissor({});
}


}
