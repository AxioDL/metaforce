#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

#include <musyx/musyx.h>

namespace metaforce {

class CAudioGroupSet {
  std::unique_ptr<u8[]> m_buffer;
  std::string x10_baseName;
  std::string x20_name;

  // Parsed section pointers (set by LoadData)
  u8* m_pool = nullptr;
  u32 m_poolLen = 0;
  u8* m_proj = nullptr;
  u32 m_projLen = 0;
  u8* m_samp = nullptr;
  u32 m_sampLen = 0;
  u8* m_sdir = nullptr;
  u32 m_sdirLen = 0;

  // Converted SDIR data for 64-bit (owns the allocation)
  std::vector<u8> m_convertedSdir;

  void LoadData();
  void SwapProjectEndian();
  void SwapPoolEndian();
  void SwapSdirEndian();
  void SwapPCM16Samples();
  void ConvertSdirTo64Bit();

public:
  explicit CAudioGroupSet(std::unique_ptr<u8[]>&& in);
  std::string_view GetName() const { return x20_name; }

  u8* GetPool() const { return m_pool; }
  u32 GetPoolLen() const { return m_poolLen; }
  u8* GetProj() const { return m_proj; }
  u32 GetProjLen() const { return m_projLen; }
  u8* GetSamp() const { return m_samp; }
  u32 GetSampLen() const { return m_sampLen; }
  u8* GetSdir() const { return m_sdir; }
  u32 GetSdirLen() const { return m_sdirLen; }
};

CFactoryFnReturn FAudioGroupSetDataFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                           const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef);

} // namespace metaforce
