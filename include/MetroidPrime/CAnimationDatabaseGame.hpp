#ifndef _CANIMATIONDATABASEGAME
#define _CANIMATIONDATABASEGAME

#include <Kyoto/Animation/CAnimationDatabase.hpp>

class CAnimationDatabaseGame : public CAnimationDatabase {
public:
  explicit CAnimationDatabaseGame(const rstl::vector< CAnimation >& anims);

  const rstl::rc_ptr< IMetaAnim >& GetMetaAnim(uint idx) const override;
  uint GetNumMetaAnims() const override;
  rstl::string GetMetaAnimName(uint idx) const override;
  void GetAllUniquePrimitives(rstl::vector< CPrimitive >& primsOut) const override;
  void GetUniquePrimitivesFromMetaAnim(rstl::set< CPrimitive >& primsOut,
                                       const rstl::string& name) const override;

private:
  rstl::vector< rstl::rc_ptr< IMetaAnim > > mAnims;
};
CHECK_SIZEOF(CAnimationDatabaseGame, 0x20)

#endif // _CANIMATIONDATABASEGAME
