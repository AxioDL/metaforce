#include "CMDLMaterials.hpp"
#include "hecl/Blender/Connection.hpp"

using Stream = hecl::blender::PyOutStream;

namespace DataSpec::DNAMP3 {
using Material = MaterialSet::Material;

template <>
void MaterialSet::Material::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  header.read(reader);
  chunks.clear();
  do {
    chunks.emplace_back().read(reader);
  } while (!chunks.back().holds_alternative<END>());
  chunks.pop_back();
}
template <>
void MaterialSet::Material::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  header.write(writer);
  for (const auto& chunk : chunks)
    chunk.visit([&](auto& arg) { arg.write(writer); });
  DNAFourCC(FOURCC('END ')).write(writer);
}
template <>
void MaterialSet::Material::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  header.binarySize(s);
  for (const auto& chunk : chunks)
    chunk.visit([&](auto& arg) { arg.binarySize(s); });
  s += 4;
}

void MaterialSet::RegisterMaterialProps(Stream& out) {
  out << "bpy.types.Material.retro_enable_bloom = bpy.props.BoolProperty(name='Retro: Enable Bloom')\n"
         "bpy.types.Material.retro_force_lighting_stage = bpy.props.BoolProperty(name='Retro: Force Lighting Stage')\n"
         "bpy.types.Material.retro_pre_inca_transparency = bpy.props.BoolProperty(name='Retro: Pre-INCA "
         "Transparency')\n"
         "bpy.types.Material.retro_alpha_test = bpy.props.BoolProperty(name='Retro: Alpha Test')\n"
         "bpy.types.Material.retro_shadow_occluder = bpy.props.BoolProperty(name='Retro: Shadow Occluder')\n"
         "bpy.types.Material.retro_solid_white = bpy.props.BoolProperty(name='Retro: Solid White Only')\n"
         "bpy.types.Material.retro_reflection_alpha_target = bpy.props.BoolProperty(name='Retro: Reflection Alpha "
         "Target')\n"
         "bpy.types.Material.retro_solid_color = bpy.props.BoolProperty(name='Retro: Solid Color Only')\n"
         "bpy.types.Material.retro_exclude_scan = bpy.props.BoolProperty(name='Retro: Exclude From Scan Visor')\n"
         "bpy.types.Material.retro_xray_opaque = bpy.props.BoolProperty(name='Retro: XRay Opaque')\n"
         "bpy.types.Material.retro_xray_alpha_target = bpy.props.BoolProperty(name='Retro: XRay Alpha Target')\n"
         "bpy.types.Material.retro_inca_color_mod = bpy.props.BoolProperty(name='Retro: INCA Color Mod')\n"
         "\n";
}

static void LoadTexture(Stream& out, const UniqueID64& tex, const PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry) {
  if (!tex.isValid()) {
    out << "image = None\n";
    return;
  }
  std::string texName = pakRouter.getBestEntryName(tex);
  const nod::Node* node;
  const typename PAKRouter<PAKBridge>::EntryType* texEntry = pakRouter.lookupEntry(tex, &node);
  hecl::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
  if (!txtrPath.isNone()) {
    txtrPath.makeDirChain(false);
    PAKEntryReadStream rs = texEntry->beginReadStream(*node);
    TXTR::Extract(rs, txtrPath);
  }
  hecl::SystemString resPath = pakRouter.getResourceRelativePath(entry, tex);
  hecl::SystemUTF8Conv resPathView(resPath);
  out.format(FMT_STRING("if '{}' in bpy.data.images:\n"
                        "    image = bpy.data.images['{}']\n"
                        "else:\n"
                        "    image = bpy.data.images.load('''//{}''')\n"
                        "    image.name = '{}'\n"
                        "\n"),
             texName, texName, resPathView, texName);
}

