#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"

namespace metaforce {
struct SConnection {
  EScriptObjectState x0_state = EScriptObjectState::Any;
  EScriptObjectMessage x4_msg = EScriptObjectMessage::None;
  TEditorId x8_objId = kInvalidEditorId;
  bool operator==(const SConnection& other) const {
    return x0_state == other.x0_state && x4_msg == other.x4_msg && x8_objId == other.x8_objId;
  }
  bool operator<(const SConnection& other) const { return x8_objId < other.x8_objId; }
};

class CEntityInfo {
  TAreaId x0_areaId;
  std::vector<SConnection> x4_conns;
  TEditorId x14_editorId;

public:
  CEntityInfo(TAreaId aid, std::vector<SConnection> conns, TEditorId eid = kInvalidEditorId)
  : x0_areaId(aid), x4_conns(std::move(conns)), x14_editorId(eid) {}
  TAreaId GetAreaId() const { return x0_areaId; }
  std::vector<SConnection> GetConnectionList() const { return x4_conns; }
  TEditorId GetEditorId() const { return x14_editorId; }
};
} // namespace metaforce
