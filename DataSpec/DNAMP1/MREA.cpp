#include "MREA.hpp"
#include "DeafBabe.hpp"
#include "../DNACommon/BabeDead.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Retro
{
namespace DNAMP1
{

void MREA::ReadBabeDeadToBlender_1_2(HECL::BlenderConnection::PyOutStream& os,
                                     Athena::io::IStreamReader& rs)
{
    atUint32 bdMagic = rs.readUint32Big();
    if (bdMagic != 0xBABEDEAD)
        Log.report(LogVisor::FatalError, "invalid BABEDEAD magic");
    os << "bpy.context.scene.render.engine = 'CYCLES'\n"
          "bpy.context.scene.world.use_nodes = True\n"
          "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
          "bg_node = bpy.context.scene.world.node_tree.nodes['Background']\n";
    for (atUint32 s=0 ; s<2 ; ++s)
    {
        atUint32 lightCount = rs.readUint32Big();
        for (atUint32 l=0 ; l<lightCount ; ++l)
        {
            BabeDeadLight light;
            light.read(rs);
            ReadBabeDeadLightToBlender(os, light, s, l);
        }
    }
}

bool MREA::Extract(const SpecBase& dataSpec,
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
    if (!conn.createBlend(outPath.getAbsolutePath()))
        return false;

    /* Open Py Stream and read sections */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "from mathutils import Vector\n"
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
          "bpy.types.Lamp.retro_layer = bpy.props.IntProperty(name='Retro: Light Layer')\n"
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
    matSet.readToBlender(os, pakRouter, entry, 0);
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
        curSec += DNACMDL::ReadGeomSectionsToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_1_2>
                      (os, rs, pakRouter, entry, dummy, true,
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

    /* Skip AROT */
    rs.seek(head.secSizes[curSec++], Athena::Current);

    /* Skip SCLY (for now) */
    rs.seek(head.secSizes[curSec++], Athena::Current);

    /* Read collision meshes */
    DeafBabe collision;
    secStart = rs.position();
    collision.read(rs);
    DeafBabe::BlenderInit(os);
    collision.sendToBlender(os);
    rs.seek(secStart + head.secSizes[curSec++], Athena::Begin);

    /* Skip unknown section */
    rs.seek(head.secSizes[curSec++], Athena::Current);

    /* Read BABEDEAD Lights as Cycles emissives */
    secStart = rs.position();
    ReadBabeDeadToBlender_1_2(os, rs);
    rs.seek(secStart + head.secSizes[curSec++], Athena::Begin);

    /* Origins to center of mass */
    os << "bpy.context.scene.layers[1] = True\n"
          "bpy.ops.object.select_by_type(type='MESH')\n"
          "bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')\n"
          "bpy.ops.object.select_all(action='DESELECT')\n"
          "bpy.context.scene.layers[1] = False\n";

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

}
}
