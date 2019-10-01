#pragma once

#include <memory>
#include <string>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"

#include <amuse/AudioGroupData.hpp>

namespace urde {

class CAudioGroupSet {
  std::unique_ptr<u8[]> m_buffer;
  std::string x10_baseName;
  std::string x20_name;
  amuse::AudioGroupData m_data;
  amuse::AudioGroupData LoadData();

public:
  CAudioGroupSet(std::unique_ptr<u8[]>&& in);
  const amuse::AudioGroupData& GetAudioGroupData() const { return m_data; }
  std::string_view GetName() const { return x20_name; }
};

CFactoryFnReturn FAudioGroupSetDataFactory(const urde::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                           const urde::CVParamTransfer& vparms, CObjectReference* selfRef);

} // namespace urde
