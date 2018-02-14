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
#include "Runtime/World/CPathFindArea.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "Audio/CSfxManager.hpp"
#include "Audio/CMidiManager.hpp"
#include "Runtime/CDependencyGroup.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "CSimplePool.hpp"
#include "MP1/MP1OriginalIDs.hpp"
#include "GameGlobalObjects.hpp"

namespace DataSpec
{
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;
}

namespace urde
{

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
    m_factoryMgr.AddFactory(FOURCC('PATH'), FMemFactoryFunc(FPathFindAreaFactory));
}

void ProjectResourceFactoryMP1::IndexMP1Resources(hecl::Database::Project& proj, CSimplePool& sp)
{
    BeginBackgroundIndex(proj, DataSpec::SpecEntMP1, DataSpec::SpecEntMP1PC);
    m_origIds = sp.GetObj("MP1OriginalIDs");
}

void ProjectResourceFactoryMP1::Shutdown()
{
    m_origIds = TLockedToken<MP1OriginalIDs>();
    ProjectResourceFactoryBase::Shutdown();
}

CAssetId ProjectResourceFactoryMP1::TranslateOriginalToNew(CAssetId id) const
{
    return m_origIds->TranslateOriginalToNew(id);
}

CAssetId ProjectResourceFactoryMP1::TranslateNewToOriginal(CAssetId id) const
{
    return m_origIds->TranslateNewToOriginal(id);
}

}
