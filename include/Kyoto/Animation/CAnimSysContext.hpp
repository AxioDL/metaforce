#ifndef _CANIMSYSCONTEXT
#define _CANIMSYSCONTEXT

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"

class CTransitionDatabase;
class IObjectStore;

class CAnimSysContext {
public:
  CAnimSysContext(const TToken< CTransitionDatabase >& transDb,
                  const rstl::ncrc_ptr< CRandom16 >& random, IObjectStore& store)
  : mTransDb(transDb), mRandom(random), mStore(store) {}

  IObjectStore& GetSimplePool() const { return mStore; }
  const TToken< CTransitionDatabase >& GetTransitionDatabase() const { return mTransDb; }
  CRandom16& GetRandomNumberGenerator() const { return *mRandom; }
  CRandom16* Random() { return mRandom.GetPtr(); }
  const CRandom16* GetRandom() const { return mRandom.GetPtr(); }

private:
  TToken< CTransitionDatabase > mTransDb;
  rstl::ncrc_ptr< CRandom16 > mRandom;
  IObjectStore& mStore;
};
CHECK_SIZEOF(CAnimSysContext, 0x10)

#endif // _CANIMSYSCONTEXT
