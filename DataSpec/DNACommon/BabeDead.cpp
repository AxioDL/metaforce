#include "BabeDead.hpp"
#include "DataSpec/DNAMP1/MREA.hpp"
#include "DataSpec/DNAMP3/MREA.hpp"
#include "zeus/CTransform.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec {

template <class BabeDeadLight>
void ReadBabeDeadLightToBlender(hecl::blender::PyOutStream& os, const BabeDeadLight& light, unsigned s, unsigned l) {
  switch (light.lightType) {
  case BabeDeadLight::LightType::LocalAmbient:
  case BabeDeadLight::LightType::LocalAmbient2:
    os.format(
        "bg_node.inputs[0].default_value = (%f,%f,%f,1.0)\n"
        "bg_node.inputs[1].default_value = %f\n",
        light.color.simd[0], light.color.simd[1], light.color.simd[2], light.q / 8.f);
    return;
  case BabeDeadLight::LightType::Directional:
    os.format(
        "lamp = bpy.data.lights.new('LAMP_%01u_%03u', 'SUN')\n"
        "lamp.color = (%f,%f,%f)\n"
        "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
        "lamp_obj.rotation_mode = 'QUATERNION'\n"
        "lamp_obj.rotation_quaternion = Vector((0,0,-1)).rotation_difference(Vector((%f,%f,%f)))\n"
        "lamp.use_shadow = %s\n"
        "\n",
        s, l, light.color.simd[0], light.color.simd[1], light.color.simd[2], light.direction.simd[0],
        light.direction.simd[1], light.direction.simd[2], light.castShadows ? "True" : "False");
    return;
  case BabeDeadLight::LightType::Custom:
    os.format(
        "lamp = bpy.data.lights.new('LAMP_%01u_%03u', 'POINT')\n"
        "lamp.color = (%f,%f,%f)\n"
        "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
        "lamp.shadow_soft_size = 1.0\n"
        "lamp.use_shadow = %s\n"
        "\n",
        s, l, light.color.simd[0], light.color.simd[1], light.color.simd[2],
        light.castShadows ? "True" : "False");
    break;
  case BabeDeadLight::LightType::Spot:
  case BabeDeadLight::LightType::Spot2:
    os.format(
        "lamp = bpy.data.lights.new('LAMP_%01u_%03u', 'SPOT')\n"
        "lamp.color = (%f,%f,%f)\n"
        "lamp.spot_size = %.6g\n"
        "lamp_obj = bpy.data.objects.new(lamp.name, lamp)\n"
        "lamp_obj.rotation_mode = 'QUATERNION'\n"
        "lamp_obj.rotation_quaternion = Vector((0,0,-1)).rotation_difference(Vector((%f,%f,%f)))\n"
        "lamp.shadow_soft_size = 0.5\n"
        "lamp.use_shadow = %s\n"
        "\n",
        s, l, light.color.simd[0], light.color.simd[1], light.color.simd[2], zeus::degToRad(light.spotCutoff),
        light.direction.simd[0], light.direction.simd[1], light.direction.simd[2],
        light.castShadows ? "True" : "False");
    break;
  default:
    return;
  }

  os.format(
      "lamp.retro_layer = %u\n"
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
      "bpy.context.scene.collection.objects.link(lamp_obj)\n"
      "\n",
      s, light.lightType, light.q / 8.f, light.color.simd[0], light.color.simd[1], light.color.simd[2],
      light.position.simd[0], light.position.simd[1], light.position.simd[2]);

  switch (light.falloff) {
  case BabeDeadLight::Falloff::Constant:
    os << "falloff_node.inputs[0].default_value *= 150.0\n"
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
  default:
    break;
  }
}

template void ReadBabeDeadLightToBlender<DNAMP1::MREA::BabeDeadLight>(hecl::blender::PyOutStream& os,
                                                                      const DNAMP1::MREA::BabeDeadLight& light,
                                                                      unsigned s, unsigned l);
template void ReadBabeDeadLightToBlender<DNAMP3::MREA::BabeDeadLight>(hecl::blender::PyOutStream& os,
                                                                      const DNAMP3::MREA::BabeDeadLight& light,
                                                                      unsigned s, unsigned l);

template <class BabeDeadLight>
void WriteBabeDeadLightFromBlender(BabeDeadLight& lightOut, const hecl::blender::Light& lightIn) {
  using InterType = hecl::blender::Light::Type;
  switch (lightIn.type) {
  case InterType::Ambient:
    lightOut.lightType = BabeDeadLight::LightType::LocalAmbient;
    break;
  case InterType::Directional:
    lightOut.lightType = BabeDeadLight::LightType::Directional;
    break;
  case InterType::Custom:
  default:
    lightOut.lightType = BabeDeadLight::LightType::Custom;
    break;
  case InterType::Spot:
    lightOut.lightType = BabeDeadLight::LightType::Spot;
    break;
  }

  if (lightIn.type == InterType::Ambient) {
    lightOut.falloff = BabeDeadLight::Falloff::Constant;
    lightOut.q = lightIn.energy * 8.f;
  } else if (lightIn.linear > lightIn.constant && lightIn.linear > lightIn.quadratic) {
    lightOut.falloff = BabeDeadLight::Falloff::Linear;
    lightOut.q = 250.f / lightIn.linear;
  } else if (lightIn.quadratic > lightIn.constant && lightIn.quadratic > lightIn.linear) {
    lightOut.falloff = BabeDeadLight::Falloff::Quadratic;
    lightOut.q = 25000.f / lightIn.quadratic;
  } else {
    lightOut.falloff = BabeDeadLight::Falloff::Constant;
    lightOut.q = 2.f / lightIn.constant;
  }

  lightOut.color = lightIn.color;
  lightOut.spotCutoff = zeus::radToDeg(lightIn.spotCutoff);
  lightOut.castShadows = lightIn.shadow;
  lightOut.position.simd[0] = lightIn.sceneXf[0].simd[3];
  lightOut.position.simd[1] = lightIn.sceneXf[1].simd[3];
  lightOut.position.simd[2] = lightIn.sceneXf[2].simd[3];

  zeus::CTransform lightXf(&lightIn.sceneXf[0]);
  lightOut.direction = (lightXf.basis.transposed() * zeus::CVector3f(0.f, 0.f, -1.f)).normalized();
}

template void WriteBabeDeadLightFromBlender<DNAMP1::MREA::BabeDeadLight>(DNAMP1::MREA::BabeDeadLight& lightOut,
                                                                         const hecl::blender::Light& lightIn);
template void WriteBabeDeadLightFromBlender<DNAMP3::MREA::BabeDeadLight>(DNAMP3::MREA::BabeDeadLight& lightOut,
                                                                         const hecl::blender::Light& lightIn);

} // namespace DataSpec
