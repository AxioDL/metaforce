#include "Camera/CCameraSpline.hpp"
#include "CStateManager.hpp"
#include "World/CScriptCameraWaypoint.hpp"
#include "TCastTo.hpp"

namespace urde
{
CCameraSpline::CCameraSpline(bool b) : x48_(b) {}

void CCameraSpline::CalculateKnots(TUniqueId cameraId, const std::vector<SConnection>& connections, CStateManager& mgr)
{
    const SConnection* lastConn = nullptr;

    for (const SConnection& conn : connections)
    {
        if (conn.x0_state == EScriptObjectState::CameraPath && conn.x4_msg == EScriptObjectMessage::Follow)
            lastConn = &conn;
    }

    if (lastConn)
    {
        TCastToPtr<CScriptCameraWaypoint> waypoint = mgr.ObjectById(mgr.GetIdForScript(lastConn->x8_objId));
        //if (waypoint)
    }
}

void CCameraSpline::Reset(int size)
{
    x4_.clear();
    x24_.clear();
    x34_.clear();
    if (size != 0)
    {
        x4_.reserve(size);
        x24_.reserve(size);
        x34_.reserve(size);
    }
}

void CCameraSpline::AddKnot(const zeus::CVector3f& v0, const zeus::CVector3f& v1)
{
    x4_.push_back(v0);
    x34_.push_back(v1);
}

float CCameraSpline::CalculateSplineLength()
{
    return 0.f;
}
}
