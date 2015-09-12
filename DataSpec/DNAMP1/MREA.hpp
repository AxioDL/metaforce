#ifndef __DNAMP1_MREA_HPP__
#define __DNAMP1_MREA_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "CMDLMaterials.hpp"
#include "CSKR.hpp"

namespace Retro
{
namespace DNAMP1
{

struct MREA
{
    struct Header : BigDNA
    {
        DECL_DNA
        Value<atUint32> magic;
        Value<atUint32> version;
        Value<atVec4f> localToWorldMtx[3];
        Value<atUint32> meshCount;
        Value<atUint32> secCount;
        Value<atUint32> geomSecIdx;
        Value<atUint32> sclySecIdx;
        Value<atUint32> collisionSecIdx;
        Value<atUint32> unkSecIdx;
        Value<atUint32> lightSecIdx;
        Value<atUint32> visiSecIdx;
        Value<atUint32> pathSecIdx;
        Value<atUint32> arotSecIdx;
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

    struct BabeDeadLight : BigDNA
    {
        DECL_DNA
        enum LightType : atUint32
        {
            LightLocalAmbient,
            LightDirectional,
            LightCustom,
            LightSpot
        };
        enum Falloff : atUint32
        {
            FalloffConstant,
            FalloffLinear,
            FalloffQuadratic
        };
        Value<LightType> lightType;
        Value<atVec3f> color;
        Value<atVec3f> position;
        Value<atVec3f> direction;
        Value<float> q;
        Value<float> spotCutoff;
        Value<float> unk5;
        Value<atUint8> unk6;
        Value<float> unk7;
        Value<Falloff> falloff;
        Value<float> unk9;
    };

    static void ReadBabeDeadToBlender_1_2(HECL::BlenderConnection::PyOutStream& os,
                                          Athena::io::IStreamReader& rs);

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool,
                        std::function<void(const HECL::SystemChar*)>);
};

}
}

#endif
