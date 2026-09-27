#ifndef _CGAMEGLOBALOBJECTS
#define _CGAMEGLOBALOBJECTS

#include "types.h"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/CMemoryCardSys.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CGraphicsSys.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

class IRenderer;
class CStringTable;

class CGameGlobalObjects : public TOneStatic< CGameGlobalObjects > {
public:
#if defined(TARGET_PC)
  CGameGlobalObjects(COsContext&);
  void PostInitialize(COsContext&);
#else
  CGameGlobalObjects(COsContext&, CMemorySys&);
  void PostInitialize(COsContext&, CMemorySys&);
#endif

#if VERSION >= VERSION_GM8E_01
  void AddPaksAndFactories(const COsContext& osContext);
#else
  void AddPaksAndFactories();
#endif
  void LoadStringTable();

  rstl::single_ptr< CGameState >& GameState() { return mGameState; }
  rstl::single_ptr< CMemoryCard >& MemoryCard() { return mMemoryCard; }

  static CRasterFont* LoadDefaultFont();

private:
  CMemoryCardSys mMemoryCardSys;
  CResFactory mResFactory;
  CSimplePool mSimplePool;
  CCharacterFactoryBuilder mCharacterFactoryBuilder;
  CAiFuncMap mAiFuncMap;
  CGraphicsSys mGraphicsSys;
  rstl::single_ptr< CGameState > mGameState;
  rstl::single_ptr< CMemoryCard > mMemoryCard;
  rstl::optional_object< TLockedToken< CStringTable > > mStringTable;
  rstl::single_ptr< IRenderer > mRenderer;
  rstl::single_ptr< CInGameTweakManager > mInGameTweakManager;
  TToken< CRasterFont > mDefaultFont;
};
CHECK_SIZEOF(CGameGlobalObjects, 0x15c)

class IController;
// TODO move to related headers
extern IController* gpController;
extern const TToken< CRasterFont >* gpDefaultFont;
extern bool sProgressiveModePrompt;

#endif // _CGAMEGLOBALOBJECTS
