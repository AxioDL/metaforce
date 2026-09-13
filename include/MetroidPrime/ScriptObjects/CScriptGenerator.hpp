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
  int x34_spawnCount;
  bool x38_24_noReuseFollowers : 1;
  bool x38_25_noInheritTransform : 1;
  CVector3f x3c_offset;
  float x48_minScale;
  float x4c_maxScale;
};

#endif // _CSCRIPTGENERATOR
