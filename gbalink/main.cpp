#include <stdio.h>
#include "GCNTypes.hpp"
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include "hecl/hecl.hpp"

namespace net
{

/** IP address class derived from SFML */
class IPAddress
{
    uint32_t m_address = 0;
    bool m_valid = false;

    void resolve(const std::string& address)
    {
        m_address = 0;
        m_valid = false;

        if (address == "255.255.255.255")
        {
            /* The broadcast address needs to be handled explicitly,
             * because it is also the value returned by inet_addr on error */
            m_address = INADDR_BROADCAST;
            m_valid = true;
        }
        else if (address == "0.0.0.0")
        {
            m_address = INADDR_ANY;
            m_valid = true;
        }
        else
        {
            /* Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx") */
            uint32_t ip = inet_addr(address.c_str());
            if (ip != INADDR_NONE)
            {
                m_address = ip;
                m_valid = true;
            }
            else
            {
                /* Not a valid address, try to convert it as a host name */
                addrinfo hints;
                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_INET;
                addrinfo* result = NULL;
                if (getaddrinfo(address.c_str(), NULL, &hints, &result) == 0)
                {
                    if (result)
                    {
                        ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
                        freeaddrinfo(result);
                        m_address = ip;
                        m_valid = true;
                    }
                }
            }
        }
    }

public:
    IPAddress(const std::string& address)
    {
        resolve(address);
    }

    uint32_t toInteger() const
    {
        return ntohl(m_address);
    }

    operator bool() const { return m_valid; }
};

/** Server-oriented TCP socket class derived from SFML */
class Socket
{
    int m_socket = -1;
    bool m_isBlocking;

    static sockaddr_in createAddress(uint32_t address, unsigned short port)
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = htonl(address);
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);

#ifdef __APPLE__
        addr.sin_len = sizeof(addr);
#endif

        return addr;
    }

    bool openSocket()
    {
        if (isOpen())
            return false;

        m_socket = socket(PF_INET, SOCK_STREAM, 0);
        if (m_socket == -1)
        {
            //err() << "Can't allocate socket" << std::endl;
            return false;
        }

        int one = 1;
        setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&one), sizeof(one));
#ifdef __APPLE__
        setsockopt(m_socket, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&one), sizeof(one));
#endif

        setBlocking(m_isBlocking);

        return true;
    }

    void setRemoteSocket(int remSocket)
    {
        m_socket = remSocket;
        setBlocking(m_isBlocking);
    }

public:
    Socket(bool blocking)
    : m_isBlocking(blocking) {}
    ~Socket() { close(); }

    void setBlocking(bool blocking)
    {
        int status = fcntl(m_socket, F_GETFL);
        if (m_isBlocking)
            fcntl(m_socket, F_SETFL, status | ~O_NONBLOCK);
        else
            fcntl(m_socket, F_SETFL, status | O_NONBLOCK);
    }

    bool isOpen() const { return m_socket != -1; }
    bool openAndListen(const IPAddress& address, uint32_t port)
    {
        if (!openSocket())
            return false;

        sockaddr_in addr = createAddress(address.toInteger(), port);
        if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
        {
            /* Not likely to happen, but... */
            //err() << "Failed to bind listener socket to port " << port << std::endl;
            return false;
        }

        if (::listen(m_socket, 0) == -1)
        {
            /* Oops, socket is deaf */
            //err() << "Failed to listen to port " << port << std::endl;
            return false;
        }

        return true;
    }

    bool accept(Socket& remoteSocketOut)
    {
        if (!isOpen())
            return false;

        /* Accept a new connection */
        sockaddr_in address;
        socklen_t length = sizeof(address);
        int remoteSocket = ::accept(m_socket, reinterpret_cast<sockaddr*>(&address), &length);

        /* Check for errors */
        if (remoteSocket == -1)
            return false;

        /* Initialize the new connected socket */
        remoteSocketOut.close();
        remoteSocketOut.setRemoteSocket(remoteSocket);

        return true;
    }

    void close()
    {
        if (!isOpen())
            return;
        ::close(m_socket);
        m_socket = -1;
    }

    ssize_t read(void* buf, size_t len)
    {
        return ::read(m_socket, buf, len);
    }

    ssize_t write(const void* buf, size_t len)
    {
        return ::write(m_socket, buf, len);
    }
};

}

