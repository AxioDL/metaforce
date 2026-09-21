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
  : x0_transDb(transDb), x8_random(random), xc_store(store) {}

  IObjectStore& GetSimplePool() const { return xc_store; }
  const TToken< CTransitionDatabase >& GetTransitionDatabase() const { return x0_transDb; }
  CRandom16& GetRandomNumberGenerator() const { return *x8_random; }
  CRandom16* Random() { return x8_random.GetPtr(); }
  const CRandom16* GetRandom() const { return x8_random.GetPtr(); }

private:
  TToken< CTransitionDatabase > x0_transDb;
  rstl::ncrc_ptr< CRandom16 > x8_random;
  IObjectStore& xc_store;
};
CHECK_SIZEOF(CAnimSysContext, 0x10)

#endif // _CANIMSYSCONTEXT
