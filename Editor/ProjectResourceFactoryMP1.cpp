#include "ProjectResourceFactoryMP1.hpp"
#include "Runtime/IOStreams.hpp"

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
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
#include "Runtime/CSaveWorld.hpp"
#include "Runtime/AutoMapper/CMapWorld.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "Runtime/CDependencyGroup.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"

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
    m_factoryMgr.AddFactory(FOURCC('STRG'), FFactoryFunc(FStringTableFactory));
    m_factoryMgr.AddFactory(FOURCC('HINT'), FFactoryFunc(FHintFactory));
    m_factoryMgr.AddFactory(FOURCC('SAVW'), FFactoryFunc(FSaveWorldFactory));
    m_factoryMgr.AddFactory(FOURCC('MAPW'), FFactoryFunc(FMapWorldFactory));
}

void ProjectResourceFactoryMP1::IndexMP1Resources(hecl::Database::Project& proj)
{
    BeginBackgroundIndex(proj, DataSpec::SpecEntMP1, DataSpec::SpecEntMP1PC);
}

}
