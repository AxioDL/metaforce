#include "Kyoto/Audio/CAudioSys.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioGroupSet.hpp"

#include "dolphin/ai.h"
#include "dolphin/ar.h"
#include "dolphin/dtk.h"
#include "dolphin/os.h"

#include "dolphin/types.h"
#include "musyx/musyx.h"

#include <rstl/math.hpp>
#include <string.h>

const ushort CAudioSys::kVolumeTable[] = {
    // pow(i / 127, 2) * 32768
    0x0000, // 0x0000 -> 0.00000000 -> 0.00000000
    0x0002, // 0x0001 -> 0.00787402 -> 2.03162003
    0x0008, // 0x0002 -> 0.01574803 -> 8.12648010
    0x0012, // 0x0003 -> 0.02362205 -> 18.28458023
    0x0020, // 0x0004 -> 0.03149606 -> 32.50592041
    0x0032, // 0x0005 -> 0.03937008 -> 50.79050064
    0x0049, // 0x0006 -> 0.04724409 -> 73.13832092
    0x0063, // 0x0007 -> 0.05511811 -> 99.54938507
    0x0082, // 0x0008 -> 0.06299213 -> 130.02368164
    0x00A4, // 0x0009 -> 0.07086615 -> 164.56124878
    0x00CB, // 0x000A -> 0.07874016 -> 203.16200256
    0x00F5, // 0x000B -> 0.08661418 -> 245.82604980
    0x0124, // 0x000C -> 0.09448819 -> 292.55328369
    0x0157, // 0x000D -> 0.10236221 -> 343.34381104
    0x018E, // 0x000E -> 0.11023622 -> 398.19754028
    0x01C9, // 0x000F -> 0.11811024 -> 457.11453247
    0x0208, // 0x0010 -> 0.12598425 -> 520.09472656
    0x024B, // 0x0011 -> 0.13385826 -> 587.13818359
    0x0292, // 0x0012 -> 0.14173229 -> 658.24499512
    0x02DD, // 0x0013 -> 0.14960630 -> 733.41485596
    0x032C, // 0x0014 -> 0.15748031 -> 812.64801025
    0x037F, // 0x0015 -> 0.16535433 -> 895.94439697
    0x03D7, // 0x0016 -> 0.17322835 -> 983.30419922
    0x0432, // 0x0017 -> 0.18110237 -> 1074.72705078
    0x0492, // 0x0018 -> 0.18897638 -> 1170.21313477
    0x04F5, // 0x0019 -> 0.19685039 -> 1269.76245117
    0x055D, // 0x001A -> 0.20472442 -> 1373.37524414
    0x05C9, // 0x001B -> 0.21259843 -> 1481.05102539
    0x0638, // 0x001C -> 0.22047244 -> 1592.79016113
    0x06AC, // 0x001D -> 0.22834645 -> 1708.59240723
    0x0724, // 0x001E -> 0.23622048 -> 1828.45812988
    0x07A0, // 0x001F -> 0.24409449 -> 1952.38696289
    0x0820, // 0x0020 -> 0.25196850 -> 2080.37890625
    0x08A4, // 0x0021 -> 0.25984251 -> 2212.43408203
    0x092C, // 0x0022 -> 0.26771653 -> 2348.55273438
    0x09B8, // 0x0023 -> 0.27559054 -> 2488.73437500
    0x0A48, // 0x0024 -> 0.28346458 -> 2632.97998047
    0x0ADD, // 0x0025 -> 0.29133859 -> 2781.28808594
    0x0B75, // 0x0026 -> 0.29921260 -> 2933.65942383
    0x0C12, // 0x0027 -> 0.30708662 -> 3090.09423828
    0x0CB2, // 0x0028 -> 0.31496063 -> 3250.59204102
    0x0D57, // 0x0029 -> 0.32283464 -> 3415.15332031
    0x0DFF, // 0x002A -> 0.33070865 -> 3583.77758789
    0x0EAC, // 0x002B -> 0.33858266 -> 3756.46533203
    0x0F5D, // 0x002C -> 0.34645671 -> 3933.21679688
    0x1012, // 0x002D -> 0.35433072 -> 4114.03076172
    0x10CA, // 0x002E -> 0.36220473 -> 4298.90820312
    0x1187, // 0x002F -> 0.37007874 -> 4487.84863281
    0x1248, // 0x0030 -> 0.37795275 -> 4680.85253906
    0x130D, // 0x0031 -> 0.38582677 -> 4877.91943359
    0x13D7, // 0x0032 -> 0.39370078 -> 5079.04980469
    0x14A4, // 0x0033 -> 0.40157479 -> 5284.24365234
    0x1575, // 0x0034 -> 0.40944883 -> 5493.50097656
    0x164A, // 0x0035 -> 0.41732284 -> 5706.82080078
    0x1724, // 0x0036 -> 0.42519686 -> 5924.20410156
    0x1801, // 0x0037 -> 0.43307087 -> 6145.65087891
    0x18E3, // 0x0038 -> 0.44094488 -> 6371.16064453
    0x19C8, // 0x0039 -> 0.44881889 -> 6600.73339844
    0x1AB2, // 0x003A -> 0.45669290 -> 6834.36962891
    0x1BA0, // 0x003B -> 0.46456692 -> 7072.06884766
    0x1C91, // 0x003C -> 0.47244096 -> 7313.83251953
    0x1D87, // 0x003D -> 0.48031497 -> 7559.65869141
    0x1E81, // 0x003E -> 0.48818898 -> 7809.54785156
    0x1F7F, // 0x003F -> 0.49606299 -> 8063.50000000
    0x2081, // 0x0040 -> 0.50393701 -> 8321.51562500
    0x2187, // 0x0041 -> 0.51181102 -> 8583.59472656
    0x2291, // 0x0042 -> 0.51968503 -> 8849.73632812
    0x239F, // 0x0043 -> 0.52755904 -> 9119.94238281
    0x24B2, // 0x0044 -> 0.53543305 -> 9394.21093750
    0x25C8, // 0x0045 -> 0.54330707 -> 9672.54199219
    0x26E2, // 0x0046 -> 0.55118108 -> 9954.93750000
    0x2801, // 0x0047 -> 0.55905509 -> 10241.39550781
    0x2923, // 0x0048 -> 0.56692916 -> 10531.91992188
    0x2A4A, // 0x0049 -> 0.57480317 -> 10826.50390625
    0x2B75, // 0x004A -> 0.58267719 -> 11125.15234375
    0x2CA3, // 0x004B -> 0.59055120 -> 11427.86328125
    0x2DD6, // 0x004C -> 0.59842521 -> 11734.63769531
    0x2F0D, // 0x004D -> 0.60629922 -> 12045.47558594
    0x3048, // 0x004E -> 0.61417323 -> 12360.37695312
    0x3187, // 0x004F -> 0.62204725 -> 12679.34082031
    0x32CA, // 0x0050 -> 0.62992126 -> 13002.36816406
    0x3411, // 0x0051 -> 0.63779527 -> 13329.45898438
    0x355C, // 0x0052 -> 0.64566928 -> 13660.61328125
    0x36AB, // 0x0053 -> 0.65354329 -> 13995.83007812
    0x37FF, // 0x0054 -> 0.66141731 -> 14335.11035156
    0x3956, // 0x0055 -> 0.66929132 -> 14678.45410156
    0x3AB1, // 0x0056 -> 0.67716533 -> 15025.86132812
    0x3C11, // 0x0057 -> 0.68503934 -> 15377.33105469
    0x3D74, // 0x0058 -> 0.69291341 -> 15732.86718750
    0x3EDC, // 0x0059 -> 0.70078743 -> 16092.46386719
    0x4048, // 0x005A -> 0.70866144 -> 16456.12304688
    0x41B7, // 0x005B -> 0.71653545 -> 16823.84570312
    0x432B, // 0x005C -> 0.72440946 -> 17195.63281250
    0x44A3, // 0x005D -> 0.73228347 -> 17571.48242188
    0x461F, // 0x005E -> 0.74015749 -> 17951.39453125
    0x479F, // 0x005F -> 0.74803150 -> 18335.37109375
    0x4923, // 0x0060 -> 0.75590551 -> 18723.41015625
    0x4AAB, // 0x0061 -> 0.76377952 -> 19115.51367188
    0x4C37, // 0x0062 -> 0.77165353 -> 19511.67773438
    0x4DC7, // 0x0063 -> 0.77952754 -> 19911.90820312
    0x4F5C, // 0x0064 -> 0.78740156 -> 20316.19921875
    0x50F4, // 0x0065 -> 0.79527557 -> 20724.55468750
    0x5290, // 0x0066 -> 0.80314958 -> 21136.97460938
    0x5431, // 0x0067 -> 0.81102359 -> 21553.45507812
    0x55D6, // 0x0068 -> 0.81889766 -> 21974.00390625
    0x577E, // 0x0069 -> 0.82677168 -> 22398.61328125
    0x592B, // 0x006A -> 0.83464569 -> 22827.28320312
    0x5ADC, // 0x006B -> 0.84251970 -> 23260.01953125
    0x5C90, // 0x006C -> 0.85039371 -> 23696.81640625
    0x5E49, // 0x006D -> 0.85826772 -> 24137.67773438
    0x6006, // 0x006E -> 0.86614174 -> 24582.60351562
    0x61C7, // 0x006F -> 0.87401575 -> 25031.59179688
    0x638C, // 0x0070 -> 0.88188976 -> 25484.64257812
    0x6555, // 0x0071 -> 0.88976377 -> 25941.75585938
    0x6722, // 0x0072 -> 0.89763778 -> 26402.93359375
    0x68F4, // 0x0073 -> 0.90551180 -> 26868.17382812
    0x6AC9, // 0x0074 -> 0.91338581 -> 27337.47851562
    0x6CA2, // 0x0075 -> 0.92125982 -> 27810.84570312
    0x6E80, // 0x0076 -> 0.92913383 -> 28288.27539062
    0x7061, // 0x0077 -> 0.93700784 -> 28769.76953125
    0x7247, // 0x0078 -> 0.94488192 -> 29255.33007812
    0x7430, // 0x0079 -> 0.95275593 -> 29744.95117188
    0x761E, // 0x007A -> 0.96062994 -> 30238.63476562
    0x7810, // 0x007B -> 0.96850395 -> 30736.38085938
    0x7A06, // 0x007C -> 0.97637796 -> 31238.19140625
    0x7C00, // 0x007D -> 0.98425198 -> 31744.06445312
    0x7DFE, // 0x007E -> 0.99212599 -> 32254.00000000
    0x8000, // 0x007F -> 1.00000000 -> 32768.00000000
};
bool CAudioSys::mInitialized = false;
bool CAudioSys::mIsListenerActive = false;
bool CAudioSys::mVerbose = false;
uchar CAudioSys::mMaxNumEmitters = 0;
rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >* CAudioSys::mpGroupSetDB = nullptr;
rstl::map< uint, rstl::string >* CAudioSys::mpGroupSetResNameDB = nullptr;
rstl::map< rstl::string, rstl::ncrc_ptr< CAudioSys::CTrkData > >* CAudioSys::mpDVDTrackDB = nullptr;
rstl::vector< CAudioSys::CEmitterData >* CAudioSys::mpEmitterDB = nullptr;
SND_LISTENER* CAudioSys::mpListener = nullptr;
uint CAudioSys::mUnusedEmitterHandle = 0;
CAudioSys::ESurroundModes CAudioSys::mSurroundMode = CAudioSys::kSM_Mono;
int CAudioSys::mMaxAramUsage = 0;
int CAudioSys::mCurrentAramUsage = 0;
const uchar CAudioSys::kMaxVolume = 0x7f;
const uchar CAudioSys::kEmitterMedPriority = 0x7f;
bool CAudioSys::mProLogic2 = true;

