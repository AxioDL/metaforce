#include "ProjectResourceFactoryMP1.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Particle/CParticleElectricDataFactory.hpp"
#include "Runtime/Particle/CParticleSwooshDataFactory.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/Particle/CProjectileWeaponDataFactory.hpp"
#include "Runtime/Particle/CDecalDataFactory.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Character/CAllFormatsAnimSource.hpp"
#include "Runtime/Character/CAnimPOIData.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/Collision/CCollisionResponseData.hpp"
#include "Runtime/CSaveWorld.hpp"
#include "Runtime/AutoMapper/CMapWorld.hpp"
#include "Runtime/AutoMapper/CMapArea.hpp"
#include "Runtime/AutoMapper/CMapUniverse.hpp"
#include "Runtime/CScannableObjectInfo.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "Audio/CSfxManager.hpp"
#include "Audio/CMidiManager.hpp"
#include "Runtime/CDependencyGroup.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"

namespace DataSpec
{
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;
}

namespace urde
{

class MP1OriginalIDs
{
    std::vector<std::pair<ResId, ResId>> m_origToNew;
    std::vector<std::pair<ResId, ResId>> m_newToOrig;

public:
    MP1OriginalIDs(CInputStream& in)
    {
        u32 count = in.readUint32Big();
        m_origToNew.reserve(count);
        for (u32 i=0 ; i<count ; ++i)
        {
            ResId a = in.readUint32Big();
            ResId b = in.readUint32Big();
            m_origToNew.push_back(std::make_pair(a, b));
        }
        m_newToOrig.reserve(count);
        for (u32 i=0 ; i<count ; ++i)
        {
            ResId a = in.readUint32Big();
            ResId b = in.readUint32Big();
            m_newToOrig.push_back(std::make_pair(a, b));
        }
    }

    ResId TranslateOriginalToNew(ResId id) const
    {
        auto search = std::lower_bound(m_origToNew.cbegin(), m_origToNew.cend(), id,
        [](const auto& id, ResId test) -> bool { return id.first < test; });
        if (search == m_origToNew.cend() || search->first != id)
            return -1;
        return search->second;
    }

