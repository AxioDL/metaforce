#ifndef __URDE_CCHARACTERFACTORY_HPP__
#define __URDE_CCHARACTERFACTORY_HPP__

#include "IFactory.hpp"
#include "IObjFactory.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CAnimationSet.hpp"

namespace urde
{
class CSimplePool;
class CAnimCharacterSet;
class CCharacterInfo;
class CCharLayoutInfo;
class CAdditiveAnimationInfo;
class CTransitionDatabaseGame;
class CAnimationManager;
class CTransitionManager;
class CAllFormatsAnimSource;
class CAnimData;

class CCharacterFactory : public IObjFactory
{
public:
    class CDummyFactory : public IFactory
    {
    public:
        CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&, CObjectReference* selfRef);
        void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**, CObjectReference* selfRef);
        void CancelBuild(const SObjectTag&);
        bool CanBuild(const SObjectTag&);
        const SObjectTag* GetResourceIdByName(const char*) const;
        FourCC GetResourceTypeById(CAssetId id) const;

        void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const;
        void EnumerateNamedResources(const std::function<bool(const std::string&, const SObjectTag&)>& lambda) const;

        u32 ResourceSize(const urde::SObjectTag& tag);
        bool LoadResourceAsync(const urde::SObjectTag& tag, std::unique_ptr<u8[]>& target);
        bool LoadResourcePartAsync(const urde::SObjectTag& tag, u32 size, u32 off, std::unique_ptr<u8[]>& target);
        std::unique_ptr<u8[]> LoadResourceSync(const urde::SObjectTag& tag);
        std::unique_ptr<u8[]> LoadResourcePartSync(const urde::SObjectTag& tag, u32 size, u32 off);
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
    static std::vector<TLockedToken<CCharLayoutInfo>>
    GetCharLayoutInfoDB(CSimplePool& store,
                        const std::vector<CCharacterInfo>& chars);

public:
    CCharacterFactory(CSimplePool& store, const CAnimCharacterSet& ancs, CAssetId);

    std::unique_ptr<CAnimData> CreateCharacter(int charIdx, bool loop,
                                               const TLockedToken<CCharacterFactory>& factory,
                                               int defaultAnim, int drawInsts) const;
    CAssetId GetEventResourceIdForAnimResourceId(CAssetId animId) const;

    const CCharacterInfo& GetCharInfo(int charIdx) const { return x4_charInfoDB[charIdx]; }
    const CAdditiveAnimationInfo& FindAdditiveInfo(u32 idx) const;
    bool HasAdditiveInfo(u32 idx) const;
};

}

#endif // __URDE_CCHARACTERFACTORY_HPP__
