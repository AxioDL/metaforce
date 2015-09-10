#ifndef __DNAMP1_MREA_HPP__
#define __DNAMP1_MREA_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "CMDLMaterials.hpp"
#include "CSKR.hpp"

namespace Retro
{
namespace DNAMP2
{

struct MREA
{
    class StreamReader : public Athena::io::IStreamReader
    {
        struct BlockInfo : BigDNA
        {
            DECL_DNA
            Value<atUint32> bufSize;
            Value<atUint32> decompSize;
            Value<atUint32> compSize;
            Value<atUint32> secCount;
        };
        std::vector<BlockInfo> m_blockInfos;

        size_t m_compBufSz;
        std::unique_ptr<atUint8[]> m_compBuf;
        size_t m_decompBufSz;
        std::unique_ptr<atUint8[]> m_decompBuf;
        Athena::io::IStreamReader& m_source;
        atUint64 m_blkBase;
        atUint32 m_blkCount;
        atUint32 m_totalDecompLen = 0;
        atUint32 m_pos = 0;

        atUint32 m_nextBlk = 0;
        atUint32 m_posInBlk = 0;
        atUint32 m_blkSz = 0;
        void nextBlock();

    public:
        StreamReader(Athena::io::IStreamReader& source, atUint32 blkCount);
        void seek(atInt64 diff, Athena::SeekOrigin whence);
        atUint64 position() const {return m_pos;}
        atUint64 length() const {return m_totalDecompLen;}
        atUint64 readUBytesToBuf(void* buf, atUint64 len);
        void writeDecompInfos(Athena::io::IStreamWriter& writer) const;
    };

    struct Header : BigDNA
    {
        DECL_DNA
        Value<atUint32> magic;
        Value<atUint32> version;
        Value<atVec4f> localToWorldMtx[3];
        Value<atUint32> meshCount;
        Value<atUint32> sclyLayerCount;
        Value<atUint32> secCount;
        Value<atUint32> geomSecIdx;
        Value<atUint32> sclySecIdx;
        Value<atUint32> scgnSecIdx;
        Value<atUint32> collisionSecIdx;
        Value<atUint32> unkSecIdx;
        Value<atUint32> lightSecIdx;
        Value<atUint32> emptySecIdx;
        Value<atUint32> pathSecIdx;
        Value<atUint32> unk2SecIdx;
        Value<atUint32> unk3SecIdx;
        Value<atUint32> egmcSecIdx;
        Value<atUint32> compressedBlockCount;
        Seek<12, Athena::Current> align1;
        Vector<atUint32, DNA_COUNT(secCount)> secSizes;
    };

    struct MeshHeader : BigDNA
    {
        DECL_DNA
        struct VisorFlags : BigDNA
        {
            DECL_DNA
            Value<atUint32> flags;
            enum ThermalLevel
            {
                ThermalCool,
                ThermalHot,
                ThermalWarm
            };
            static const char* GetThermalLevelStr(ThermalLevel t)
            {
                switch (t)
                {
                case ThermalCool: return "COOL";
                case ThermalHot: return "HOT";
                case ThermalWarm: return "WARM";
                default: break;
                }
                return nullptr;
            }
            bool disableEnviro() const {return flags >> 1 & 0x1;}
            void setDisableEnviro(bool v) {flags &= ~0x2; flags |= v << 1;}
            bool disableThermal() const {return flags >> 2 & 0x1;}
            void setDisableThermal(bool v) {flags &= ~0x4; flags |= v << 2;}
            bool disableXray() const {return flags >> 3 & 0x1;}
            void setDisableXray(bool v) {flags &= ~0x8; flags |= v << 3;}
            ThermalLevel thermalLevel() const {return ThermalLevel(flags >> 4 & 0x3);}
            void setThermalLevel(ThermalLevel v) {flags &= ~0x30; flags |= v << 4;}
            const char* thermalLevelStr() const {return GetThermalLevelStr(thermalLevel());}
        } visorFlags;
        Value<atVec4f> xfMtx[3];
        Value<atVec3f> aabb[2];
    };

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool,
                        std::function<void(const HECL::SystemChar*)>);
};

}
}

#endif