class CGBASupport
{
public:
    enum class EPhase
    {
        LoadClientPad,
        Standby,
        StartProbeTimeout,
        PollProbe,
        StartJoyBusBoot,
        PollJoyBusBoot,
        DataTransfer,
        Complete,
        Failed
    };

private:
    u32 x28_fileSize;
    std::unique_ptr<u8[]> x2c_buffer;
    EPhase x34_phase = EPhase::LoadClientPad;
    float x38_timeout = 0.f;
    u8 x3c_status = 0;
    u32 x40_siChan = -1;
    bool x44_fusionLinked = false;
    bool x45_fusionBeat = false;
    static CGBASupport* SharedInstance;

public:
    CGBASupport(const char* clientPadPath);
    ~CGBASupport();
    bool PollResponse();
    void Update(float dt);
    bool IsReady();
    void InitializeSupport();
    void StartLink();
    EPhase GetPhase() const { return x34_phase; }
    bool IsFusionLinked() const { return x44_fusionLinked; }
    bool IsFusionBeat() const { return x45_fusionBeat; }
};

CGBASupport* CGBASupport::SharedInstance;

net::Socket DataServer = {true};
net::Socket DataSocket = {true};
net::Socket ClockServer = {true};
net::Socket ClockSocket = {true};

static u64 GetGCTicks()
{
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    return nanos * 486000000 / 1000000000;
}

#define GBA_JSTAT_MASK                  0x3a
#define GBA_JSTAT_FLAGS_SHIFT           4
#define GBA_JSTAT_FLAGS_MASK            0x30
#define GBA_JSTAT_PSF1                  0x20
#define GBA_JSTAT_PSF0                  0x10
#define GBA_JSTAT_SEND                  0x08
#define GBA_JSTAT_RECV                  0x02

#define GBA_READY                       0
#define GBA_NOT_READY                   1
#define GBA_BUSY                        2
#define GBA_JOYBOOT_UNKNOWN_STATE       3
#define GBA_JOYBOOT_ERR_INVALID         4

static void GBAInit()
{
    DataServer.openAndListen(net::IPAddress("0.0.0.0"), 0xd6ba);
    printf("data listening\n");
    ClockServer.openAndListen(net::IPAddress("0.0.0.0"), 0xc10c);
    printf("clock listening\n");

    DataServer.accept(DataSocket);
    printf("data accepted\n");
    ClockServer.accept(ClockSocket);
    printf("clock accepted\n");
}

static s32 GBAGetProcessStatus(s32 chan, u8* percentp)
{
    return GBA_READY;
}

static s32 GBAGetStatus(s32 chan, u8* status)
{
    return GBA_READY;
}

static s32 GBAReset(s32 chan, u8* status)
{
    return GBA_READY;
}

static s32 GBARead(s32 chan, u8* dst, u8* status)
{
    return GBA_READY;
}

static s32 GBAWrite(s32 chan, u8* src, u8* status)
{
    return GBA_READY;
}

static const u8 KawasedoLUT[] =
{
  0x18, 0xFC, 0xC0, 0x80, 0x7F, 0x40, 0x3F, 0x01, 0x00, 0x2F,
  0x2F, 0x20, 0x43, 0x6F, 0x64, 0x65, 0x64, 0x20, 0x62, 0x79,
  0x20, 0x4B, 0x61, 0x77, 0x61, 0x73, 0x65, 0x64, 0x6F, 0x00,
  0x00, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xAC, 0xC4,
  0xF8, 0x08, 0x10, 0xBF, 0x18, 0x00, 0x00, 0x00
};

