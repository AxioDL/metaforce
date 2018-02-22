#ifndef __DNAMP1_MREA_HPP__
#define __DNAMP1_MREA_HPP__

#include "../DNACommon/DNACommon.hpp"
#include "CMDLMaterials.hpp"
#include "CSKR.hpp"

namespace DataSpec::DNAMP1
{

struct MREA
{
    struct Header : BigDNA
    {
        AT_DECL_DNA
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
            void setFromBlenderProps(const std::unordered_map<std::string, std::string>& props);
        } visorFlags;
        Value<atVec4f> xfMtx[3];
        Value<atVec3f> aabb[2];
    };

    struct BabeDeadLight : BigDNA
    {
        AT_DECL_DNA
        enum class LightType : atUint32
        {
            LocalAmbient,
            Directional,
            Custom,
            Spot,
            Spot2,
            LocalAmbient2
        };
        enum class Falloff : atUint32
        {
            Constant,
            Linear,
            Quadratic
        };
        Value<LightType> lightType;
        Value<atVec3f> color;
        Value<atVec3f> position;
        Value<atVec3f> direction;
        Value<float> q;
        Value<float> spotCutoff;
        Value<float> unk5;
        Value<bool> castShadows;
        Value<float> unk7;
        Value<Falloff> falloff;
        Value<float> unk9;
    };

    static void ReadBabeDeadToBlender_1_2(hecl::blender::PyOutStream& os,
                                          athena::io::IStreamReader& rs);

    static void AddCMDLRigPairs(PAKEntryReadStream& rs,
                                PAKRouter<PAKBridge>& pakRouter,
                                std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo);

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)>);

    static void Name(const SpecBase& dataSpec,
                     PAKEntryReadStream& rs,
                     PAKRouter<PAKBridge>& pakRouter,
                     PAK::Entry& entry);

    using ColMesh = hecl::blender::ColMesh;
    using Light = hecl::blender::Light;

    static bool Cook(const hecl::ProjectPath& outPath,
                     const hecl::ProjectPath& inPath,
                     const std::vector<DNACMDL::Mesh>& meshes,
                     const ColMesh& cMesh,
                     const std::vector<Light>& lights);

    static bool PCCook(const hecl::ProjectPath& outPath,
                       const hecl::ProjectPath& inPath,
                       const std::vector<DNACMDL::Mesh>& meshes,
                       const ColMesh& cMesh,
                       const std::vector<Light>& lights,
                       hecl::blender::Token& btok);

    static bool CookPath(const hecl::ProjectPath& outPath,
                         const hecl::ProjectPath& inPath);
};

}

#endif