short CAudioSys::mVolumeScale = 0x7f;
short CAudioSys::mDefaultVolumeScale = 0x7f;

bool CAudioSys::mAICallbackEnabled = true;
void* CAudioSys::mAICallback = nullptr;

void* CAudioSys::mpSampleDataUploadBuffer = nullptr;

const rstl::string CAudioSys::mpDefaultInvalidString(rstl::string_l("NULL"));

void* DoMalloc(size_t len) { return rs_new char[len]; }

void DoFree(void* ptr) {
  if (!ptr) {
    return;
  }
  delete[] reinterpret_cast< char* >(ptr);
}

CAudioSys::CAudioSys(const uchar numVoices, const uchar numMusic, const uchar numSfx,
                     const uchar maxNumEmitters, const uint aramSize) {
  mInitialized = true;
  SND_HOOKS hooks = {DoMalloc, DoFree};
  AIInit(NULL);
  sndSetHooks(&hooks);
  if (mProLogic2) {
    sndInit(numVoices, numMusic, numSfx, 1, 1, aramSize);
  } else {
    sndInit(numVoices, numMusic, numSfx, 1, 0, aramSize);
  }
  DTKInit();

  mpGroupSetDB = rs_new rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >();
  mpGroupSetResNameDB = rs_new rstl::map< uint, rstl::string >();
  mpDVDTrackDB = rs_new rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >();
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  mpEmitterDB = rs_new rstl::vector< CEmitterData >(maxNumEmitters, CEmitterData(),
                                                 rstl::rmemory_allocator());
#else
  mpEmitterDB = rs_new rstl::vector< CEmitterData >(maxNumEmitters, CEmitterData());
#endif
  mpListener = rs_new SND_LISTENER;
  mIsListenerActive = false;
  mMaxNumEmitters = maxNumEmitters;

  if (OSGetSoundMode() == OS_SOUND_MODE_MONO) {
    sndOutputMode(SND_OUTPUTMODE_MONO);
    mSurroundMode = kSM_Mono;
  } else {
    sndOutputMode(SND_OUTPUTMODE_SURROUND);
    mSurroundMode = kSM_Surround;
  }

  mMaxAramUsage = aramSize;
}

