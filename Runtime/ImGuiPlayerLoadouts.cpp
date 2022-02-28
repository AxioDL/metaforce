#include "Runtime/ImGuiPlayerLoadouts.hpp"
#include "Runtime/Streams/ContainerReaders.hpp"
#include "Runtime/Streams/ContainerWriters.hpp"

#include <logvisor/logvisor.hpp>

#include "magic_enum.hpp"
namespace metaforce {
namespace {
logvisor::Module Log("metaforce::ImGuiPlayerLoadouts");
constexpr u32 CurrentVersion = 1;
} // namespace
ImGuiPlayerLoadouts::Item::Item(CInputStream& in)
: type(magic_enum::enum_cast<CPlayerState::EItemType>(in.Get<std::string>()).value()), amount(in.ReadLong()) {}

void ImGuiPlayerLoadouts::Item::PutTo(COutputStream& out) const {
  out.Put(magic_enum::enum_name<CPlayerState::EItemType>(type));
  out.Put(amount);
}
ImGuiPlayerLoadouts::LoadOut::LoadOut(CInputStream& in) : name(in.Get<std::string>()) { read_vector(items, in); }

void ImGuiPlayerLoadouts::LoadOut::PutTo(COutputStream& out) const {
  out.Put(name);
  write_vector(items, out);
}

ImGuiPlayerLoadouts::ImGuiPlayerLoadouts(CInputStream& in) {
  FourCC magic;
  in.Get(reinterpret_cast<u8*>(&magic), 4);
  auto version = in.ReadLong();
  if (magic != FOURCC('LOAD') && version != CurrentVersion) {
    Log.report(logvisor::Error, FMT_STRING("Incorrect loadout version, expected {} got {}"), CurrentVersion, version);
    return;
  }
  read_vector(loadouts, in);
}
void ImGuiPlayerLoadouts::PutTo(COutputStream& out) const {
  auto magic = FOURCC('LOAD');
  out.Put(reinterpret_cast<const u8*>(&magic), 4);
  out.Put(CurrentVersion);
  write_vector(loadouts, out);
}
} // namespace metaforce