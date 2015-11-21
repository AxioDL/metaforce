#ifndef __RETRO_CDVDFILE_HPP__
#define __RETRO_CDVDFILE_HPP__

#include "RetroTypes.hpp"

namespace Retro
{

const char* DecodeARAMFile(const char* name);

enum class ESeekOrigin
{
    Begin = 0,
    Cur = 1,
    End = 2
};

struct DVDFileInfo;
class IDvdRequest;

class CDvdFile
{
    friend class CResLoader;
    std::string x18_name;
public:
    CDvdFile(const char*) {}
    void UpdateFilePos(int) {}
    void CalcFileOffset(int, ESeekOrigin) {}
    static void internalCallback(s32, DVDFileInfo*) {}
    static bool FileExists(const char*) {return false;}
    void CloseFile() {}
    IDvdRequest* AsyncSeekRead(void*, u32, ESeekOrigin, int) {return nullptr;}
    void SyncSeekRead(void*, u32, ESeekOrigin, int) {}
    IDvdRequest* AsyncRead(void*, u32) {return nullptr;}
    void SyncRead(void*, u32) {}
    void StallForARAMFile() {}
    void StartARAMFileLoad() {}
    void PopARAMFileLoad() {}
    void PushARAMFileLoad() {}
    void TryARAMFile() {}
    void PingARAMTransfer() {}
    void HandleDVDInterrupt() {}
    void HandleARAMInterrupt() {}
    static void ARAMARAMXferCallback(u32) {}
    static void DVDARAMXferCallback(s32, DVDFileInfo*) {}
};

}

#endif // __RETRO_CDVDFILE_HPP__
