#ifndef _CZIPOUTPUTSTREAM
#define _CZIPOUTPUTSTREAM


#include "Kyoto/Streams/COutputStream.hpp"

#include "rstl/auto_ptr.hpp"

#include <zlib.h>

class CZipOutputStream : public COutputStream {
public:
  CZipOutputStream(COutputStream* out, int level);
  ~CZipOutputStream();

  void Finish();
  bool Process(bool v);
  void Write(const void* ptr, size_t len);

  int GetCompressedBytesWritten() { return mCompressedBytesWritten; }
private:
  COutputStream* mOutput;
  int mCompressedBytesWritten;
  rstl::auto_ptr<z_stream> mZStream;
  int mUnk;
};


#endif // _CZIPOUTPUTSTREAM
