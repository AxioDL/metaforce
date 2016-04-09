#ifndef _DNACOMMON_BABEDEAD_HPP_
#define _DNACOMMON_BABEDEAD_HPP_

#include "BlenderConnection.hpp"
#include "zeus/Math.hpp"
#include <cfloat>

namespace DataSpec
{

template<class BabeDeadLight>
void ReadBabeDeadLightToBlender(hecl::BlenderConnection::PyOutStream& os,
                                const BabeDeadLight& light, unsigned s, unsigned l)
{
    switch (light.lightType)
    {
    case BabeDeadLight::LightType::LocalAmbient:
    case BabeDeadLight::LightType::LocalAmbient2:
        os.format("bg_node.inputs[0].default_value = (%f,%f,%f,1.0)\n"
                  "bg_node.inputs[1].default_value = %f\n",
                  light.color.vec[0], light.color.vec[1], light.color.vec[2],
                  light.q / 8.0);
        return;
    case BabeDeadLight::LightType::Directional:
        os.format("lamp = bpy.data.lamps.new('LAMP_%01u_%03u', 'SPOT')\n"
                  "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
                  "lamp_obj.rotation_mode = 'QUATERNION'\n"
                  "lamp_obj.rotation_quaternion = Vector((0,0,-1)).rotation_difference(Vector((%f,%f,%f)))\n"
                  "\n", s, l,
                  light.direction.vec[0], light.direction.vec[1], light.direction.vec[2]);
        break;
    case BabeDeadLight::LightType::Custom:
        os.format("lamp = bpy.data.lamps.new('LAMP_%01u_%03u', 'POINT')\n"
                  "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
                  "\n", s, l);
        break;
    case BabeDeadLight::LightType::Spot:
    case BabeDeadLight::LightType::Spot2:
        os.format("lamp = bpy.data.lamps.new('LAMP_%01u_%03u', 'SPOT')\n"
                  "lamp.spot_size = %.6g\n"
                  "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
                  "lamp_obj.rotation_mode = 'QUATERNION'\n"
                  "lamp_obj.rotation_quaternion = Vector((0,0,-1)).rotation_difference(Vector((%f,%f,%f)))\n"
                  "\n", s, l,
                  zeus::degToRad(light.spotCutoff),
                  light.direction.vec[0], light.direction.vec[1], light.direction.vec[2]);
        break;
    default: return;
    }

    os.format("lamp.retro_layer = %u\n"
              "lamp.retro_origtype = %u\n"
              "lamp.falloff_type = 'INVERSE_COEFFICIENTS'\n"
              "lamp.constant_coefficient = 0\n"
              "lamp.use_nodes = True\n"
              "falloff_node = lamp.node_tree.nodes.new('ShaderNodeLightFalloff')\n"
              "lamp.energy = 0.0\n"
              "falloff_node.inputs[0].default_value = %f\n"
              "hue_sat_node = lamp.node_tree.nodes.new('ShaderNodeHueSaturation')\n"
              "hue_sat_node.inputs[1].default_value = 1.25\n"
              "hue_sat_node.inputs[4].default_value = (%f,%f,%f,1.0)\n"
              "lamp.node_tree.links.new(hue_sat_node.outputs[0], lamp.node_tree.nodes['Emission'].inputs[0])\n"
              "lamp_obj.location = (%f,%f,%f)\n"
              "bpy.context.scene.objects.link(lamp_obj)\n"
              "\n", s, light.lightType, light.q / 8.0,
              light.color.vec[0], light.color.vec[1], light.color.vec[2],
              light.position.vec[0], light.position.vec[1], light.position.vec[2]);

    switch (light.falloff)
    {
    case BabeDeadLight::Falloff::Constant:
        os << "falloff_node.inputs[0].default_value *= 75.0\n"
              "lamp.node_tree.links.new(falloff_node.outputs[2], lamp.node_tree.nodes['Emission'].inputs[1])\n";
        if (light.q > FLT_EPSILON)
            os.format("lamp.constant_coefficient = 2.0 / %f\n", light.q);
        break;
    case BabeDeadLight::Falloff::Linear:
        os << "lamp.node_tree.links.new(falloff_node.outputs[1], lamp.node_tree.nodes['Emission'].inputs[1])\n";
        if (light.q > FLT_EPSILON)
            os.format("lamp.linear_coefficient = 250 / %f\n", light.q);
        break;
    case BabeDeadLight::Falloff::Quadratic:
        os << "lamp.node_tree.links.new(falloff_node.outputs[0], lamp.node_tree.nodes['Emission'].inputs[1])\n";
        if (light.q > FLT_EPSILON)
            os.format("lamp.quadratic_coefficient = 25000 / %f\n", light.q);
        break;
    default: break;
    }
}

}

#endif // _DNACOMMON_BABEDEAD_HPP_