CAudioSys::~CAudioSys() {
  S3dFlushAllEmitters();
  S3dRemoveListener();
  sndQuit();
  delete mpGroupSetDB;
  mpGroupSetDB = nullptr;
  delete mpGroupSetResNameDB;
  mpGroupSetResNameDB = nullptr;
  delete mpDVDTrackDB;
  mpDVDTrackDB = nullptr;

  delete mpEmitterDB;
  mpEmitterDB = nullptr;
  delete mpListener;
  mpListener = nullptr;
  mInitialized = false;
}

void CAudioSys::SysSetVolume(const uchar volume, const ushort time, const uchar group) {
  sndVolume(volume, time, group);
}

void CAudioSys::SysSetSfxVolume(const uchar volume, const ushort time, const uchar music,
                                const uchar fx) {
  sndMasterVolume(volume, time, music, fx);
}

bool CAudioSys::SysLoadGroupSet(CSimplePool* pool, const uint id) {
  const rstl::string& name = SysGetGroupSetName(id);
  rstl::rc_ptr< CAudioGroupSet > existing = FindGroupSet(name);
  if (!existing) {
    TLockedToken< CAudioGrpSetLoc > token(pool->GetObj(SObjectTag('AGSC', id)));
    rstl::ncrc_ptr< CAudioGroupSet > group(rs_new CAudioGroupSet(token));
    int aramUsage = mCurrentAramUsage + group->AramUsage();
    if (aramUsage > mMaxAramUsage) {
      return true;
    }
    mCurrentAramUsage = aramUsage;
    const rstl::string& groupName = group->GetName();
    mpGroupSetDB->insert(
        rstl::pair< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >(groupName, group));
    mpGroupSetResNameDB->insert(rstl::pair< uint, rstl::string >(id, groupName));
    return false;
  }
  existing->Reload();
  return true;
}