/** Self-contained class for solving Kawasedo's GBA BootROM challenge.
 *  GBA will boot client_pad.bin code on completion. */
class CKawasedoChallenge
{
    /** DSP-hosted HMAC function
     *  Reference: https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Core/HW/DSPHLE/UCodes/GBA.cpp */
    struct DSPSecParms
    {
        union ShortAndLong
        {
            u16 s[2];
            u32 l;
        };

        ShortAndLong x0_gbaK1; /* Challenge key first read from GBA */
        ShortAndLong x4_pColor; /* Palette of Nintendo logo */
        ShortAndLong x8_pSpeed; /* Speed of logo pulsing */
        u32 xc_progLen; /* Length of program to boot */
        //u32* x10_resultsDest; /* Written to resK1 and resK2 instead */

        u32 x20_resK1; /* Transformed key */
        u32 x24_resMAC; /* Message authentication code */

        void ProcessGBACrypto()
        {
            // 32 bytes from mram addr to DRAM @ 0
            x0_gbaK1.l = hecl::SBig(x0_gbaK1.l);

            // This is the main decrypt routine
            u16 x11 = 0, x12 = 0, x20 = 0, x21 = 0, x22 = 0, x23 = 0;

            x20 = hecl::SBig(x0_gbaK1.s[0]) ^ 0x6f64;
            x21 = hecl::SBig(x0_gbaK1.s[1]) ^ 0x6573;

            s16 unk2 = (s8)x8_pSpeed.s[0];
            if (unk2 < 0)
                x11 = ((~unk2 + 3) << 1) | (x4_pColor.s[0] << 4);
            else if (unk2 == 0)
                x11 = (x4_pColor.s[0] << 1) | 0x70;
            else  // unk2 > 0
                x11 = ((unk2 - 1) << 1) | (x4_pColor.s[0] << 4);

            s32 rounded_sub = ((xc_progLen + 7) & ~7) - 0x200;
            u16 size = (rounded_sub < 0) ? 0 : rounded_sub >> 3;

            u32 t = (((size << 16) | 0x3f80) & 0x3f80ffff) << 1;
            s16 t_low = (s8)(t >> 8);
            t += (t_low & size) << 16;
            x12 = t >> 16;
            x11 |= (size & 0x4000) >> 14;
            t = ((x11 & 0xff) << 16) + ((x12 & 0xff) << 16) + (x12 << 8);

            u16 final11 = 0, final12 = 0;
            final11 = x11 | ((t >> 8) & 0xff00) | 0x8080;
            final12 = x12 | 0x8080;

            if ((final12 & 0x200) != 0)
            {
                x22 = final11 ^ 0x6f64;
                x23 = final12 ^ 0x6573;
            }
            else
            {
                x22 = final11 ^ 0x6177;
                x23 = final12 ^ 0x614b;
            }

            // Send the result back to mram
            x20_resK1 = (x20 << 16) | x21;
            x24_resMAC = (x22 << 16) | x23;
        }
    } xf8_dspHmac;

    u32 m_chan;

    s32 x0_pColor;
    s32 x4_pSpeed;
    u8* x8_progPtr;
    u32 xc_progLen;
    u8* x10_statusPtr;
    void* x14_callback;
    u8 x18_readBuf[4];
    u8 x1c_writeBuf[4];
    u32 x20_xfMAC;
    u64 x28_ticksAfterXf;
    u32 x30_xfDone;
    u32 x34_secret;
    u32 x38_xfSecret;
    u32 x3c_[7];

    u32 x58_resK1;
    u32 x5c_resMAC;
    u32 x60_progChecksum;
    u32 x64_intermediateMAC;

