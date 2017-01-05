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
#if __APPLE__
#include <mach/mach_time.h>
#endif

namespace net
{

/* Define the low-level send/receive flags, which depend on the OS */
#ifdef __linux__
static const int _flags = MSG_NOSIGNAL;
#else
static const int _flags = 0;
#endif

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
        m_isBlocking = blocking;
        int status = fcntl(m_socket, F_GETFL);
        if (m_isBlocking)
            fcntl(m_socket, F_SETFL, status & ~O_NONBLOCK);
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

    ssize_t send(const void* buf, size_t len)
    {
        if (!isOpen())
            return -1;

        if (!buf || !len)
            return -1;

        /* Loop until every byte has been sent */
        ssize_t result = 0;
        for (size_t sent = 0; sent < len; sent += result)
        {
            /* Send a chunk of data */
            result = ::send(m_socket, static_cast<const char*>(buf) + sent, len - sent, _flags);

            /* Check for errors */
            if (result < 0)
                return -1;
        }

        return len;
    }

    ssize_t recv(void* buf, size_t len)
    {
        if (!isOpen())
            return -1;

        if (!buf)
            return -1;

        if (!len)
            return 0;

        /* Receive a chunk of bytes */
        int sizeReceived = ::recv(m_socket, static_cast<char*>(buf), static_cast<int>(len), _flags);

        if (sizeReceived <= 0)
            return -1;

        return sizeReceived;
    }

    operator bool() const { return isOpen(); }

    int GetInternalSocket() const { return m_socket; }
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

static net::Socket DataServer = {false};
static net::Socket DataSocket = {false};
static net::Socket ClockServer = {false};
static net::Socket ClockSocket = {false};
static u8 Cmd = 0;
static u64 LastGCTick = 0;
static u64 TimeCmdSent = 0;
static bool Booted = false;

static u64 SysToDolphinNum;
static u64 SysToDolphinDenom;

static u64 GetGCTicks()
{
#if __APPLE__
    return mach_absolute_time() * SysToDolphinNum / SysToDolphinDenom;
#elif __linux__
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);

    return u64((tp.tv_sec * 1000000000ull) + tp.tv_nsec) * SysToDolphinNum / SysToDolphinDenom;
#else
    return 0;
#endif
}

static constexpr u64 GetGCTicksPerSec()
{
    return 486000000ull;
}

static void WaitGCTicks(u64 ticks)
{
    struct timeval tv = {};
    tv.tv_sec = ticks / GetGCTicksPerSec();
    tv.tv_usec = (ticks % GetGCTicksPerSec()) * 1000000 / GetGCTicksPerSec();
    select(0, NULL, NULL, NULL, &tv);
}

enum EJoybusCmds
{
    CMD_RESET = 0xff,
    CMD_STATUS = 0x00,
    CMD_READ = 0x14,
    CMD_WRITE = 0x15
};

static const u64 BITS_PER_SECOND = 115200;
static const u64 BYTES_PER_SECOND = BITS_PER_SECOND / 8;

static u64 GetTransferTime(u8 cmd)
{
    u64 bytes = 0;

    switch (cmd)
    {
    case CMD_RESET:
    case CMD_STATUS:
    {
        bytes = 4;
        break;
    }
    case CMD_READ:
    {
        bytes = 6;
        break;
    }
    case CMD_WRITE:
    {
        bytes = 1;
        break;
    }
    default:
    {
        bytes = 1;
        break;
    }
    }
    return bytes * GetGCTicksPerSec() / BYTES_PER_SECOND;
}

static void ClockSync()
{
    if (!ClockSocket)
        return;

    u32 TickDelta = 0;
    if (!LastGCTick)
    {
        LastGCTick = GetGCTicks();
        TickDelta = GetGCTicksPerSec() / 60;
    }
    else
        TickDelta = GetGCTicks() - LastGCTick;

    /* Scale GameCube clock into GBA clock */
    TickDelta = u32(u64(TickDelta) * 16777216 / GetGCTicksPerSec());
    LastGCTick = GetGCTicks();
    TickDelta = hecl::SBig(TickDelta);
    u8* deltaStr = reinterpret_cast<u8*>(&TickDelta);
    //printf("%02x %02x %02x %02x\n", deltaStr[0], deltaStr[1], deltaStr[2], deltaStr[3]);
    if (ClockSocket.send(&TickDelta, 4) < 0)
        ClockSocket.close();
}