bool CAudioSys::SysLoadGroupSet(const CToken& token, const rstl::string& name, const uint id) {
  rstl::rc_ptr< CAudioGroupSet > existing = FindGroupSet(name);
  if (!existing) {
    rstl::ncrc_ptr< CAudioGroupSet > group(
        rs_new CAudioGroupSet(TLockedToken< CAudioGrpSetLoc >(token)));
    int aramUsage = mCurrentAramUsage + group->AramUsage();
    if (aramUsage > mMaxAramUsage) {
      return true;
    }
    mCurrentAramUsage = aramUsage;
    const rstl::string& groupName = group->GetName();
    mpGroupSetDB->insert(
        rstl::pair< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >(groupName, group));
    mpGroupSetResNameDB->insert(rstl::pair< uint, rstl::string >(id, groupName));
    return false;
  }
  existing->Reload();
  return true;
}

bool CAudioSys::SysIsGroupSetLoaded(const rstl::string& name) { return FindGroupSet(name); }

bool CAudioSys::SysUnloadSampleData(const rstl::string& name) {
  rstl::ncrc_ptr< CAudioGroupSet > groupSet = FindGroupSet(name);
  CAudioGroupSet* ptr = groupSet.GetPtr();
  if (ptr) {
    ptr->FreeSampleBuffer();
    return true;
  }
  return false;
}

