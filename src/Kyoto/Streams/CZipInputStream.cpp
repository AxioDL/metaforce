#include "Kyoto/Streams/CZipInputStream.hpp"
#include "Kyoto/Streams/CZipSupport.hpp"

CZipInputStream::CZipInputStream(rstl::auto_ptr<CInputStream> in)
: CInputStream(4096)
, mCompBuf(rs_new uchar[4096])
, mStream(in)
, mZStream(rs_new z_stream_s) {
  z_stream_s* zs = mZStream.get();
  zs->next_in = mCompBuf.get();
  mZStream->avail_in = 0;
  mZStream->zalloc = CZipSupport::Alloc;
  mZStream->zfree = CZipSupport::Free;
  mZStream->opaque = 0;
#if NONMATCHING
  inflateInit(mZStream.get());
#else
  inflateInit2(mZStream.get());
#endif
}

CZipInputStream::~CZipInputStream() {
  inflateEnd(mZStream.get());
}


size_t CZipInputStream::Read(void* buf, size_t len) {
  mZStream->next_out = static_cast<Bytef*>(buf);
  mZStream->avail_out = len;
  if (mZStream->avail_in == 0) {
    mZStream->avail_in = mStream->ReadBytes(mCompBuf.get(), 4096);
    mZStream->next_in = mCompBuf.get();
  }
  inflate(mZStream.get(), Z_NO_FLUSH);
  return len - mZStream->avail_out;
}
