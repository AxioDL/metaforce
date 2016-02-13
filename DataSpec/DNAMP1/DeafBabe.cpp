#include "DeafBabe.hpp"

namespace DataSpec
{
namespace DNAMP1
{

void DeafBabe::BlenderInit(HECL::BlenderConnection::PyOutStream& os)
{
    os << "TYPE_COLORS = {'Ground':(1.0, 0.43, 0.15),\n"
          "               'Stone':(0.28, 0.28, 0.28),\n"
          "               'Hard Stone':(0.1, 0.1, 0.1),\n"
          "               'Metal':(0.5, 0.5, 0.5),\n"
          "               'Leaves':(0.61, 0.03, 0.05)}\n"
          "\n"
          "# Diffuse Color Maker\n"
          "def make_color(index, mat_type, name):\n"
          "    new_mat = bpy.data.materials.new(name)\n"
          "    if mat_type in TYPE_COLORS:\n"
          "        new_mat.diffuse_color = TYPE_COLORS[mat_type]\n"
          "    else:\n"
          "        new_mat.diffuse_color.hsv = ((index / 6.0) % 1.0, 1.0-((index // 6) / 6.0), 1)\n"
          "    return new_mat\n"
          "\n"
          "bpy.types.Material.retro_collision_type = bpy.props.IntProperty(name='Retro: Collsion Type')\n"
          "bpy.types.Material.retro_projectile_passthrough = bpy.props.BoolProperty(name='Retro: Projectile Passthrough')\n"
          "\n"
          "material_dict = {}\n"
          "material_index = []\n"
          "def select_material(data):\n"
          "\n"
          "    type_id = data & 0xff\n"
          "    mat_type = str(type_id)\n"
          "    if type_id == 1:\n"
          "        mat_type = 'Ground'\n"
          "    elif type_id == 3:\n"
          "        mat_type = 'Stone'\n"
          "    elif type_id == 4:\n"
          "        mat_type = 'Hard Stone'\n"
          "    elif type_id == 5:\n"
          "        mat_type = 'Metal'\n"
          "    elif type_id == 9:\n"
          "        mat_type = 'Leaves'\n"
          "\n"
          "    if ((data >> 18) & 1):\n"
          "        mat_name = mat_type + ' Fire Through'\n"
          "    else:\n"
          "        mat_name = mat_type\n"
          "\n"
          "    if mat_name in material_index:\n"
          "        return material_index.index(mat_name)\n"
          "    elif mat_name in material_dict:\n"
          "        material_index.append(mat_name)\n"
          "        return len(material_index)-1\n"
          "    else:\n"
          "        mat = make_color(len(material_dict), mat_type, mat_name)\n"
          "        mat.retro_collision_type = type_id\n"
          "        mat.retro_projectile_passthrough = ((data >> 18) & 1)\n"
          "        material_dict[mat_name] = mat\n"
          "        material_index.append(mat_name)\n"
          "        return len(material_index)-1\n"
          "\n"
          "\n";
}

}
}