static void Send(const u8* buffer)
{
    Cmd = buffer[0];

    ssize_t status;
    if (Cmd == CMD_WRITE)
        status = DataSocket.send(buffer, 5);
    else
        status = DataSocket.send(buffer, 1);

    if (Cmd != CMD_STATUS)
        Booted = true;

    if (status < 0)
        DataSocket.close();
    else
    {
        printf("Send %02x [> %02x%02x%02x%02x] (%ld)\n", buffer[0],
               buffer[1], buffer[2], buffer[3], buffer[4], status);
    }

    TimeCmdSent = GetGCTicks();
}

static size_t Receive(u8* buffer)
{
    if (!DataSocket)
        return 0;

    ssize_t recvBytes = 0;
    u64 transferTime = GetTransferTime(Cmd);
    bool block = (GetGCTicks() - TimeCmdSent) > transferTime;
    if (Cmd == CMD_STATUS && !Booted)
        block = false;

    if (block)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(DataSocket.GetInternalSocket(), &fds);
        struct timeval tv = {};
        tv.tv_sec = 1;
        select(DataSocket.GetInternalSocket() + 1, &fds, NULL, NULL, &tv);
    }

    recvBytes = DataSocket.recv(buffer, 5);
    if (recvBytes < 0)
    {
        if (errno == EAGAIN)
            recvBytes = 0;
        else
        {
            DataSocket.close();
            return 5;
        }
    }

    if (recvBytes > 5)
        recvBytes = 5;

    if (recvBytes > 0)
    {
        if (Cmd == CMD_STATUS || Cmd == CMD_RESET)
        {
            printf("Stat/Reset [< %02x%02x%02x%02x%02x] (%lu)\n",
                   (u8)buffer[0], (u8)buffer[1], (u8)buffer[2],
                   (u8)buffer[3], (u8)buffer[4], recvBytes);
        }
        else
        {
            printf("Receive [< %02x%02x%02x%02x%02x] (%lu)\n",
                   (u8)buffer[0], (u8)buffer[1], (u8)buffer[2],
                   (u8)buffer[3], (u8)buffer[4], recvBytes);
        }
    }

    return recvBytes;
}

static bool WaitingForResp = false;
static size_t DataReceivedBytes = 0;
static u64 TimeSent = 0;

