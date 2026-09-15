#ifndef _CDSPSTREAM
#define _CDSPSTREAM

#include "types.h"

#include "dolphin/dvd.h"
#include "musyx/musyx.h"

struct dspadpcm_header {
  uint x0_numSamples;
  uint x4_numNibbles;
  uint x8_sampleRate;
  ushort xc_loopFlag;
  ushort xe_format;
  uint x10_loopStartNibble;
  uint x14_loopEndNibble;
  uint x18_currentAddress;
  short x1c_coef[8][2];
  short x3c_gain;
  short x3e_predScale;
  short x40_hist1;
  short x42_hist2;
  short x44_loopPredScale;
  short x46_loopHist1;
  short x48_loopHist2;
  ushort x4a_pad[11];
};
CHECK_SIZEOF(dspadpcm_header, 0x60);

struct SStreamInfo {
  const char* x0_fileName;
  ushort x4_sampleRate;
  ushort x6_pad;
  uint x8_headerSize;
  uint xc_adpcmBytes;
  bool x10_loopFlag;
  uchar x11_pad[3];
  uint x14_loopStartByte;
  uint x18_loopEndByte;
  SND_ADPCMSTREAM_INFO x1c_adpcmInfo;
};
CHECK_SIZEOF(SStreamInfo, 0x3C);

class CDSPStream {
public:
  static void ReadCompleted(s32 result, DVDFileInfo* fileInfo);

  int InitializeStream();
  static u32 UpdateStream(void* dest, u32 destOffset, void* src, u32 len, u32 user);
  void BufferStream();
  void StopStream();

  static bool IsStreamAvailable(int handle);
  static bool IsStreamActive(int handle);
  static void UpdateVolume(int handle, int vol);
  void UpdateStreamVolume(int vol);

  static void Silence(int handle);
  void SilenceStream();

  static int AllocateStereo(const SStreamInfo& leftInfo, const SStreamInfo& rightInfo, char vol,
                            int oneshot);
  void DeallocateStream();
  static int AllocateMono(const SStreamInfo& info, char vol, char pan, int oneshot);
  uint AllocateStream(const SStreamInfo& info, char vol, char pan);

  static void FreeAllStreams();
  static void Initialize();

private:
  void DoAllocateStream();
  static int FindStreamIdx(int handle);
  static int PickFreeStream(CDSPStream*& streamOut, int oneshot);
  void CloseFiles();
  static void OpenFiles(const char* fileName, CDSPStream& stream);

private:
  uchar x0_state;
  uchar x1_oneshot;
  ushort x2_pad;
  uint x4_handle;
  CDSPStream* x8_right;
  CDSPStream* xc_left;
  const char* x10_fileName;
  ushort x14_sampleRate;
  ushort x16_pad;
  uint x18_headerSize;
  uint x1c_adpcmBytes;
  bool x20_loopFlag;
  uchar x21_pad[3];
  uint x24_loopStartByte;
  uint x28_loopEndByte;
  SND_ADPCMSTREAM_INFO x2c_adpcmInfo;
  uchar x4c_vol;
  uchar x4d_pan;
  ushort x4e_pad;
  DVDFileInfo x50_fileInfo1;
  DVDFileInfo x8c_fileInfo2;
  uint xc8_streamId;
  uint xcc_fileCur;
  uint xd0_remaining;
  void* xd4_buffer;
  uint xd8_bufferBytes;
  uint xdc_streamSamples;
  uchar xe0_curBuffer;
  uchar xe1_pad[3];
  int xe4_needsPrime;
  int xe8_silenced;
  uchar xec_readsPending;
  uchar xed_pad[3];
  int xf0_stopRequested;
};
CHECK_SIZEOF(CDSPStream, 0xF4);

#endif // _CDSPSTREAM
