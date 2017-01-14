#include "CDSPAudioPlayer.hpp"
#include <memory>

namespace urde
{

struct SDSPStreamInfo
{
    const char* x0_fileName;
    u32 x4_;
    u32 x8_;
    u32 xc_;
    u32 x10_;
    u32 x14_;
    u32 x18_adpcmOffset;
    u32 x1c_;
    u32 x20_;
    u32 x24_adpcmCur;
    u32 x28_;
    u32 x2c_;
    u32 x30_;
    u32 x34_;
    u32 x38_;
};

struct SDSPStream
{
    bool x0_active;
    u32 x4_ownerId;
    SDSPStream* x8_stereoLeft;
    SDSPStream* xc_stereoRight;
    SDSPStreamInfo x10_info;
    u8 x4c_vol;
    u8 x4d_pan;
    //DVDFileInfo x50_dvdHandle1;
    //DVDFileInfo x8c_dvdHandle2;
    u32 xc8_streamId = -1; // MusyX stream handle
    u32 xcc_adpcmCur2;
    std::unique_ptr<u8[]> xd4_ringBuffer;
    u32 xd8_ringBytes = 0x11DC0; // 73152 4sec in ADPCM bytes
    u32 xdc_ringSamples = 0x1f410; // 128016 4sec in samples
    bool xe0_curBuffer;
    u32 xe8_silent;
    u8 xec_readState; // 0: NoRead 1: Read 2: ReadWrap

    void UpdateStreamVolume(u8 vol)
    {
        x4c_vol = vol;
        if (!x0_active || xe8_silent)
            return;
        //sndStreamMixParameterEx(xc8_streamId, x4c_vol, x4d_pan, 0, 0, 0);
    }

    void SilenceStream()
    {
        if (!x0_active || xe8_silent)
            return;
        //sndStreamMixParameterEx(xc8_streamId, 0, x4d_pan, 0, 0, 0);
        xe8_silent = true;
    }
};

}
