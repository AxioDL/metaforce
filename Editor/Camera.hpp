#pragma once

#include "zeus/CProjection.hpp"
#include "zeus/CFrustum.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/Math.hpp"

namespace urde
{
class Camera
{
    zeus::CFrustum    m_frustum;
    zeus::CProjection m_projection;
    zeus::CVector3f   m_position;
    zeus::CQuaternion m_orientation;
public:

    void setPosition(const zeus::CVector3f& position) { m_position = position; }
    void setOrientation(const zeus::CQuaternion& orientation) { m_orientation = orientation; }

    const zeus::CMatrix4f& projectionMatrix() const { return m_projection.getCachedMatrix(); }
    const zeus::CProjection& projection() const { return m_projection; }

    virtual void think()
    {}
};
}


