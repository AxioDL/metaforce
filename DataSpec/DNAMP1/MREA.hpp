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

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool,
                        std::function<void(const HECL::SystemChar*)>)
    {
        using RigPair = std::pair<CSKR*, CINF*>;
        RigPair dummy(nullptr, nullptr);

        /* Do extract */
        Header head;
        head.read(rs);
        rs.seekAlign32();

        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.createBlend(outPath.getWithExtension(_S(".blend")).getAbsolutePath()))
            return false;

        /* Open Py Stream and read sections */
        HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
        os.format("import bpy\n"
                  "import bmesh\n"
                  "\n"
                  "bpy.context.scene.name = '%s'\n"
                  "bpy.context.scene.hecl_type = 'AREA'\n",
                  pakRouter.getBestEntryName(entry).c_str());
        DNACMDL::InitGeomBlenderContext(os, dataSpec.getMasterShaderPath());
        MaterialSet::RegisterMaterialProps(os);
        os << "# Clear Scene\n"
              "for ob in bpy.data.objects:\n"
              "    bpy.context.scene.objects.unlink(ob)\n"
              "    bpy.data.objects.remove(ob)\n"
              "bpy.types.Object.retro_disable_enviro_visor = bpy.props.BoolProperty(name='Retro: Disable in Combat/Scan Visor')\n"
              "bpy.types.Object.retro_disable_thermal_visor = bpy.props.BoolProperty(name='Retro: Disable in Thermal Visor')\n"
              "bpy.types.Object.retro_disable_xray_visor = bpy.props.BoolProperty(name='Retro: Disable in X-Ray Visor')\n"
              "bpy.types.Object.retro_thermal_level = bpy.props.EnumProperty(items=[('COOL', 'Cool', 'Cool Temperature'),"
                                                                                   "('HOT', 'Hot', 'Hot Temperature'),"
                                                                                   "('WARM', 'Warm', 'Warm Temperature')],"
                                                                            "name='Retro: Thermal Visor Level')\n"
              "\n";

        /* One shared material set for all meshes */
        os << "# Materials\n"
              "materials = []\n"
              "\n";
        MaterialSet matSet;
        atUint64 secStart = rs.position();
        matSet.read(rs);
        matSet.readToBlender(os, pakRouter, entry, 0, dataSpec);
        rs.seek(secStart + head.secSizes[0], Athena::Begin);
        std::vector<DNACMDL::VertexAttributes> vertAttribs;
        DNACMDL::GetVertexAttributes(matSet, vertAttribs);

        /* Read meshes */
        atUint32 curSec = 1;
        for (int m=0 ; m<head.meshCount ; ++m)
        {
            MeshHeader mHeader;
            secStart = rs.position();
            mHeader.read(rs);
            rs.seek(secStart + head.secSizes[curSec++], Athena::Begin);
            curSec += DNACMDL::ReadGeomSectionsToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair>
                          (os, rs, pakRouter, entry, dataSpec, dummy, true,
                           true, vertAttribs, m, head.secCount, 0, &head.secSizes[curSec]);
            os.format("obj.retro_disable_enviro_visor = %s\n"
                      "obj.retro_disable_thermal_visor = %s\n"
                      "obj.retro_disable_xray_visor = %s\n"
                      "obj.retro_thermal_level = '%s'\n",
                      mHeader.visorFlags.disableEnviro() ? "True" : "False",
                      mHeader.visorFlags.disableThermal() ? "True" : "False",
                      mHeader.visorFlags.disableXray() ? "True" : "False",
                      mHeader.visorFlags.thermalLevelStr());
        }

        /* Center view */
        os << "bpy.context.user_preferences.view.smooth_view = 0\n"
              "for window in bpy.context.window_manager.windows:\n"
              "    screen = window.screen\n"
              "    for area in screen.areas:\n"
              "        if area.type == 'VIEW_3D':\n"
              "            for region in area.regions:\n"
              "                if region.type == 'WINDOW':\n"
              "                    override = {'scene': bpy.context.scene, 'window': window, 'screen': screen, 'area': area, 'region': region}\n"
              "                    bpy.ops.view3d.view_all(override)\n"
              "                    break\n";

        os.close();
        return conn.saveBlend();
    }
};

}
}

#endif
