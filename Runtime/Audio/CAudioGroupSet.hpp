#pragma once

#include <memory>
#include <string>

#include "Runtime/Factory/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Factory/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

#include <amuse/AudioGroupData.hpp>

namespace metaforce {

class CAudioGroupSet {
  std::unique_ptr<u8[]> m_buffer;
  std::string x10_baseName;
  std::string x20_name;
  amuse::AudioGroupData m_data;
  amuse::AudioGroupData LoadData();

public:
  explicit CAudioGroupSet(std::unique_ptr<u8[]>&& in);
  const amuse::AudioGroupData& GetAudioGroupData() const { return m_data; }
  std::string_view GetName() const { return x20_name; }
};

CFactoryFnReturn FAudioGroupSetDataFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                           const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef);

} // namespace metaforce
