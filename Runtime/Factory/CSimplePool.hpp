#pragma once

#include <unordered_map>
#include <vector>

#include "Runtime/Factory/IObjectStore.hpp"
#include "Runtime/Factory/IVParamObj.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CObjectReference;
class IFactory;

class CSimplePool : public IObjectStore {
protected:
  u8 x4_;
  u8 x5_;
  std::unordered_map<SObjectTag, CObjectReference*> x8_resources;
  IFactory& x18_factory;
  CVParamTransfer x1c_paramXfer;

public:
  CSimplePool(IFactory& factory);
  ~CSimplePool() override;
  CToken GetObj(const SObjectTag&, const CVParamTransfer&) override;
  CToken GetObj(const SObjectTag&) override;
  CToken GetObj(std::string_view) override;
  CToken GetObj(std::string_view, const CVParamTransfer&) override;
  bool HasObject(const SObjectTag&) const override;
  bool ObjectIsLive(const SObjectTag&) const override;
  IFactory& GetFactory() const override { return x18_factory; }
  void Flush() override;
  void ObjectUnreferenced(const SObjectTag&) override;
  std::vector<SObjectTag> GetReferencedTags() const;
  size_t GetLiveObjects() const { return x8_resources.size(); }
};

} // namespace metaforce