bool CAudioSys::SysUnloadGroupSet(const rstl::string& name) {
  rstl::rc_ptr< CAudioGroupSet > group = FindGroupSet(name);
  if (group) {
    AUTO(it, mpGroupSetResNameDB->begin());
    while (it != mpGroupSetResNameDB->end()) {
      if (it->second == name) {
        it = mpGroupSetResNameDB->erase(it);
      } else {
        ++it;
      }
    }
    mCurrentAramUsage -= group->AramUsage();
    mpGroupSetDB->erase(name);
    return true;
  }
  return false;
}

void* CAudioSys::SampleDataUploadCallback(u32 address, u32 bytes) {
  volatile bool busy = true;
  while (busy) {
    busy = ARGetDMAStatus() != 0;
  }
  memcpy(mpSampleDataUploadBuffer, reinterpret_cast< void* >(address), bytes);
  DCFlushRange(mpSampleDataUploadBuffer, bytes);
  return mpSampleDataUploadBuffer;
}

bool CAudioSys::SysPushGroupIntoARAM(const rstl::string& name, const uchar groupId) {
  rstl::ncrc_ptr< CAudioGroupSet > groupSet = FindGroupSet(name);
  CAudioGroupSet* group = groupSet.GetPtr();
  if (group) {
    void* project = group->GetProjBuffer();
    void* samples = group->GetSampleBuffer();
    void* sampleDir = group->GetSDirBuffer();
    void* pool = group->GetPoolBuffer();
    uchar buffer[0x1020];
    mpSampleDataUploadBuffer =
        reinterpret_cast< void* >((reinterpret_cast< uintptr_t >(buffer) + 31) & ~31);
    sndSetSampleDataUploadCallback(SampleDataUploadCallback, 0x1000);
    const bool result = sndPushGroup(project, groupId, samples, sampleDir, pool);
    sndSetSampleDataUploadCallback(nullptr, 0);
    return result;
  }
  return false;
}

void CAudioSys::SysPopGroupFromARAM() { sndPopGroup(); }

const rstl::string& CAudioSys::SysGetGroupSetName(const uint id) {
  rstl::map< uint, rstl::string >::const_iterator it = mpGroupSetResNameDB->find(id);
  rstl::map< uint, rstl::string >::const_iterator end = mpGroupSetResNameDB->end();
  if (it != end) {
    return it->second;
  }
  return mpDefaultInvalidString;
}

rstl::ncrc_ptr< CAudioGroupSet > CAudioSys::FindGroupSet(const rstl::string& name) {
  if (mpGroupSetDB->size() <= 0) {
    return rstl::ncrc_ptr< CAudioGroupSet >();
  }
  rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >::const_iterator it(
      mpGroupSetDB->find(name));
  rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >::const_iterator end(
      mpGroupSetDB->end());
  if (it != end) {
    return it->second;
  }
  return rstl::ncrc_ptr< CAudioGroupSet >();
}

