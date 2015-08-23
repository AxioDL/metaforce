#ifndef __RETRO_CDVDFILE_HPP__
#define __RETRO_CDVDFILE_HPP__

#include "RetroTypes.hpp"

namespace Retro
{

const char* DecodeARAMFile(const char* name);

enum ESeekOrigin
{
    OriginBegin = 0,
    OriginCur = 1,
    OriginEnd = 2
};

struct DVDFileInfo;
class CDvdRequest;

class CDvdFile
{
public:
    CDvdFile(const char*);
    void UpdateFilePos(int);
    void CalcFileOffset(int, ESeekOrigin);
    static void internalCallback(s32, DVDFileInfo*);
    bool FileExists(const char*);
    void CloseFile();
    CDvdRequest* AsyncSeekRead(void*, u32, ESeekOrigin, int);
    void SyncSeekRead(void*, u32, ESeekOrigin, int);
    CDvdRequest* AsyncRead(void*, u32);
    void SyncRead(void*, u32);
    void StallForARAMFile();
    void StartARAMFileLoad();
    void PopARAMFileLoad();
    void PushARAMFileLoad();
    void TryARAMFile();
    void PingARAMTransfer();
    void HandleDVDInterrupt();
    void HandleARAMInterrupt();
    static void ARAMARAMXferCallback(u32);
    static void DVDARAMXferCallback(s32, DVDFileInfo*);
};

}

#endif // __RETRO_CDVDFILE_HPP__
