#include "Runtime/RetroTypes.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
logvisor::Module Log("metaforce::RetroTypes::CAssetId");

SObjectTag::SObjectTag(CInputStream& in) {
  in.ReadBytes(reinterpret_cast<u8*>(&type), 4);
  id = in.Get<CAssetId>();
}

void SObjectTag::ReadMLVL(CInputStream& in) {
  id = in.Get<CAssetId>();
  in.ReadBytes(reinterpret_cast<u8*>(&type), 4);
}

CAssetId::CAssetId(CInputStream& in) {
  if (g_Main != nullptr) {
    if (g_Main->GetExpectedIdSize() == sizeof(u32)) {
      Assign(u32(in.ReadLong()));
    } else if (g_Main->GetExpectedIdSize() == sizeof(u64)) {
      Assign(in.ReadLongLong());
    } else {
      Log.report(logvisor::Fatal, FMT_STRING("Unsupported id length {}"), g_Main->GetExpectedIdSize());
    }
  } else {
    Log.report(logvisor::Fatal, FMT_STRING("Input constructor called before runtime Main entered!"));
  }
}

void CAssetId::PutTo(COutputStream& out) const {
  if (g_Main != nullptr) {
    if (g_Main->GetExpectedIdSize() == sizeof(u32)) {
      out.Put(u32(id));
    } else if (g_Main->GetExpectedIdSize() == sizeof(u64)) {
      out.Put(id);
    } else {
      Log.report(logvisor::Fatal, FMT_STRING("Unsupported id length {}"), g_Main->GetExpectedIdSize());
    }
  } else {
    Log.report(logvisor::Fatal, FMT_STRING("PutTo called before runtime Main entered!"));
  }
}

} // namespace metaforce