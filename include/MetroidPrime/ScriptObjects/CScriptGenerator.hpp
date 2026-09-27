#ifndef _CSCRIPTGENERATOR
#define _CSCRIPTGENERATOR

#include "MetroidPrime/CEntity.hpp"

class CScriptGenerator : public CEntity {
public:
  CScriptGenerator(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   int spawnCount, bool noReuseFollowers, const CVector3f& vec1, bool noInheritXf,
                   bool active, float minScale, float maxScale);
  ~CScriptGenerator();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;

private:
  int mSpawnCount;
  bool mNoReuseFollowers : 1;
  bool mNoInheritTransform : 1;
  CVector3f mOffset;
  float mMinScale;
  float mMaxScale;
};

#endif // _CSCRIPTGENERATOR
