#ifndef _CTRANSITIONDATABASE
#define _CTRANSITIONDATABASE

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"

class IMetaTrans;

class CTransitionDatabase {
public:
  virtual const rstl::rc_ptr< IMetaTrans >& GetMetaTrans(uint from, uint to) const = 0;

  explicit CTransitionDatabase(
      const rstl::optional_object< TToken< CTransitionDatabase > >& token = rstl::optional_object_null())
  : mToken(token) {}
  ~CTransitionDatabase() {}

private:
  rstl::optional_object< TToken< CTransitionDatabase > > mToken;
};
CHECK_SIZEOF(CTransitionDatabase, 0x10)

#endif // _CTRANSITIONDATABASE
