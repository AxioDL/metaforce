#pragma once

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/IObjFactory.hpp"
#include "Runtime/Character/CAnimationSet.hpp"
#include "Runtime/Character/CCharacterInfo.hpp"

namespace metaforce {
class CAdditiveAnimationInfo;
class CAllFormatsAnimSource;
class CAnimCharacterSet;
class CAnimData;
class CAnimationManager;
class CCharLayoutInfo;
class CSimplePool;
class CTransitionDatabaseGame;
class CTransitionManager;

class CCharacterFactory : public IObjFactory {
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
  std::vector<CCharacterInfo> x4_charInfoDB;
  std::vector<TLockedToken<CCharLayoutInfo>> x14_charLayoutInfoDB;
  std::shared_ptr<CAnimSysContext> x24_sysContext;
  std::shared_ptr<CAnimationManager> x28_animMgr;
  std::shared_ptr<CTransitionManager> x2c_transMgr;
  std::vector<TCachedToken<CAllFormatsAnimSource>> x30_animSourceDB;
  std::vector<std::pair<u32, CAdditiveAnimationInfo>> x40_additiveInfo;
  CAdditiveAnimationInfo x50_defaultAdditiveInfo;
  std::vector<std::pair<CAssetId, CAssetId>> x58_animResources;
  CAssetId x68_selfId;
  CDummyFactory x6c_dummyFactory;
  CSimplePool x70_cacheResPool;

  static std::vector<CCharacterInfo> GetCharacterInfoDB(const CAnimCharacterSet& ancs);
  static std::vector<TLockedToken<CCharLayoutInfo>> GetCharLayoutInfoDB(CSimplePool& store,
                                                                        const std::vector<CCharacterInfo>& chars);

public:
  CCharacterFactory(CSimplePool& store, const CAnimCharacterSet& ancs, CAssetId);

  std::unique_ptr<CAnimData> CreateCharacter(int charIdx, bool loop, const TLockedToken<CCharacterFactory>& factory,
                                             int defaultAnim);
  CAssetId GetEventResourceIdForAnimResourceId(CAssetId animId) const;

  const CCharacterInfo& GetCharInfo(int charIdx) const { return x4_charInfoDB[charIdx]; }
  const CAdditiveAnimationInfo& FindAdditiveInfo(s32 idx) const;
  bool HasAdditiveInfo(s32 idx) const;
};

} // namespace metaforce
