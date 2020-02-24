#pragma once

#include <vector>

#include <athena/Types.hpp>

namespace DataSpec::DNAMP1 {
struct IScriptObject;

struct ScriptObjectSpec {
  atUint8 type;
  IScriptObject* (*a)();
};

extern const std::vector<const struct ScriptObjectSpec*> SCRIPT_OBJECT_DB;
} // namespace DataSpec::DNAMP1
