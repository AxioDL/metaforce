#pragma once

#include <athena/DNA.hpp>
#include <athena/DNAYaml.hpp>

namespace metaforce {
struct ImGuiPlayerLoadouts : athena::io::DNA<athena::Endian::Big> {
  AT_DECL_DNA_YAML
  struct Item : athena::io::DNA<athena::Endian::Big> {
    AT_DECL_DNA_YAML
    String<-1> type;
    Value<atUint32> amount;
  };
  struct LoadOut : athena::io::DNA<athena::Endian::Big> {
    AT_DECL_DNA_YAML
    String<-1> name;
    Value<atUint32> itemCount;
    Vector<Item, AT_DNA_COUNT(itemCount)> items;
  };
  Value<atUint32> loadoutCount;
  Vector<LoadOut, AT_DNA_COUNT(loadoutCount)> loadouts;
};
} // namespace metaforce
