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
    x4_positions.clear();
    x24_.clear();
    x34_directions.clear();
    if (size != 0)
    {
        x4_positions.reserve(size);
        x24_.reserve(size);
        x34_directions.reserve(size);
    }
}

void CCameraSpline::AddKnot(const zeus::CVector3f& pos, const zeus::CVector3f& dir)
{
    x4_positions.push_back(pos);
    x34_directions.push_back(dir);
}

void CCameraSpline::SetKnotPosition(int idx, const zeus::CVector3f& pos)
{
    if (idx >= x4_positions.size())
        return;
    x4_positions[idx] = pos;
}

float CCameraSpline::CalculateSplineLength()
{
    return 0.f;
}
}
