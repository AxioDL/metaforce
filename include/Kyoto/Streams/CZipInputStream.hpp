#ifndef _CZIPINPUTSTREAM
#define _CZIPINPUTSTREAM

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"

#include <zlib.h>

class CZipInputStream : public CInputStream {
public:
  CZipInputStream(rstl::auto_ptr< CInputStream > in);
  ~CZipInputStream() override;
  size_t Read(void* dest, size_t len) override;

private:
  rstl::single_ptr<uchar> mCompBuf;
  rstl::auto_ptr< CInputStream > mStream;
  rstl::single_ptr<z_stream_s> mZStream;
};

#endif // _CZIPINPUTSTREAM
