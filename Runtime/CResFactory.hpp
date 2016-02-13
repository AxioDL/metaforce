#ifndef __PSHAG_CRESFACTORY_HPP__
#define __PSHAG_CRESFACTORY_HPP__

#include <unordered_map>
#include "IFactory.hpp"
#include "CResLoader.hpp"
#include "IVParamObj.hpp"

namespace pshag
{
class IDvdRequest;

class CResFactory : public IFactory
{
    CResLoader x4_loader;
public:
    struct SLoadingData
    {
        SObjectTag x0_tag;
        IDvdRequest* x8_dvdReq;
        IObj** xc_targetPtr;
        void* x10_loadBuffer;
        u32 x14_resSize;
        CVParamTransfer x18_cvXfer;
    };
private:
    std::unordered_map<SObjectTag, SLoadingData> m_loadList;
    void AddToLoadList(const SLoadingData& data) {m_loadList[data.x0_tag] = data;}
public:
    CResLoader& GetLoader() {return x4_loader;}
    std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&);
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
    void CancelBuild(const SObjectTag&);
    bool CanBuild(const SObjectTag& tag) {return x4_loader.ResourceExists(tag);}
    const SObjectTag* GetResourceIdByName(const char* name) const {return x4_loader.GetResourceIdByName(name);}

    std::vector<std::pair<std::string, SObjectTag>> GetResourceIdToNameList() const
    {
        std::vector<std::pair<std::string, SObjectTag>> retval;
        return retval;
    }
};

}

#endif // __PSHAG_CRESFACTORY_HPP__
