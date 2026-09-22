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
  CGameGlobalObjects(COsContext&, CMemorySys&);

  void PostInitialize(COsContext&, CMemorySys&);
#if VERSION >= VERSION_GM8E_01
  void AddPaksAndFactories(const COsContext& osContext);
#else
  void AddPaksAndFactories();
#endif
  void LoadStringTable();

  rstl::single_ptr< CGameState >& GameState() { return x134_gameState; }
  rstl::single_ptr< CMemoryCard >& MemoryCard() { return x138_memoryCard; }

  static CRasterFont* LoadDefaultFont();

private:
  CMemoryCardSys x0_memoryCardSys;
  CResFactory x4_resFactory;
  CSimplePool xcc_simplePool;
  CCharacterFactoryBuilder xec_characterFactoryBuilder;
  CAiFuncMap x110_aiFuncMap;
  CGraphicsSys x130_graphicsSys;
  rstl::single_ptr< CGameState > x134_gameState;
  rstl::single_ptr< CMemoryCard > x138_memoryCard;
  rstl::optional_object< TLockedToken< CStringTable > > x13c_stringTable;
  rstl::single_ptr< IRenderer > x14c_renderer;
  rstl::single_ptr< CInGameTweakManager > x150_inGameTweakManager;
  TToken< CRasterFont > x154_defaultFont;
};
CHECK_SIZEOF(CGameGlobalObjects, 0x15c)

class IController;
// TODO move to related headers
extern IController* gpController;
extern const TToken< CRasterFont >* gpDefaultFont;
extern bool sProgressiveModePrompt;

#endif // _CGAMEGLOBALOBJECTS
