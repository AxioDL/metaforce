#ifndef _CGRAPHICSSYS
#define _CGRAPHICSSYS

#include "types.h"

class COsContext;
class CMemorySys;

class CGraphicsSys {
public:
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, uint, void*);
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, bool progressive);
  ~CGraphicsSys();

private:
  uint pad;

  static bool mGraphicsInitialized;
};

#endif // _CGRAPHICSSYS
