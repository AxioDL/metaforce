#include "CDSPAudioPlayer.hpp"
#include <memory>

namespace urde
{

struct SDSPStreamCacheEntry
{
    std::string x0_fileName;
    u32 x10_;
    u32 x14_;
    float x18_;
    float x1c_;
    u32 x20_;
    float x24_;
    bool x28_;
};

/* Standard DSPADPCM header */
struct dspadpcm_header
{
    uint32_t x0_num_samples;
    uint32_t x4_num_nibbles;
    uint32_t x8_sample_rate;
    uint16_t xc_loop_flag;
    uint16_t xe_format; /* 0 for ADPCM */
    uint32_t x10_loop_start_nibble;
    uint32_t x14_loop_end_nibble;
    uint32_t x18_ca;
    int16_t x1c_coef[16];
    int16_t x3c_gain;
    int16_t x3e_ps;
    int16_t x40_hist1;
    int16_t x42_hist2;
    int16_t x44_loop_ps;
    int16_t x46_loop_hist1;
    int16_t x48_loop_hist2;
    uint16_t x4a_pad[11];
};

struct CDSPStreamManager
{
    dspadpcm_header x0_header;
    std::string x60_fileName; // arg1
    union
    {
        u8 dummy = 0;
        struct
        {
            bool x70_24_claimed : 1;
            bool x70_25_ : 1;
            u8 x70_26_ : 2;
        };
    };
    u8 x71_ = -1;
    u8 x72_ = -1;
    u8 x73_arg3;
    u8 x74_arg4;
    u32 x78_handleId; // arg2
    //DVDFileInfo x80_dvdHandle;
};

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
