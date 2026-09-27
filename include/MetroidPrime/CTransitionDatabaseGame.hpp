#ifndef _CTRANSITIONDATABASEGAME
#define _CTRANSITIONDATABASEGAME

#include "Kyoto/Animation/CTransitionDatabase.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CTransition;
class CHalfTransition;

class CTransitionDatabaseGame : public CTransitionDatabase {
public:
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans(uint from, uint to) const override;

  ~CTransitionDatabaseGame() {}

  CTransitionDatabaseGame(const rstl::vector< CTransition >& transitions,
                          const rstl::vector< CHalfTransition >& halfTransitions,
                          rstl::rc_ptr< IMetaTrans > defaultTrans);

private:
  rstl::rc_ptr< IMetaTrans > mDefaultTrans;
  rstl::vector< rstl::pair< rstl::pair< uint, uint >, rstl::rc_ptr< IMetaTrans > > >
      mTransitions;
  rstl::vector< rstl::pair< uint, rstl::rc_ptr< IMetaTrans > > > mHalfTransitions;
};
CHECK_SIZEOF(CTransitionDatabaseGame, 0x34)

#endif // _CTRANSITIONDATABASEGAME
