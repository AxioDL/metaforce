#ifndef __DNAMP2_MREA_HPP__
#define __DNAMP2_MREA_HPP__

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "CMDLMaterials.hpp"
#include "CSKR.hpp"

namespace DataSpec::DNAMP2
{

struct MREA
{
    class StreamReader : public athena::io::IStreamReader
    {
    protected:
        struct BlockInfo : BigDNA
        {
            AT_DECL_DNA
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
        athena::io::IStreamReader& m_source;
        atUint64 m_blkBase;
        atUint32 m_blkCount;
        atUint32 m_totalDecompLen = 0;
        atUint32 m_pos = 0;

        atUint32 m_nextBlk = 0;
        atUint32 m_posInBlk = 0;
        atUint32 m_blkSz = 0;
        void nextBlock();

        StreamReader(athena::io::IStreamReader& source)
        : m_compBufSz(0x4120), m_compBuf(new atUint8[0x4120]),
          m_decompBufSz(0x4120), m_decompBuf(new atUint8[0x4120]),
          m_source(source) {} /* Empty constructor for inheriting */

    public:
        StreamReader(athena::io::IStreamReader& source, atUint32 blkCount);
        void seek(atInt64 diff, athena::SeekOrigin whence);
        atUint64 position() const {return m_pos;}
        atUint64 length() const {return m_totalDecompLen;}
        atUint64 readUBytesToBuf(void* buf, atUint64 len);
        void writeDecompInfos(athena::io::IStreamWriter& writer) const;
    };

    struct Header : BigDNA
    {
        AT_DECL_DNA
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
        Seek<12, athena::Current> align1;
        Vector<atUint32, AT_DNA_COUNT(secCount)> secSizes;
    };

    struct MeshHeader : BigDNA
    {
        AT_DECL_DNA
        struct VisorFlags : BigDNA
        {
            AT_DECL_DNA
            Value<atUint32> flags;
            enum class ThermalLevel
            {
                Cool,
                Hot,
                Warm
            };
            static const char* GetThermalLevelStr(ThermalLevel t)
            {
                switch (t)
                {
                case ThermalLevel::Cool: return "COOL";
                case ThermalLevel::Hot: return "HOT";
                case ThermalLevel::Warm: return "WARM";
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
            void setThermalLevel(ThermalLevel v) {flags &= ~0x30; flags |= atUint32(v) << 4;}
            const char* thermalLevelStr() const {return GetThermalLevelStr(thermalLevel());}
        } visorFlags;
        Value<atVec4f> xfMtx[3];
        Value<atVec3f> aabb[2];
    };

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)>);
};

}

#endif