    ResId TranslateNewToOriginal(ResId id) const
    {
        auto search = std::lower_bound(m_newToOrig.cbegin(), m_newToOrig.cend(), id,
        [](const auto& id, ResId test) -> bool { return id.first < test; });
        if (search == m_newToOrig.cend() || search->first != id)
            return -1;
        return search->second;
    }
};

CFactoryFnReturn FMP1OriginalIDsFactory(const SObjectTag& tag, CInputStream& in,
                                        const CVParamTransfer& param,
                                        CObjectReference* selfRef)
{
    return TToken<MP1OriginalIDs>::GetIObjObjectFor(std::make_unique<MP1OriginalIDs>(in));
}

ProjectResourceFactoryMP1::ProjectResourceFactoryMP1(hecl::ClientProcess& clientProc)
: ProjectResourceFactoryBase(clientProc)
{
    m_factoryMgr.AddFactory(FOURCC('TXTR'), FMemFactoryFunc(FTextureFactory));
    m_factoryMgr.AddFactory(FOURCC('PART'), FFactoryFunc(FParticleFactory));
    m_factoryMgr.AddFactory(FOURCC('FRME'), FFactoryFunc(RGuiFrameFactoryInGame));
    m_factoryMgr.AddFactory(FOURCC('FONT'), FFactoryFunc(FRasterFontFactory));
    m_factoryMgr.AddFactory(FOURCC('CMDL'), FMemFactoryFunc(FModelFactory));
    m_factoryMgr.AddFactory(FOURCC('CINF'), FFactoryFunc(FCharLayoutInfo));
    m_factoryMgr.AddFactory(FOURCC('CSKR'), FFactoryFunc(FSkinRulesFactory));
    m_factoryMgr.AddFactory(FOURCC('ANCS'), FFactoryFunc(FAnimCharacterSet));
    m_factoryMgr.AddFactory(FOURCC('ANIM'), FFactoryFunc(AnimSourceFactory));
    m_factoryMgr.AddFactory(FOURCC('EVNT'), FFactoryFunc(AnimPOIDataFactory));
    m_factoryMgr.AddFactory(FOURCC('DCLN'), FFactoryFunc(FCollidableOBBTreeGroupFactory));
    m_factoryMgr.AddFactory(FOURCC('DGRP'), FFactoryFunc(FDependencyGroupFactory));
    m_factoryMgr.AddFactory(FOURCC('AGSC'), FMemFactoryFunc(FAudioGroupSetDataFactory));
    m_factoryMgr.AddFactory(FOURCC('CSNG'), FFactoryFunc(FMidiDataFactory));
    m_factoryMgr.AddFactory(FOURCC('ATBL'), FFactoryFunc(FAudioTranslationTableFactory));
    m_factoryMgr.AddFactory(FOURCC('STRG'), FFactoryFunc(FStringTableFactory));
    m_factoryMgr.AddFactory(FOURCC('HINT'), FFactoryFunc(FHintFactory));
    m_factoryMgr.AddFactory(FOURCC('SAVW'), FFactoryFunc(FSaveWorldFactory));
    m_factoryMgr.AddFactory(FOURCC('MAPW'), FFactoryFunc(FMapWorldFactory));
    m_factoryMgr.AddFactory(FOURCC('OIDS'), FFactoryFunc(FMP1OriginalIDsFactory));
    m_factoryMgr.AddFactory(FOURCC('SCAN'), FFactoryFunc(FScannableObjectInfoFactory));
    m_factoryMgr.AddFactory(FOURCC('CRSC'), FFactoryFunc(FCollisionResponseDataFactory));
    m_factoryMgr.AddFactory(FOURCC('SWHC'), FFactoryFunc(FParticleSwooshDataFactory));
    m_factoryMgr.AddFactory(FOURCC('ELSC'), FFactoryFunc(FParticleElectricDataFactory));
    m_factoryMgr.AddFactory(FOURCC('WPSC'), FFactoryFunc(FProjectileWeaponDataFactory));
    m_factoryMgr.AddFactory(FOURCC('DPSC'), FFactoryFunc(FDecalDataFactory));
    m_factoryMgr.AddFactory(FOURCC('MAPA'), FFactoryFunc(FMapAreaFactory));
    m_factoryMgr.AddFactory(FOURCC('MAPU'), FFactoryFunc(FMapUniverseFactory));
}

void ProjectResourceFactoryMP1::IndexMP1Resources(hecl::Database::Project& proj, CSimplePool& sp)
{
    BeginBackgroundIndex(proj, DataSpec::SpecEntMP1, DataSpec::SpecEntMP1PC);
    m_origIds = sp.GetObj("MP1OriginalIDs");
}

void ProjectResourceFactoryMP1::GetTagListForFile(const char* pakName, std::vector<SObjectTag>& out) const
{
    std::string pathPrefix("MP1/");
    pathPrefix += pakName;
    pathPrefix += '/';

    std::unique_lock<std::mutex> lk(
        const_cast<ProjectResourceFactoryMP1&>(*this).m_backgroundIndexMutex);
    for (const auto& tag : m_tagToPath)
        if (!tag.second.getRelativePathUTF8().compare(0, pathPrefix.size(), pathPrefix))
            out.push_back(tag.first);
}

void ProjectResourceFactoryMP1::Shutdown()
{
    m_origIds = TLockedToken<MP1OriginalIDs>();
    ProjectResourceFactoryBase::Shutdown();
}

ResId ProjectResourceFactoryMP1::TranslateOriginalToNew(ResId id) const
{
    return m_origIds->TranslateOriginalToNew(id);
}

ResId ProjectResourceFactoryMP1::TranslateNewToOriginal(ResId id) const
{
    return m_origIds->TranslateNewToOriginal(id);
}

}