SND_VOICEID CAudioSys::SfxStart(const SND_FXID sfxId, const uchar vol, const uchar pan,
                                const uchar prio) {
  const uchar scaledVol = (mVolumeScale * (vol > 0x7f ? 0x7f : vol)) / 0x7f;
  return sndFXStartEx(sfxId, scaledVol, pan, prio);
}

void CAudioSys::SfxStop(const SND_VOICEID handle) { sndFXKeyOff(handle); }

SND_VOICEID CAudioSys::SfxCheck(SND_VOICEID handle) { return sndFXCheck(handle); }

void CAudioSys::SfxSpan(SND_VOICEID handle, const uchar span) {
  sndFXSurroundPanning(handle, span);
}

void CAudioSys::SfxVolume(SND_VOICEID handle, const u8 vol) { sndFXVolume(handle, vol); }

void CAudioSys::SfxPitchBend(SND_VOICEID handle, const ushort pitch) {
  sndFXPitchBend(handle, pitch);
}

void CAudioSys::SfxCtrl(const SND_VOICEID handle, uchar ctrl, uchar val) {
  sndFXCtrl(handle, ctrl, val);
}

int CAudioSys::TrkQueueTrack(const rstl::string& name, void (*callback)(u32),
                             const uint eventMask) {
  while (TrkGetState() == 3) {
  }

  rstl::ncrc_ptr< CTrkData > trk = FindTrack(name);
  if (!trk.GetPtr()) {
    CTrkData* data = rs_new CTrkData(name);
    rstl::ncrc_ptr< CTrkData > newTrk(data);
    mpDVDTrackDB->insert(rstl::pair< rstl::string, rstl::ncrc_ptr< CTrkData > >(name, newTrk));
    return DTKQueueTrack(newTrk->GetFileName(), newTrk->GetTrack(), eventMask, callback);
  }

  if (!trk->GetIsTrackInUse()) {
    trk->SetIsTrackInUse(true);
    return DTKQueueTrack(trk->GetFileName(), trk->GetTrack(), eventMask, callback);
  }
  return 5000;
}

static void track_flusher_callback() {}

void CAudioSys::TrkFlushTracks() {
  if (mpDVDTrackDB->size() > 0) {
    DTKFlushTracks(nullptr);
    mpDVDTrackDB->clear();
  }
}

void CAudioSys::TrkSetSampleRate(const ETRKSampleRate rate) { DTKSetSampleRate(rate); }

void CAudioSys::TrkSetRepeatMode(const ETRKRepeatMode mode) { DTKSetRepeatMode(mode); }

void CAudioSys::TrkSetState(const ETRKPlayState state) { DTKSetState(state); }

ETRKPlayState CAudioSys::TrkGetState() { return ETRKPlayState(DTKGetState()); }

void CAudioSys::TrkSetVolume(const uchar left, const uchar right) { DTKSetVolume(left, right); }

void CAudioSys::TrkNextTrack() { DTKNextTrack(); }

rstl::ncrc_ptr< CAudioSys::CTrkData > CAudioSys::FindTrack(const rstl::string& name) {
  if (mpDVDTrackDB->size() <= 0) {
    return rstl::ncrc_ptr< CTrkData >();
  }
  rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >::const_iterator it(
      mpDVDTrackDB->find(name));
  rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >::const_iterator end(mpDVDTrackDB->end());
  if (it != end) {
    return it->second;
  }
  return rstl::ncrc_ptr< CTrkData >();
}

void CAudioSys::S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                               const CVector3f& up, const float frontSur, const float backSur,
                               const float soundSpeed, const uint flags, const uchar volume) {
  if (mIsListenerActive) {
    S3dRemoveListener();
  }

  SND_FVECTOR _pos;
  _pos.x = pos.GetX();
  _pos.y = pos.GetY();
  _pos.z = pos.GetZ();
  SND_FVECTOR _dir;
  _dir.x = dir.GetX();
  _dir.y = dir.GetY();
  _dir.z = dir.GetZ();
  SND_FVECTOR _heading;
  _heading.x = heading.GetX();
  _heading.y = heading.GetY();
  _heading.z = heading.GetZ();
  SND_FVECTOR _up;
  _up.x = up.GetX();
  _up.y = up.GetY();
  _up.z = up.GetZ();
  mIsListenerActive = true;
  sndAddListener(mpListener, &_pos, &_dir, &_heading, &_up, frontSur, backSur, soundSpeed, flags,
                 volume, nullptr);
}

