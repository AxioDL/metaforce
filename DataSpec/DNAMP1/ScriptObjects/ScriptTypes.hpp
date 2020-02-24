#pragma once

#include <array>

#include <athena/Types.hpp>

namespace DataSpec::DNAMP1 {
struct IScriptObject;

struct ScriptObjectSpec {
  atUint8 type;
  IScriptObject* (*a)();
};

using ScriptObjectDBArray = std::array<const ScriptObjectSpec*, 127>;

extern const ScriptObjectDBArray SCRIPT_OBJECT_DB;
} // namespace DataSpec::DNAMP1
