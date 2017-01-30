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
}