bool CAudioSys::S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                  const CVector3f& heading, const CVector3f& up,
                                  const uchar volume) {
  if (mIsListenerActive) {
    SND_FVECTOR _pos;
    _pos.x = pos.GetX();
    _pos.y = pos.GetY();
    _pos.z = pos.GetZ();
    SND_FVECTOR _dir;
    _dir.x = dir.GetX();
    _dir.y = dir.GetY();
    _dir.z = dir.GetZ();
    SND_FVECTOR _heading;
    _heading.x = heading.GetX();
    _heading.y = heading.GetY();
    _heading.z = heading.GetZ();
    SND_FVECTOR _up;
    _up.x = up.GetX();
    _up.y = up.GetY();
    _up.z = up.GetZ();
    return sndUpdateListener(mpListener, &_pos, &_dir, &_heading, &_up, volume, nullptr);
  }
  return false;
}

bool CAudioSys::S3dRemoveListener() {
  if (mIsListenerActive) {
    mIsListenerActive = false;
    return sndRemoveListener(mpListener);
  }
  return false;
}

uint CAudioSys::S3dAddEmitterParaEx(const C3DEmitterParmData& params, ushort groupId,
                                    SND_PARAMETER_INFO* paraInfo) {

  uint handle = mUnusedEmitterHandle;
  if (handle == -1) {
    uint lowerHandle = S3dFindLowerPriorityHandle(params.x29_prio);
    if (lowerHandle == -1) {
      return -1;
    }
    handle = lowerHandle;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];

  SND_FVECTOR _pos;
  _pos.x = params.x0_pos.GetX();
  _pos.y = params.x0_pos.GetY();
  _pos.z = params.x0_pos.GetZ();
  SND_FVECTOR _dir;
  _dir.x = params.xc_dir.GetX();
  _dir.y = params.xc_dir.GetY();
  _dir.z = params.xc_dir.GetZ();

  const uchar scaledMaxVol =
      (mVolumeScale * (params.x26_maxVol > 0x7f ? 0x7f : params.x26_maxVol)) / 0x7f;
  const char maxVol = scaledMaxVol;
  const uchar minVol =
      (mVolumeScale * (params.x27_minVol > 0x7f ? 0x7f : params.x27_minVol)) / 0x7f;
  sndAddEmitterParaEx(&data.x0_emitter, &_pos, &_dir, params.x18_maxDist, params.x1c_distComp,
                      params.x20_flags, params.x24_sfxId, groupId, maxVol, minVol, nullptr,
                      paraInfo);
  data.x50_used = true;
  data.x51_important = params.x28_important;
  data.x52_prio = params.x29_prio;
  mUnusedEmitterHandle = S3dFindUnusedHandle();
  return handle;
}

const bool CAudioSys::S3dUpdateEmitter(const uint handle, const CVector3f& pos,
                                       const CVector3f& dir, const uchar maxVol) {
  if (handle == -1) {
    return false;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];

  SND_FVECTOR _pos;
  _pos.x = pos.GetX();
  _pos.y = pos.GetY();
  _pos.z = pos.GetZ();
  SND_FVECTOR _dir;
  _dir.x = dir.GetX();
  _dir.y = dir.GetY();
  _dir.z = dir.GetZ();
  return sndUpdateEmitter(&data.x0_emitter, &_pos, &_dir, maxVol, nullptr);
}
const bool CAudioSys::S3dRemoveEmitter(uint handle) {
  if (handle == -1) {
    return false;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.x50_used) {
    data.x50_used = false;
    mUnusedEmitterHandle = handle;
    return sndRemoveEmitter(&data.x0_emitter);
  }

  return true;
}

