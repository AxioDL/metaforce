#pragma once

#include <unordered_map>

#include "Runtime/IFactory.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde {
struct SObjectTag;
class CVParamTransfer;
class IObj;

class CFactoryMgr {
  std::unordered_map<FourCC, FFactoryFunc> m_factories;
  std::unordered_map<FourCC, FMemFactoryFunc> m_memFactories;

public:
  CFactoryFnReturn MakeObject(const SObjectTag& tag, urde::CInputStream& in, const CVParamTransfer& paramXfer,
                              CObjectReference* selfRef);
  bool CanMakeMemory(const urde::SObjectTag& tag) const;
  CFactoryFnReturn MakeObjectFromMemory(const SObjectTag& tag, std::unique_ptr<u8[]>&& buf, int size, bool compressed,
                                        const CVParamTransfer& paramXfer, CObjectReference* selfRef);
  void AddFactory(FourCC key, FFactoryFunc func) { m_factories[key] = func; }
  void AddFactory(FourCC key, FMemFactoryFunc func) { m_memFactories[key] = func; }

  enum class ETypeTable : u8 {
    CLSN,
    CMDL,
    CSKR,
    ANIM,
    CINF,
    TXTR,
    PLTT,
    FONT,
    ANCS,
    EVNT,
    MADF,
    MLVL,
    MREA,
    MAPW,
    MAPA,
    SAVW,
    SAVA,
    PART,
    WPSC,
    SWHC,
    DPSC,
    ELSC,
    CRSC,
    AFSM,
    DCLN,
    AGSC,
    ATBL,
    CSNG,
    STRG,
    SCAN,
    PATH,
    DGRP,
    HMAP,
    CTWK,
    FRME,
    HINT,
    MAPU,
    DUMB,
    OIDS,
    Invalid = 127
  };

  static ETypeTable FourCCToTypeIdx(FourCC fcc);
  static FourCC TypeIdxToFourCC(ETypeTable fcc);
};

} // namespace urde
