#ifndef _RSTL_STRINGEXTRAS
#define _RSTL_STRINGEXTRAS

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CStringExtras {
public:
  static int IndexOfSubstring(const rstl::string&, const rstl::string&);
  static int CompareCaseInsensitive(const rstl::string&, const rstl::string&);
  static char ConvertToUpperCase(const char c);
  static rstl::string CreateFromInteger(int v);
  static rstl::string CreateFromReal(float v, int i);
  static rstl::string ConvertToANSI(const rstl::wstring& str);
  static rstl::wstring ConvertToUNICODE(const rstl::string& str);
  static rstl::string ReadString(CInputStream& in);
  static rstl::vector< rstl::string > TokenizeString(const rstl::string&, const char*, int);
};

#endif // _RSTL_STRINGEXTRAS
