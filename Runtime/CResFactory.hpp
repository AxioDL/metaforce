#ifndef __RETRO_CRESFACTORY_HPP__
#define __RETRO_CRESFACTORY_HPP__

#include <unordered_map>
#include "IFactory.hpp"
#include "CResLoader.hpp"

namespace Retro
{
class CDvdRequest;

class CResFactory : public IFactory
{
    CResLoader x4_loader;
public:
    struct SLoadingData
    {
        SObjectTag tag;
        CDvdRequest* dvdReq;
        IObj** targetPtr;
        void* loadBuffer;
        u32 resSize;
    };
private:
    std::unordered_map<SObjectTag, SLoadingData> m_loadList;
    void AddToLoadList(const SLoadingData& data) {m_loadList[data.tag] = data;}
public:
    CResLoader& GetLoader() {return x4_loader;}
    std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&);
    void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
    void CancelBuild(const SObjectTag&);
    bool CanBuild(const SObjectTag& tag) {return x4_loader.ResourceExists(tag);}
    u32 GetResourceIdByName(const char* name) const {return x4_loader.GetResourceIdByName(name);}

    std::vector<std::pair<std::string, SObjectTag>> GetResourceIdToNameList() const
    {
        std::vector<std::pair<std::string, SObjectTag>> retval;
        return retval;
    }
};

}

#endif // __RETRO_CRESFACTORY_HPP__
