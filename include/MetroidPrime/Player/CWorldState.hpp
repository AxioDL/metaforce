#ifndef _CWORLDSTATE
#define _CWORLDSTATE

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CScriptMailbox.hpp"
#include "MetroidPrime/CScriptLayerManager.hpp"
#include "rstl/rc_ptr.hpp"

class CWorldState {
public:
  explicit CWorldState(CAssetId worldId);
  CWorldState(CInputStream& in, CAssetId worldId, const CWorldSaveGameInfo& saveWorld);
  void PutTo(COutputStream& out, CWorldSaveGameInfo& saveWorld);

  rstl::ncrc_ptr< CScriptLayerManager >& GetLayerState();
  rstl::ncrc_ptr< CScriptMailbox >& Mailbox();
  CAssetId GetWorldAssetId() const;
  CAssetId GetDesiredAreaAssetId() const;
  TAreaId GetCurrentArea() const;
  void SetDesiredAreaAssetId(CAssetId id);
  void SetAreaId(TAreaId id);
  rstl::ncrc_ptr< CMapWorldInfo >& MapWorldInfo();
  rstl::rc_ptr< CMapWorldInfo > GetMapWorldInfo() const;

private:
  CAssetId mWorldId;
  TAreaId mAreaId;
  rstl::ncrc_ptr< CScriptMailbox > mMailbox;
  rstl::ncrc_ptr< CMapWorldInfo > mMapWorldInfo;
  CAssetId mDesiredAreaAssetId;
  rstl::ncrc_ptr< CScriptLayerManager > mLayerState;
};
CHECK_SIZEOF(CWorldState, 0x18)

#endif // _CWORLDSTATE
