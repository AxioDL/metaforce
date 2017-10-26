#include "CResFactory.hpp"
#include "CSimplePool.hpp"

namespace urde
{

void CResFactory::AddToLoadList(SLoadingData&& data)
{
    m_loadMap[data.x0_tag] = m_loadList.insert(m_loadList.end(), std::move(data));
}

CFactoryFnReturn CResFactory::BuildSync(const SObjectTag& tag, const CVParamTransfer& xfer, CObjectReference* selfRef)
{
    if (x5c_factoryMgr.CanMakeMemory(tag))
    {
        std::unique_ptr<uint8_t[]> data;
        int size;
        x4_loader.LoadMemResourceSync(tag, data, &size);
        return x5c_factoryMgr.MakeObjectFromMemory(tag, std::move(data), size,
               x4_loader.GetResourceCompression(tag), xfer, selfRef);
    }
    else
    {
        auto rp = x4_loader.LoadNewResourceSync(tag, nullptr);
        return x5c_factoryMgr.MakeObject(tag, *rp, xfer, selfRef);
    }
}

bool CResFactory::PumpResource(SLoadingData& data)
{
    if (data.x8_dvdReq->IsComplete())
    {
        *data.xc_targetPtr =
            x5c_factoryMgr.MakeObjectFromMemory(data.x0_tag, std::move(data.x10_loadBuffer),
                                                data.x14_resSize, data.m_compressed, data.x18_cvXfer,
                                                data.m_selfRef);
        return true;
    }
    return false;
}

std::unique_ptr<IObj> CResFactory::Build(const SObjectTag& tag, const CVParamTransfer& xfer, CObjectReference* selfRef)
{
    auto search = m_loadMap.find(tag);
    if (search != m_loadMap.end())
    {
        while (!PumpResource(*search->second) || !search->second->xc_targetPtr) {}
        m_loadList.erase(search->second);
        m_loadMap.erase(search);
        return std::move(*search->second->xc_targetPtr);
    }
    return BuildSync(tag, xfer, selfRef);
}

void CResFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& xfer, std::unique_ptr<IObj>* target,
                             CObjectReference* selfRef)
{
    auto search = m_loadMap.find(tag);
    if (search != m_loadMap.end())
    {
        SLoadingData data(tag, target, xfer, x4_loader.GetResourceCompression(tag), selfRef);
        data.x10_loadBuffer = std::unique_ptr<u8[]>(new u8[x4_loader.ResourceSize(tag)]);
        x4_loader.LoadResourceAsync(tag, data.x10_loadBuffer.get());
        AddToLoadList(std::move(data));
    }
}

void CResFactory::AsyncIdle()
{
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::steady_clock::now() - startTime).count() < 2)
    {
        auto& task = m_loadList.front();
        if (PumpResource(task))
        {
            m_loadMap.erase(task.x0_tag);
            m_loadList.pop_front();
        }
    }
}

void CResFactory::CancelBuild(const SObjectTag& tag)
{
    auto search = m_loadMap.find(tag);
    if (search != m_loadMap.end())
    {
        search->second->x8_dvdReq->PostCancelRequest();
        m_loadList.erase(search->second);
        m_loadMap.erase(search);
    }
}

void CResFactory::LoadOriginalIDs(CSimplePool& sp)
{
    m_origIds = sp.GetObj("MP1OriginalIDs");
}

CAssetId CResFactory::TranslateOriginalToNew(CAssetId id) const
{
    return m_origIds->TranslateOriginalToNew(id);
}

CAssetId CResFactory::TranslateNewToOriginal(CAssetId id) const
{
    return m_origIds->TranslateNewToOriginal(id);
}

}
