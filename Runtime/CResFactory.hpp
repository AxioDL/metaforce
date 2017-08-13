#ifndef __URDE_CRESFACTORY_HPP__
#define __URDE_CRESFACTORY_HPP__

#include <unordered_map>
#include "IFactory.hpp"
#include "CResLoader.hpp"
#include "IVParamObj.hpp"

namespace urde
{
class IDvdRequest;

class CResFactory : public IFactory
{
    CResLoader x4_loader;
public:
    struct SLoadingData
    {
        SObjectTag x0_tag;
        IDvdRequest* x8_dvdReq = nullptr;
        IObj** xc_targetPtr = nullptr;
        void* x10_loadBuffer = nullptr;
        u32 x14_resSize = 0;
        CVParamTransfer x18_cvXfer;

        SLoadingData() = default;
        SLoadingData(const SObjectTag& tag, IObj** ptr, const CVParamTransfer& xfer)
        : x0_tag(tag), xc_targetPtr(ptr), x18_cvXfer(xfer) {}
    };
private:
    std::unordered_map<SObjectTag, SLoadingData> m_loadList;
    void AddToLoadList(const SLoadingData& data) {m_loadList[data.x0_tag] = data;}
public:
    CResLoader& GetLoader() {return x4_loader;}
    std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&);
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
    void CancelBuild(const SObjectTag&);

    bool CanBuild(const SObjectTag& tag)
    {
        return x4_loader.ResourceExists(tag);
    }

    const SObjectTag* GetResourceIdByName(const char* name) const
    {
        return x4_loader.GetResourceIdByName(name);
    }

    FourCC GetResourceTypeById(CAssetId id) const
    {
        return x4_loader.GetResourceTypeById(id);
    }

    std::vector<std::pair<std::string, SObjectTag>> GetResourceIdToNameList() const
    {
        std::vector<std::pair<std::string, SObjectTag>> retval;
        return retval;
    }

    void EnumerateResources(std::function<bool(const SObjectTag&)> lambda) const
    {
    }

    void EnumerateNamedResources(std::function<bool(const std::string&, const SObjectTag&)> lambda) const
    {
    }
};

}

#endif // __URDE_CRESFACTORY_HPP__
