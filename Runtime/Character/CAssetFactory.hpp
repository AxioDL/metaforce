#pragma once

#include <functional>
#include <memory>

#include "Runtime/Factory/CSimplePool.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Factory/IFactory.hpp"
#include "Runtime/Factory/IObj.hpp"

namespace metaforce {
class CCharacterFactory;
class CAnimRes;

class CCharacterFactoryBuilder {
public:
  class CDummyFactory : public IFactory {
  public:
    CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&, CObjectReference* selfRef) override;
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, std::unique_ptr<IObj>*,
                    CObjectReference* selfRef) override;
    void CancelBuild(const SObjectTag&) override;
    bool CanBuild(const SObjectTag&) override;
    const SObjectTag* GetResourceIdByName(std::string_view) const override;
    FourCC GetResourceTypeById(CAssetId id) const override;

    void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const override;
    void EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const override;

    u32 ResourceSize(const metaforce::SObjectTag& tag) override;
    std::shared_ptr<IDvdRequest> LoadResourceAsync(const metaforce::SObjectTag& tag, void* target) override;
    std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const metaforce::SObjectTag& tag, u32 off, u32 size,
                                                       void* target) override;
    std::unique_ptr<u8[]> LoadResourceSync(const metaforce::SObjectTag& tag) override;
    std::unique_ptr<u8[]> LoadNewResourcePartSync(const metaforce::SObjectTag& tag, u32 off, u32 size) override;
  };

private:
  CDummyFactory x0_dummyFactory;
  CSimplePool x4_dummyStore;

public:
  CCharacterFactoryBuilder();
  TToken<CCharacterFactory> GetFactory(const CAnimRes& res);
};

} // namespace metaforce