static size_t RunBuffer(u8* buffer, u64& remTicks)
{
    if (!WaitingForResp)
    {
        DataReceivedBytes = 0;
        ClockSync();
        Send(buffer);
        TimeSent = GetGCTicks();
        WaitingForResp = true;
    }

    if (WaitingForResp && DataReceivedBytes == 0)
    {
        DataReceivedBytes = Receive(buffer);
    }

    u64 ticksSinceSend = GetGCTicks() - TimeSent;
    u64 targetTransferTime = GetTransferTime(Cmd);
    if (targetTransferTime > ticksSinceSend)
    {
        remTicks = targetTransferTime - ticksSinceSend;
        return 0;
    }
    else
    {
        remTicks = 0;
        if (DataReceivedBytes != 0)
            WaitingForResp = false;
        return DataReceivedBytes;
    }
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

static void SleepABit()
{
    struct timeval tv = {};
    tv.tv_usec = 1000000  / 120;
    select(0, NULL, NULL, NULL, &tv);
}

static void GBAInit()
{
    if (!DataServer.openAndListen(net::IPAddress("0.0.0.0"), 0xd6ba))
    {
        printf("data open failed %s\n", strerror(errno));
        exit(1);
    }
    printf("data listening\n");
    if (!ClockServer.openAndListen(net::IPAddress("0.0.0.0"), 0xc10c))
    {
        printf("clock open failed %s\n", strerror(errno));
        exit(1);
    }
    printf("clock listening\n");

    while (!DataSocket && !ClockSocket)
    {
        if (!DataSocket)
        {
            if (!DataServer.accept(DataSocket))
            {
                if (errno != EAGAIN)
                {
                    printf("data accept failed %d %s\n", errno, strerror(errno));
                    exit(1);
                }
            }
            else
                printf("data accepted\n");
        }
        if (!ClockSocket)
        {
            if (!ClockServer.accept(ClockSocket))
            {
                if (errno != EAGAIN)
                {
                    printf("clock accept failed %s\n", strerror(errno));
                    exit(1);
                }
            }
            else
                printf("clock accepted\n");
        }
        SleepABit();
    }

    Cmd = 0;
    Booted = false;
}

static s32 GBAGetProcessStatus(s32 chan, u8* percentp)
{
    return GBA_READY;
}

static s32 GBAGetStatus(s32 chan, u8* status)
{
    u8 buffer[] = { CMD_STATUS, 0, 0, 0, 0 };
    u64 waitTicks;
    while (!RunBuffer(buffer, waitTicks)) { WaitGCTicks(waitTicks); }
    *status = buffer[2];
    return GBA_READY;
}

static s32 GBAReset(s32 chan, u8* status)
{
    u8 buffer[] = { CMD_RESET, 0, 0, 0, 0 };
    u64 waitTicks;
    while (!RunBuffer(buffer, waitTicks)) { WaitGCTicks(waitTicks); }
    *status = buffer[2];
    return GBA_READY;
}

static s32 GBARead(s32 chan, u8* dst, u8* status)
{
    u8 buffer[] = { CMD_READ, 0, 0, 0, 0 };
    u64 waitTicks;
    while (!RunBuffer(buffer, waitTicks)) { WaitGCTicks(waitTicks); }
    *status = buffer[4];
    memmove(dst, buffer, 4);
    return GBA_READY;
}

static s32 GBAWrite(s32 chan, u8* src, u8* status)
{
    u8 buffer[] = { CMD_WRITE, src[0], src[1], src[2], src[3] };
    u64 waitTicks;
    while (!RunBuffer(buffer, waitTicks)) { WaitGCTicks(waitTicks); }
    *status = buffer[0];
    return GBA_READY;
}

/** Self-contained class for solving Kawasedo's GBA BootROM challenge.
 *  GBA will boot client_pad.bin code on completion. */
class CKawasedoChallenge
{
    /** DSP-hosted public-key unwrap and initial message crypt
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
        //u32* x10_resultsDest; /* Written to x20_publicKey and x24_initMessage instead */

        u32 x20_publicKey; /* Transformed public key */
        u32 x24_initMessage; /* Message authentication code */

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
            x20_publicKey = (x20 << 16) | x21;
            x24_initMessage = (x22 << 16) | x23;

            printf("key: %08x, len: %08x, unk1: %08x, unk2: %08x 20: %04x, 21: %04x, 22: %04x, 23: %04x\n",
                      x0_gbaK1.l, xc_progLen,
                      x4_pColor.l, x8_pSpeed.l, x20, x21, x22, x23);
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
    s32 x20_byteInWindow;
    u64 x28_ticksAfterXf;
    u32 x30_justStarted;
    u32 x34_bytesSent;
    u32 x38_crc;
    u32 x3c_checkStore[7];
    s32 x58_currentKey;
    s32 x5c_initMessage;
    s32 x60_gameId;
    u32 x64_totalBytes;

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
        if (status != GBA_READY || *x10_statusPtr != GBA_JSTAT_SEND ||
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
        if (status != GBA_READY || *x10_statusPtr != (GBA_JSTAT_PSF0 | GBA_JSTAT_SEND) ||
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
        x58_currentKey = xf8_dspHmac.x20_publicKey;
        x5c_initMessage = xf8_dspHmac.x24_initMessage;

        x20_byteInWindow = ROUND_UP_8(xc_progLen);
        if (x20_byteInWindow < 512)
            x20_byteInWindow = 512;
        x64_totalBytes = x20_byteInWindow;
        x20_byteInWindow -= 512;
        x20_byteInWindow /= 8;

        reinterpret_cast<u32&>(x1c_writeBuf) = x5c_initMessage;

        x38_crc = 0x15a0;
        x34_bytesSent = 0;

        x28_ticksAfterXf = GetGCTicks();
        x30_justStarted = 1;

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
            printf("PROG [%d/%d]\n", x34_bytesSent, x64_totalBytes);
            if (x30_justStarted)
            {
                x30_justStarted = 0;
            }
            else
            {
                if (!(*x10_statusPtr & GBA_JSTAT_PSF1) ||
                    (*x10_statusPtr & GBA_JSTAT_PSF0) >> 4 != (x34_bytesSent & 4) >> 2)
                    return false;
                x34_bytesSent += 4;
            }

            if (x34_bytesSent <= x64_totalBytes)
            {
                u32 cryptWindow;
                if (x34_bytesSent != x64_totalBytes)
                {
                    x20_byteInWindow = 0;
                    cryptWindow = 0;
                    while (x20_byteInWindow < 4)
                    {
                        if (xc_progLen)
                        {
                            cryptWindow |= *x8_progPtr++ << (x20_byteInWindow * 8);
                            --xc_progLen;
                        }
                        ++x20_byteInWindow;
                    }

                    if (x34_bytesSent == 0xac)
                    {
                        x60_gameId = cryptWindow;
                    }
                    else if (x34_bytesSent == 0xc4)
                    {
                        cryptWindow = m_chan << 0x8;
                    }

                    if (x34_bytesSent >= 0xc0)
                    {
                        u32 shiftWindow = cryptWindow;
                        u32 shiftCrc = x38_crc;
                        for (int i=0 ; i<32 ; ++i)
                        {
                            if ((shiftWindow ^ shiftCrc) & 0x1)
                                shiftCrc = (shiftCrc >> 1) ^ 0xa1c1;
                            else
                                shiftCrc >>= 1;

                            shiftWindow >>= 1;
                        }
                        x38_crc = shiftCrc;
                    }

                    if (x34_bytesSent == 0x1f8)
                    {
                        x3c_checkStore[0] = cryptWindow;
                    }
                    else if (x34_bytesSent == 0x1fc)
                    {
                        x20_byteInWindow = 1;
                        x3c_checkStore[x20_byteInWindow] = cryptWindow;
                    }
                }
                else
                {
                    cryptWindow = x38_crc | x34_bytesSent << 16;
                }

                if (x34_bytesSent > 0xbf)
                {
                    x58_currentKey = 0x6177614b * x58_currentKey + 1;

                    cryptWindow ^= x58_currentKey;
                    cryptWindow ^= -(0x2000000 + x34_bytesSent);
                    cryptWindow ^= 0x20796220;
                }

                x1c_writeBuf[0] = cryptWindow >> 0;
                x1c_writeBuf[1] = cryptWindow >> 8;
                x1c_writeBuf[2] = cryptWindow >> 16;
                x1c_writeBuf[3] = cryptWindow >> 24;

                if (x34_bytesSent == 0x1f8)
                    x3c_checkStore[2] = cryptWindow;

                if (x20_byteInWindow < 4)
                {
                    x3c_checkStore[2 + x20_byteInWindow] = cryptWindow;
                    x3c_checkStore[5 - x20_byteInWindow] = x3c_checkStore[1 + x20_byteInWindow] * x3c_checkStore[4 - x20_byteInWindow];
                    x3c_checkStore[4 + x20_byteInWindow] = x3c_checkStore[1 + x20_byteInWindow] * x3c_checkStore[1 - x20_byteInWindow];
                    x3c_checkStore[7 - x20_byteInWindow] = x3c_checkStore[-1 + x20_byteInWindow] * x3c_checkStore[4 - x20_byteInWindow];
                }

                if (GBAWrite(m_chan, x1c_writeBuf, x10_statusPtr) != GBA_READY)
                {
                    x28_ticksAfterXf = 0;
                    x14_callback = nullptr;
                    return false;
                }
                continue;
            }
            else // x34_bytesWritten > x64_totalBytes
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

            if (*x10_statusPtr != GBA_JSTAT_SEND)
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
        x34_bytesSent = 0;
    }

    bool DoChallenge()
    {
        if (GBAGetStatus(m_chan, x10_statusPtr) != GBA_READY)
        {
            x14_callback = nullptr;
            return false;
        }
        if (!F23(GBA_READY))
            return false;
        if (!F25(GBA_READY))
            return false;
        if (!F27(GBA_READY))
            return false;
        if (!F29(GBA_READY))
            return false;
        GBAX02();
        if (!GBAX01())
            return false;
        if (!F31(GBA_READY))
            return false;
        if (!F33(GBA_READY))
            return false;
        if (!F35(GBA_READY))
            return false;
        if (!F37(GBA_READY))
            return false;
        if (!F39(GBA_READY))
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
    if (!challenge.DoChallenge())
        return GBA_NOT_READY;

    return GBA_READY;
}

static u8 calculateJBusChecksum(const u8* data, size_t len)
{
    u32 sum = -1;
    for (int i = 0 ; i < len; ++i)
    {
        u8 ch = *data++;
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
    const u64 timeToSpin = GetGCTicksPerSec() / 8000;
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

    if (bytes[3] != calculateJBusChecksum(bytes, 3))
        return false;

    x44_fusionLinked = (bytes[2] & 0x2) == 0;
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
        x40_siChan = 3;
        x34_phase = EPhase::StartJoyBusBoot;

    case EPhase::StartJoyBusBoot:
        x34_phase = EPhase::PollJoyBusBoot;
        if (GBAJoyBootAsync(x40_siChan, x40_siChan * 2, 2,
                            x2c_buffer.get(), x28_fileSize, &x3c_status) != GBA_READY)
            x34_phase = EPhase::Failed;
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
    //reinterpret_cast<u32&>(x2c_buffer[0xc8]) = u32(GetGCTicks());
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
#if __APPLE__
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    SysToDolphinNum = GetGCTicksPerSec() * timebase.numer;
    SysToDolphinDenom = 1000000000ull * timebase.denom;
#elif __linux__
    SysToDolphinNum = GetGCTicksPerSec();
    SysToDolphinDenom = 1000000000ull;
#endif

    CGBASupport gba("client_pad.bin");
    gba.Update(0.f);
    gba.InitializeSupport();
    gba.StartLink();

    printf("Waiting 5 sec\n");
    s64 waitedTicks = 0;
    while (waitedTicks < GetGCTicksPerSec() * 5)
    {
        s64 start = GetGCTicks();
        u8 status;
        GBAGetStatus(1, &status);
        s64 end = GetGCTicks();
        waitedTicks += end - start;

        WaitGCTicks(GetGCTicksPerSec() * 0.684);
        waitedTicks += GetGCTicksPerSec() * 0.684;

        start = GetGCTicks();
        GBAGetStatus(1, &status);
        end = GetGCTicks();
        waitedTicks += end - start;

        WaitGCTicks(GetGCTicksPerSec() * 0.066);
        waitedTicks += GetGCTicksPerSec() * 0.066;
    }

    printf("Connecting\n");
    while (gba.GetPhase() < CGBASupport::EPhase::Complete)
    {
        s64 frameStart = GetGCTicks();
        gba.Update(1.f / 60.f);
        s64 frameEnd = GetGCTicks();
        s64 passedTicks = frameEnd - frameStart;
        s64 waitTicks = GetGCTicksPerSec() / 60 - passedTicks;
        if (waitTicks > 0)
            WaitGCTicks(waitTicks);
    }

    u8 status;
    GBAGetStatus(1, &status);

    CGBASupport::EPhase finalPhase = gba.GetPhase();
    printf("%s Linked: %d Beat: %d\n",
           finalPhase == CGBASupport::EPhase::Complete ? "Complete" : "Failed",
           gba.IsFusionLinked(), gba.IsFusionBeat());

    return 0;
}