void CAudioSys::S3dFlushAllEmitters() {
  rstl::vector< CEmitterData >::iterator iter = mpEmitterDB->begin();
  for (; iter != mpEmitterDB->end(); ++iter) {
    if (!iter->x50_used) {
      continue;
    }

    iter->x50_used = false;
    sndRemoveEmitter(&iter->x0_emitter);
  }
  mUnusedEmitterHandle = 0;
}

void CAudioSys::S3dFlushUnusedEmitters() {
  rstl::vector< CEmitterData >::iterator iter = mpEmitterDB->begin();
  for (; iter != mpEmitterDB->end(); ++iter) {
    if (!iter->x50_used || sndCheckEmitter(&iter->x0_emitter) || iter->x51_important) {
      continue;
    }

    iter->x50_used = false;
    sndRemoveEmitter(&iter->x0_emitter);
  }
}

const bool CAudioSys::S3dCheckEmitter(const uint handle) {
  if (handle == -1) {
    return false;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.x50_used) {
    return sndCheckEmitter(&data.x0_emitter);
  }

  return false;
}

uint CAudioSys::S3dEmitterVoiceID(const uint handle) {
  if (handle == -1) {
    return 0;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.x50_used) {
    return sndEmitterVoiceID(&data.x0_emitter);
  }

  return -1;
}

uint CAudioSys::S3dFindUnusedHandle() {
  int i = 0;
  do {
    if (!(*mpEmitterDB)[i].x50_used) {
      break;
    }
    ++i;
  } while (i < mMaxNumEmitters);

  if (i < mMaxNumEmitters) {
    return i;
  }

  return -1;
}

uint CAudioSys::S3dFindLowerPriorityHandle(const uint prio) {
  int max = mMaxNumEmitters;
  int i = 0;
  do {
    CEmitterData& data = (*mpEmitterDB)[i];
    if (!data.x50_used) {
      break;
    }

    if (data.x52_prio <= prio && !data.x51_important) {
      S3dRemoveEmitter(i);
      break;
    }
    ++i;
  } while (i < max);
  return i < max ? i : -1;
}

uint CAudioSys::SeqPlayEx(const ushort gid, const ushort sid, void* arrfile, SND_PLAYPARA* para,
                          const uchar studio) {
  return sndSeqPlayEx(gid, sid, arrfile, para, studio);
}

void CAudioSys::SeqStop(u32 seqId) { sndSeqStop(seqId); }

void CAudioSys::SeqVolume(const u8 volume, const u16 time, const u32 seqId, const u8 mode) {
  sndSeqVolume(volume, time, seqId, mode);
}

void CAudioSys::SetStereoMode(const bool mode) {
  if (mode) {
    sndOutputMode(SND_OUTPUTMODE_STEREO);
    OSSetSoundMode(OS_SOUND_MODE_STEREO);
  } else {
    sndOutputMode(SND_OUTPUTMODE_MONO);
    OSSetSoundMode(OS_SOUND_MODE_MONO);
  }
}

void CAudioSys::SetSurroundMode(const ESurroundModes mode) {
  switch (mode) {
  case kSM_Mono:
    SetStereoMode(false);
    break;
  case kSM_Stereo:
    SetStereoMode(true);
    break;
  case kSM_Surround:
    SetStereoMode(true);
    sndOutputMode(SND_OUTPUTMODE_SURROUND);
    break;
  }

  mSurroundMode = mode;
}

bool CAudioSys::GetVerbose() { return mVerbose; }

void CAudioSys::SetVolumeScale(const short scale) { mVolumeScale = scale; }

void CAudioSys::SetDefaultVolumeScale(const short scale) { mDefaultVolumeScale = scale; }

short CAudioSys::GetDefaultVolumeScale() { return mDefaultVolumeScale; }

void CAudioSys::EnableAICallback(const bool enable) {
  if (mAICallbackEnabled != enable) {
    mAICallbackEnabled = enable;
    if (enable) {
      AIRegisterDMACallback(reinterpret_cast< AIDCallback >(mAICallback));
    } else {
      mAICallback = reinterpret_cast< void* >(AIRegisterDMACallback(nullptr));
    }
  }
}

bool CAudioSys::IsAICallbackEnabled() { return mAICallbackEnabled; }
