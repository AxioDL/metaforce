#pragma once

#include "Runtime/CObjectList.hpp"

namespace urde {

class CActorList : public CObjectList {
public:
  CActorList();

  bool IsQualified(const CEntity&) const override;
};

class CPhysicsActorList : public CObjectList {
public:
  CPhysicsActorList();
  bool IsQualified(const CEntity&) const override;
};

class CGameCameraList : public CObjectList {
public:
  CGameCameraList();
  bool IsQualified(const CEntity&) const override;
};

class CListeningAiList : public CObjectList {
public:
  CListeningAiList();
  bool IsQualified(const CEntity&) const override;
};

class CAiWaypointList : public CObjectList {
public:
  CAiWaypointList();
  bool IsQualified(const CEntity&) const override;
};

class CPlatformAndDoorList : public CObjectList {
public:
  CPlatformAndDoorList();

  bool IsQualified(const CEntity&) const override;
  bool IsDoor(const CEntity&) const;
  bool IsPlatform(const CEntity&) const;
};

class CGameLightList : public CObjectList {
public:
  CGameLightList();

  bool IsQualified(const CEntity&) const override;
};

} // namespace urde