    bool F23(u8 status)
    {
        if (status != GBA_READY || GBAReset(m_chan, x10_statusPtr) != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    bool F25(u8 status)
    {
        if (status != GBA_READY || *x10_statusPtr != KawasedoLUT[0x25] ||
            GBAGetStatus(m_chan, x10_statusPtr) != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    bool F27(u8 status)
    {
        if (status != GBA_READY || *x10_statusPtr != KawasedoLUT[0x0] ||
            GBARead(m_chan, x18_readBuf, x10_statusPtr) != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    bool F29(u8 status)
    {
        if (status != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    void GBAX02()
    {
        xf8_dspHmac.x0_gbaK1.l = reinterpret_cast<u32&>(x18_readBuf);
        xf8_dspHmac.x4_pColor.l = x0_pColor;
        xf8_dspHmac.x8_pSpeed.l = x4_pSpeed;
        xf8_dspHmac.xc_progLen = xc_progLen;
        xf8_dspHmac.ProcessGBACrypto();
    }

    bool GBAX01()
    {
        x58_resK1 = xf8_dspHmac.x20_resK1;
        x5c_resMAC = xf8_dspHmac.x24_resMAC;

        x20_xfMAC = ~KawasedoLUT[0x24] & (KawasedoLUT[0x24] + xc_progLen);
        u32 tmp = KawasedoLUT[0x14] << KawasedoLUT[0x21];
        if (x20_xfMAC < tmp)
            x20_xfMAC = tmp;
        x64_intermediateMAC = x20_xfMAC;
        x20_xfMAC -= tmp;
        x20_xfMAC >>= KawasedoLUT[0x20];

        reinterpret_cast<u32&>(x1c_writeBuf) = x5c_resMAC;

        x28_ticksAfterXf = GetGCTicks();
        x30_xfDone = 1;

        if (GBAWrite(m_chan, x1c_writeBuf, x10_statusPtr) != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    bool F31(u8 status)
    {
        if (status != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        for (;;)
        {
            if (x30_xfDone)
            {
                x30_xfDone = 0;
            }
            else
            {
                if (!(*x10_statusPtr & KawasedoLUT[0x14]) ||
                    (*x10_statusPtr & KawasedoLUT[0x2a]) >> KawasedoLUT[0x21] !=
                    (x34_secret & KawasedoLUT[0x21]) >> KawasedoLUT[0x1f])
                    return false;
                x34_secret -= (KawasedoLUT[0x19] - KawasedoLUT[0x17]);
            }

            if (x34_secret <= x64_intermediateMAC)
            {
                u32 checksum;
                if (x34_secret != x64_intermediateMAC)
                {
                    x20_xfMAC = KawasedoLUT[0x1d];
                    checksum = KawasedoLUT[0x1d];
                    while (x20_xfMAC < KawasedoLUT[0x21])
                    {
                        if (xc_progLen)
                        {
                            checksum |= *x8_progPtr++ << (x20_xfMAC * KawasedoLUT[0x25]);
                            --xc_progLen;
                        }
                    }

                    if (x34_secret == KawasedoLUT[0x26])
                    {
                        x60_progChecksum = checksum;
                    }
                    else if (KawasedoLUT[0x27] == x34_secret)
                    {
                        checksum = m_chan << KawasedoLUT[0x25];
                    }

                    if (x34_secret >= KawasedoLUT[0x2])
                    {
                        u32 checksum2 = checksum;
                        u32 tmp = KawasedoLUT[0x14];
                        u32 tmpSecret = x38_xfSecret;
                        u32 xorTerm = (KawasedoLUT[0x26] << 8) +
                            (((KawasedoLUT[0x2b] - (KawasedoLUT[0x2b] << 4)) +
                            KawasedoLUT[0x28]) - KawasedoLUT[0x23]);
                        while (tmp > KawasedoLUT[0x1e])
                        {
                            if (checksum2 ^ tmpSecret)
                                tmpSecret = (tmpSecret >> 1) ^ xorTerm;
                            else
                                tmpSecret >>= 1;

                            ++checksum2;
                            --tmp;
                        }
                        x38_xfSecret = tmpSecret;
                    }

                    if (x34_secret == KawasedoLUT[0x28] + 256)
                    {
                        x3c_[0] = checksum;
                    }
                    else if (x34_secret == KawasedoLUT[0x1] + 256)
                    {
                        x20_xfMAC = KawasedoLUT[0x7];
                        x3c_[x20_xfMAC] = checksum;
                    }
                }
                else
                {
                    checksum = x38_xfSecret | x34_secret << 16;
                }

                if (x34_secret > KawasedoLUT[0x2b])
                {
                    x58_resK1 = ((KawasedoLUT[0x18] << KawasedoLUT[0x25]) | KawasedoLUT[0x15] |
                        (KawasedoLUT[0x18] << KawasedoLUT[0x2c]) | (KawasedoLUT[0x17] << KawasedoLUT[0x2a])) *
                        x58_resK1 - (KawasedoLUT[0x1b] - KawasedoLUT[0x1a]);

                    checksum ^= x58_resK1;
                    checksum ^= -((KawasedoLUT[0xb] << 20) + x34_secret);
                    checksum ^= KawasedoLUT[0xb] | (KawasedoLUT[0x13] << 8) | (KawasedoLUT[0x12] << 16);
                }

                x1c_writeBuf[3] = checksum >> KawasedoLUT[0x0];
                x1c_writeBuf[0] = checksum >> KawasedoLUT[0x1e];
                x1c_writeBuf[1] = checksum >> KawasedoLUT[0x29];
                x1c_writeBuf[2] = checksum >> KawasedoLUT[0x2a];

                if (x34_secret == KawasedoLUT[0x1] + KawasedoLUT[0x1])
                    x3c_[2] = checksum;

                if (x20_xfMAC < KawasedoLUT[0x21])
                {
                    x3c_[(3 - (1 - x20_xfMAC))] = checksum;
                    x3c_[5 - x20_xfMAC] = x3c_[(2 - (1 - x20_xfMAC))] * x3c_[4 - x20_xfMAC];
                    x3c_[(5 - (1 - x20_xfMAC))] = x3c_[(2 - (1 - x20_xfMAC))] * x3c_[1 - x20_xfMAC];
                    x3c_[7 - x20_xfMAC] = x3c_[-(1 - x20_xfMAC)] * x3c_[4 - x20_xfMAC];
                }

                if (GBAWrite(m_chan, x1c_writeBuf, x10_statusPtr) != GBA_READY)
                {
                    x28_ticksAfterXf = 0;
                    x14_callback = nullptr;
                    return false;
                }
                continue;
            }
            else // x34_secret > x64_intermediateMAC
            {
                if (GBARead(m_chan, x18_readBuf, x10_statusPtr) != GBA_READY)
                {
                    x28_ticksAfterXf = 0;
                    x14_callback = nullptr;
                    return false;
                }
            }
            break;
        }

        return true;
    }

    bool F33(u8 status)
    {
        if (status != GBA_READY ||
            GBAGetStatus(m_chan, x10_statusPtr) != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    bool F35(u8 status)
    {
        for (;;)
        {
            if (status != GBA_READY || (*x10_statusPtr & (GBA_JSTAT_FLAGS_MASK | GBA_JSTAT_RECV)))
            {
                x28_ticksAfterXf = 0;
                x14_callback = nullptr;
                return false;
            }

            if (*x10_statusPtr == GBA_JSTAT_SEND)
            {
                if (GBAGetStatus(m_chan, x10_statusPtr) != GBA_READY)
                {
                    x28_ticksAfterXf = 0;
                    x14_callback = nullptr;
                    return false;
                }
                continue;
            }

            if (GBARead(m_chan, x18_readBuf, x10_statusPtr) != GBA_READY)
            {
                x28_ticksAfterXf = 0;
                x14_callback = nullptr;
                return false;
            }

            break;
        }

        return true;
    }

    bool F37(u8 status)
    {
        if (status != GBA_READY ||
            GBAWrite(m_chan, x18_readBuf, x10_statusPtr) != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        return true;
    }

    bool F39(u8 status)
    {
        if (status != GBA_READY)
        {
            x28_ticksAfterXf = 0;
            x14_callback = nullptr;
            return false;
        }

        x28_ticksAfterXf = 0;

        return true;
    }

public:
    CKawasedoChallenge(u32 chan, s32 paletteColor, s32 paletteSpeed,
                       u8* programp, s32 length, u8* status)
    : m_chan(chan), x0_pColor(paletteColor),
      x4_pSpeed(paletteSpeed), x8_progPtr(programp),
      xc_progLen(length), x10_statusPtr(status)
    {
        x34_secret = KawasedoLUT[0x8];
    }

    bool DoChallenge()
    {
        if (GBAGetStatus(m_chan, x10_statusPtr) != GBA_READY)
        {
            x14_callback = nullptr;
            return false;
        }
        if (!F23(*x10_statusPtr))
            return false;
        if (!F25(*x10_statusPtr))
            return false;
        if (!F27(*x10_statusPtr))
            return false;
        if (!F29(*x10_statusPtr))
            return false;
        GBAX02();
        if (!GBAX01())
            return false;
        if (!F31(*x10_statusPtr))
            return false;
        if (!F33(*x10_statusPtr))
            return false;
        if (!F35(*x10_statusPtr))
            return false;
        if (!F37(*x10_statusPtr))
            return false;
        if (!F39(*x10_statusPtr))
            return false;
        return true;
    }
};

static s32 GBAJoyBootAsync(s32 chan, s32 paletteColor, s32 paletteSpeed,
                           u8* programp, s32 length, u8* status)
{
    if (chan > 3)
        return GBA_JOYBOOT_ERR_INVALID;

    if (!length || length >= 0x40000)
        return GBA_JOYBOOT_ERR_INVALID;

    if (paletteSpeed < -4 || paletteSpeed > 4)
        return GBA_JOYBOOT_ERR_INVALID;

    if (paletteColor < 0 || paletteColor > 6)
        return GBA_JOYBOOT_ERR_INVALID;

    if (programp[0xac] * programp[0xac] * programp[0xac] * programp[0xac] == 0)
        return GBA_JOYBOOT_ERR_INVALID;

    u8 tmpStatus;
    s32 ret = GBAGetProcessStatus(chan, &tmpStatus);
    if (ret != GBA_READY)
        return ret;

    CKawasedoChallenge challenge(chan, paletteColor, paletteSpeed, programp, length, status);
    challenge.DoChallenge();

    return GBA_READY;
}

static u32 calculateJBusChecksum(const u8* data, size_t len)
{
    const u8* ptr = data;
    u32 sum = -1;
    for (int i = len ; i > 0; --i)
    {
        u8 ch = *ptr;
        ptr++;
        sum ^= ch;
        for (int j = 0; j < 8; ++j)
        {
            if ((sum & 1))
            {
                sum >>= 1;
                sum ^= 0xb010;
            }
            else
                sum >>= 1;
        }
    }

    return sum;
}

CGBASupport::CGBASupport(const char* clientPadPath)
{
    FILE* fp = fopen(clientPadPath, "rb");
    if (!fp)
    {
        fprintf(stderr, "No file at %s\n", clientPadPath);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    x28_fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    x2c_buffer.reset(new u8[x28_fileSize]);
    fread(x2c_buffer.get(), 1, x28_fileSize, fp);
    fclose(fp);
    GBAInit();
    SharedInstance = this;
}

CGBASupport::~CGBASupport()
{
    SharedInstance = nullptr;
}

bool CGBASupport::PollResponse()
{
    u8 status;
    if (GBAReset(x40_siChan, &status) == GBA_NOT_READY)
        if (GBAReset(x40_siChan, &status) == GBA_NOT_READY)
            return false;

    if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
        return false;
    if (status != (GBA_JSTAT_PSF1 | GBA_JSTAT_SEND))
        return false;

    u8 bytes[4];
    if (GBARead(x40_siChan, bytes, &status) == GBA_NOT_READY)
        return false;
    if (reinterpret_cast<u32&>(bytes) != SBIG('AMTE'))
        return false;

    if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
        return false;
    if (status != GBA_JSTAT_PSF1)
        return false;

    if (GBAWrite(x40_siChan, (unsigned char*)"AMTE", &status) == GBA_NOT_READY)
        return false;

    if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
        return false;
    if ((status & GBA_JSTAT_FLAGS_MASK) != GBA_JSTAT_FLAGS_MASK)
        return false;

    u64 profStart = GetGCTicks();
    const u64 timeToSpin = 486000000 / 8000;
    for (;;)
    {
        u64 curTime = GetGCTicks();
        if (curTime - profStart > timeToSpin)
            return true;

        if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
            continue;
        if (!(status & GBA_JSTAT_SEND))
            continue;

        if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
            continue;
        if (status != (GBA_JSTAT_FLAGS_MASK | GBA_JSTAT_SEND))
            continue;
        break;
    }

    if (GBARead(x40_siChan, bytes, &status) != GBA_READY)
        return false;

    u32 swapped = hecl::SBig(reinterpret_cast<u32&>(bytes));
    if (bytes[0] != calculateJBusChecksum(reinterpret_cast<u8*>(&swapped), 3))
        return false;

    x44_fusionLinked = (bytes[2] & 0x2) != 0;
    if (x44_fusionLinked && (bytes[2] & 0x1) != 0)
        x45_fusionBeat = true;

    return true;
}

void CGBASupport::Update(float dt)
{
    switch (x34_phase)
    {
    case EPhase::LoadClientPad:
        IsReady();
        break;

    case EPhase::StartProbeTimeout:
        x38_timeout = 4.f;
        x34_phase = EPhase::PollProbe;

    case EPhase::PollProbe:
        /* SIProbe poll normally occurs here with 4 second timeout */
        x40_siChan = 1;
        x34_phase = EPhase::StartJoyBusBoot;

    case EPhase::StartJoyBusBoot:
        x34_phase = EPhase::PollJoyBusBoot;
        GBAJoyBootAsync(x40_siChan, x40_siChan * 2, 2, x2c_buffer.get(), x28_fileSize, &x3c_status);
        break;

    case EPhase::PollJoyBusBoot:
        if (GBAGetProcessStatus(x40_siChan, &x3c_status) == GBA_BUSY)
            break;
        if (GBAGetStatus(x40_siChan, &x3c_status) == GBA_NOT_READY)
        {
            x34_phase = EPhase::Failed;
            break;
        }
        x38_timeout = 4.f;
        x34_phase = EPhase::DataTransfer;
        break;

    case EPhase::DataTransfer:
        if (PollResponse())
        {
            x34_phase = EPhase::Complete;
            break;
        }
        x38_timeout = std::max(0.f, x38_timeout - dt);
        if (x38_timeout == 0.f)
            x34_phase = EPhase::Failed;
        break;

    default: break;
    }
}

bool CGBASupport::IsReady()
{
    if (x34_phase != EPhase::LoadClientPad)
        return true;

    x34_phase = EPhase::Standby;
    /* Conveniently already little-endian */
    reinterpret_cast<u32&>(x2c_buffer[0xc8]) = u32(GetGCTicks());
    x2c_buffer[0xaf] = 'E';
    x2c_buffer[0xbd] = 0xc9;
    return true;
}

void CGBASupport::InitializeSupport()
{
    x34_phase = EPhase::Standby;
    x38_timeout = 0.f;
    x3c_status = false;
    x40_siChan = -1;
    x44_fusionLinked = false;
    x45_fusionBeat = false;
}

void CGBASupport::StartLink()
{
    x34_phase = EPhase::StartProbeTimeout;
    x40_siChan = -1;
}

int main(int argc, char** argv)
{
    CGBASupport gba("client_pad.bin");
    gba.InitializeSupport();
    gba.StartLink();
}
