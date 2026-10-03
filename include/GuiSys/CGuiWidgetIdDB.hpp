#ifndef _CGUIWIDGETIDDB
#define _CGUIWIDGETIDDB

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CGuiWidgetIdDB {
public:
  CGuiWidgetIdDB();
  void Reserve(int);
  const short AddWidget(const rstl::string& name);
  const short FindWidgetID(const rstl::string& name) const;

private:
  rstl::vector< rstl::string > mDb;
  short mLastPoolId;
};

#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CGuiWidgetIdDB, 0x10);
#else
CHECK_SIZEOF(CGuiWidgetIdDB, 0x14);
#endif

#endif // _CGUIWIDGETIDDB