void MaterialSet::ConstructMaterial(Stream& out, const PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry,
                                    const Material& material, unsigned groupIdx, unsigned matIdx) {
  out.format(FMT_STRING("new_material = bpy.data.materials.new('MAT_{}_{}')\n"), groupIdx, matIdx);
  out << "new_material.use_fake_user = True\n"
         "new_material.use_nodes = True\n"
         "new_material.use_backface_culling = True\n"
         "new_material.show_transparent_back = False\n"
         "new_material.blend_method = 'BLEND'\n"
         "new_nodetree = new_material.node_tree\n"
         "for n in new_nodetree.nodes:\n"
         "    new_nodetree.nodes.remove(n)\n"
         "\n"
         "gridder = hecl.Nodegrid(new_nodetree)\n"
         "new_nodetree.nodes.remove(gridder.frames[2])\n"
         "\n"
         "texture_nodes = []\n"
         "kcolors = {}\n"
         "kalphas = {}\n"
         "tex_links = []\n"
         "\n";

  /* Material Flags */
  out.format(FMT_STRING("new_material.retro_enable_bloom = {}\n"
                        "new_material.retro_force_lighting_stage = {}\n"
                        "new_material.retro_pre_inca_transparency = {}\n"
                        "new_material.retro_alpha_test = {}\n"
                        "new_material.retro_shadow_occluder = {}\n"
                        "new_material.retro_solid_white = {}\n"
                        "new_material.retro_reflection_alpha_target = {}\n"
                        "new_material.retro_solid_color = {}\n"
                        "new_material.retro_exclude_scan = {}\n"
                        "new_material.retro_xray_opaque = {}\n"
                        "new_material.retro_xray_alpha_target = {}\n"
                        "new_material.retro_inca_color_mod = False\n"),
             material.header.flags.enableBloom() ? "True" : "False",
             material.header.flags.forceLightingStage() ? "True" : "False",
             material.header.flags.preIncaTransparency() ? "True" : "False",
             material.header.flags.alphaTest() ? "True" : "False",
             material.header.flags.shadowOccluderMesh() ? "True" : "False",
             material.header.flags.justWhite() ? "True" : "False",
             material.header.flags.reflectionAlphaTarget() ? "True" : "False",
             material.header.flags.justSolidColor() ? "True" : "False",
             material.header.flags.excludeFromScanVisor() ? "True" : "False",
             material.header.flags.xrayOpaque() ? "True" : "False",
             material.header.flags.xrayAlphaTarget() ? "True" : "False");

  out << "pnode = new_nodetree.nodes.new('ShaderNodeGroup')\n"
         "pnode.name = 'Output'\n"
         "pnode.node_tree = bpy.data.node_groups['RetroShaderMP3']\n"
         "gridder.place_node(pnode, 1)\n";

  if (material.header.flags.additiveIncandecence())
    out << "pnode.inputs['Add INCA'].default_value = 1\n";

  int texMtxIdx = 0;
  for (const auto& chunk : material.chunks) {
    if (const Material::PASS* pass = chunk.get_if<Material::PASS>()) {
      LoadTexture(out, pass->txtrId, pakRouter, entry);
      out << "# Texture\n"
             "tex_node = new_nodetree.nodes.new('ShaderNodeTexImage')\n"
             "texture_nodes.append(tex_node)\n"
             "tex_node.image = image\n";

      if (!pass->uvAnim.empty()) {
        const auto& uva = pass->uvAnim[0];
        switch (uva.uvSource) {
        case Material::UVAnimationUVSource::Position:
        default:
          out << "tex_uv_node = new_nodetree.nodes.new('ShaderNodeTexCoord')\n"
                 "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['Window'], tex_node.inputs['Vector']))\n";
          break;
        case Material::UVAnimationUVSource::Normal:
          out << "tex_uv_node = new_nodetree.nodes.new('ShaderNodeTexCoord')\n"
                 "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['Normal'], tex_node.inputs['Vector']))\n";
          break;
        case Material::UVAnimationUVSource::UV:
          out.format(
              FMT_STRING(
                  "tex_uv_node = new_nodetree.nodes.new('ShaderNodeUVMap')\n"
                  "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['UV'], tex_node.inputs['Vector']))\n"
                  "tex_uv_node.uv_map = 'UV_{}'\n"),
              pass->uvSrc);
          break;
        }
        out.format(FMT_STRING("tex_uv_node.label = 'MTX_{}'\n"), texMtxIdx);
      } else {
        out.format(
            FMT_STRING(
                "tex_uv_node = new_nodetree.nodes.new('ShaderNodeUVMap')\n"
                "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['UV'], tex_node.inputs['Vector']))\n"
                "tex_uv_node.uv_map = 'UV_{}'\n"),
            pass->uvSrc);
      }

      out << "gridder.place_node(tex_uv_node, 0)\n"
             "gridder.place_node(tex_node, 0)\n"
             "tex_uv_node.location[0] -= 120\n"
             "tex_node.location[0] += 120\n"
             "tex_node.location[1] += 176\n"
             "\n";

      if (!pass->uvAnim.empty()) {
        const auto& uva = pass->uvAnim[0];
        DNAMP1::MaterialSet::Material::AddTextureAnim(out, uva.anim.mode, texMtxIdx++, uva.anim.vals);
      }

      auto DoSwap = [&]() {
        if (pass->flags.swapColorComponent() == Material::SwapColorComponent::Alpha) {
          out << "swap_output = tex_node.outputs['Alpha']\n";
        } else {
          out << "separate_node = new_nodetree.nodes.new('ShaderNodeSeparateRGB')\n"
                 "gridder.place_node(separate_node, 0, False)\n"
                 "separate_node.location[0] += 350\n"
                 "separate_node.location[1] += 350\n"
                 "new_nodetree.links.new(tex_node.outputs['Color'], separate_node.inputs[0])\n";
          out.format(FMT_STRING("swap_output = separate_node.outputs[{}]\n"), int(pass->flags.swapColorComponent()));
        }
      };

      using Subtype = Material::PASS::Subtype;
      switch (Subtype(pass->subtype.toUint32())) {
      case Subtype::DIFF:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['DIFFC'])\n"
               "new_nodetree.links.new(tex_node.outputs['Alpha'], pnode.inputs['DIFFA'])\n";
        break;
      case Subtype::BLOL:
        DoSwap();
        out << "new_nodetree.links.new(swap_output, pnode.inputs['BLOL'])\n";
        break;
      case Subtype::BLOD:
        DoSwap();
        out << "new_nodetree.links.new(swap_output, pnode.inputs['BLOD'])\n";
        break;
      case Subtype::CLR:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['CLR'])\n"
               "new_nodetree.links.new(tex_node.outputs['Alpha'], pnode.inputs['CLRA'])\n";
        break;
      case Subtype::TRAN:
        DoSwap();
        if (pass->flags.TRANInvert())
          out << "invert_node = new_nodetree.nodes.new('ShaderNodeInvert')\n"
                 "gridder.place_node(invert_node, 0, False)\n"
                 "invert_node.location[0] += 400\n"
                 "invert_node.location[1] += 350\n"
                 "new_nodetree.links.new(swap_output, invert_node.inputs['Color'])\n"
                 "swap_output = invert_node.outputs['Color']\n";
        out << "new_nodetree.links.new(swap_output, pnode.inputs['TRAN'])\n";
        break;
      case Subtype::INCA:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['INCAC'])\n";
        if (pass->flags.alphaContribution()) {
          DoSwap();
          out << "new_nodetree.links.new(swap_output, pnode.inputs['INCAA'])\n";
        }
        out.format(FMT_STRING("new_material.retro_inca_color_mod = {}\n"),
                   pass->flags.INCAColorMod() ? "True" : "False");
        break;
      case Subtype::RFLV:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['RFLV'])\n";
        break;
      case Subtype::RFLD:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['RFLD'])\n"
               "new_nodetree.links.new(tex_node.outputs['Alpha'], pnode.inputs['RFLDA'])\n";
        break;
      case Subtype::LRLD:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['LRLD'])\n";
        break;
      case Subtype::LURD:
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['LURDC'])\n"
               "new_nodetree.links.new(tex_node.outputs['Alpha'], pnode.inputs['LURDA'])\n";
        break;
      case Subtype::BLOI:
        DoSwap();
        out << "new_nodetree.links.new(swap_output, pnode.inputs['BLOI'])\n";
        break;
      case Subtype::XRAY:
        DoSwap();
        out << "new_nodetree.links.new(tex_node.outputs['Color'], pnode.inputs['XRAYC'])\n"
               "new_nodetree.links.new(swap_output, pnode.inputs['XRAYA'])\n";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("Unknown PASS subtype"));
        break;
      }
    } else if (const Material::CLR* clr = chunk.get_if<Material::CLR>()) {
      using Subtype = Material::CLR::Subtype;
      athena::simd_floats vec4;
      clr->color.toVec4f().simd.copy_to(vec4);
      switch (Subtype(clr->subtype.toUint32())) {
      case Subtype::CLR:
        out.format(FMT_STRING("pnode.inputs['CLR'].default_value = ({}, {}, {}, 1.0)\n"
                              "pnode.inputs['CLRA'].default_value = {}\n"),
                   vec4[0], vec4[1], vec4[2], vec4[3]);
        break;
      case Subtype::DIFB:
        out.format(FMT_STRING("pnode.inputs['DIFBC'].default_value = ({}, {}, {}, 1.0)\n"
                              "pnode.inputs['DIFBA'].default_value = {}\n"),
                   vec4[0], vec4[1], vec4[2], vec4[3]);
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("Unknown CLR subtype"));
        break;
      }
    } else if (const Material::INT* val = chunk.get_if<Material::INT>()) {
      using Subtype = Material::INT::Subtype;
      switch (Subtype(val->subtype.toUint32())) {
      case Subtype::OPAC:
        out.format(FMT_STRING("pnode.inputs['OPAC'].default_value = {}\n"), val->value / 255.f);
        break;
      case Subtype::BLOD:
        out.format(FMT_STRING("pnode.inputs['BLOD'].default_value = {}\n"), val->value / 255.f);
        break;
      case Subtype::BLOI:
        out.format(FMT_STRING("pnode.inputs['BLOI'].default_value = {}\n"), val->value / 255.f);
        break;
      case Subtype::BNIF:
        out.format(FMT_STRING("pnode.inputs['BNIF'].default_value = {}\n"), val->value / 255.f);
        break;
      case Subtype::XRBR:
        out.format(FMT_STRING("pnode.inputs['XRBR'].default_value = {}\n"), val->value / 255.f);
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("Unknown INT subtype"));
        break;
      }
    }
  }
}

} // namespace DataSpec::DNAMP3

AT_SPECIALIZE_TYPED_VARIANT_BIGDNA(DataSpec::DNAMP3::MaterialSet::Material::PASS,
                                   DataSpec::DNAMP3::MaterialSet::Material::CLR,
                                   DataSpec::DNAMP3::MaterialSet::Material::INT,
                                   DataSpec::DNAMP3::MaterialSet::Material::END)
