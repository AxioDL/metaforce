#include "CMDLMaterials.hpp"
#include "../DNAMP2/CMDLMaterials.hpp"
#include "hecl/Blender/Connection.hpp"

using Stream = hecl::blender::PyOutStream;

namespace DataSpec::DNAMP1 {
using Material = MaterialSet::Material;

void MaterialSet::RegisterMaterialProps(Stream& out) {
  out << "bpy.types.Material.retro_depth_sort = bpy.props.BoolProperty(name='Retro: Transparent Depth Sort')\n"
         "bpy.types.Material.retro_alpha_test = bpy.props.BoolProperty(name='Retro: Punchthrough Alpha')\n"
         "bpy.types.Material.retro_samus_reflection = bpy.props.BoolProperty(name='Retro: Samus Reflection')\n"
         "bpy.types.Material.retro_depth_write = bpy.props.BoolProperty(name='Retro: Depth Write')\n"
         "bpy.types.Material.retro_samus_reflection_persp = bpy.props.BoolProperty(name='Retro: Samus Reflection "
         "Perspective')\n"
         "bpy.types.Material.retro_shadow_occluder = bpy.props.BoolProperty(name='Retro: Shadow Occluder')\n"
         "bpy.types.Material.retro_samus_reflection_indirect = bpy.props.BoolProperty(name='Retro: Samus Reflection "
         "Indirect Texture')\n"
         "bpy.types.Material.retro_lightmapped = bpy.props.BoolProperty(name='Retro: Lightmapped')\n"
         "\n";
}

void Material::AddTexture(Stream& out, GX::TexGenSrc type, int mtxIdx, uint32_t texIdx, bool diffuse) {
  char mtxLabel[64];
  if (mtxIdx == -1)
    strncpy(mtxLabel, "IDENTITY", 64);
  else
    snprintf(mtxLabel, 64, "MTX_%u", mtxIdx);

  char texLabel[64];
  if (diffuse)
    strncpy(texLabel, "Diffuse", 64);
  else
    strncpy(texLabel, "Texture", 64);

  out.format(
      "# Texture\n"
      "tex_node = new_nodetree.nodes.new('ShaderNodeTexImage')\n"
      "tex_node.label = '%s %u'\n"
      "texture_nodes.append(tex_node)\n",
      texLabel, texIdx);

  if (texIdx != 0xff)
    out.format("tex_node.image = tex_maps[%u]\n", texIdx);

  if (type == GX::TG_POS)
    out << "tex_uv_node = new_nodetree.nodes.new('ShaderNodeTexCoord')\n"
           "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['Window'], tex_node.inputs['Vector']))\n";
  else if (type == GX::TG_NRM)
    out << "tex_uv_node = new_nodetree.nodes.new('ShaderNodeTexCoord')\n"
           "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['Normal'], tex_node.inputs['Vector']))\n";
  else if (type >= GX::TG_TEX0 && type <= GX::TG_TEX7) {
    uint8_t texIdx = type - GX::TG_TEX0;
    out.format(
        "tex_uv_node = new_nodetree.nodes.new('ShaderNodeUVMap')\n"
        "tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['UV'], tex_node.inputs['Vector']))\n"
        "tex_uv_node.uv_map = 'UV_%u'\n",
        texIdx);
  }

  out.format("tex_uv_node.label = '%s'\n", mtxLabel);

  out << "gridder.place_node(tex_uv_node, 0)\n"
         "gridder.place_node(tex_node, 0)\n"
         "tex_uv_node.location[0] -= 120\n"
         "tex_node.location[0] += 120\n"
         "tex_node.location[1] += 176\n"
         "\n";
}

void Material::AddTextureAnim(Stream& out, UVAnimation::Mode type, unsigned idx, const float* vals) {
  switch (type) {
  case UVAnimation::Mode::MvInvNoTranslation:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode0NodeN']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx);
    break;
  case UVAnimation::Mode::MvInv:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode1NodeN']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx);
    break;
  case UVAnimation::Mode::Scroll:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode2Node']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        node.inputs[1].default_value = (%f,%f,0)\n"
        "        node.inputs[2].default_value = (%f,%f,0)\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx, vals[0], vals[1], vals[2], vals[3]);
    break;
  case UVAnimation::Mode::Rotation:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode3Node']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        node.inputs[1].default_value = %f\n"
        "        node.inputs[2].default_value = %f\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx, vals[0], vals[1]);
    break;
  case UVAnimation::Mode::HStrip:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode4Node']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        node.inputs[1].default_value = %f\n"
        "        node.inputs[2].default_value = %f\n"
        "        node.inputs[3].default_value = %f\n"
        "        node.inputs[4].default_value = %f\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx, vals[0], vals[1], vals[2], vals[3]);
    break;
  case UVAnimation::Mode::VStrip:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode5Node']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        node.inputs[1].default_value = %f\n"
        "        node.inputs[2].default_value = %f\n"
        "        node.inputs[3].default_value = %f\n"
        "        node.inputs[4].default_value = %f\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx, vals[0], vals[1], vals[2], vals[3]);
    break;
  case UVAnimation::Mode::Model:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode6NodeN']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx);
    break;
  case UVAnimation::Mode::CylinderEnvironment:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode7NodeN']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        node.inputs[1].default_value = %f\n"
        "        node.inputs[2].default_value = %f\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx, vals[0], vals[1]);
    break;
  case UVAnimation::Mode::Eight:
    out.format(
        "for link in list(tex_links):\n"
        "    if link.from_node.label == 'MTX_%u':\n"
        "        tex_links.remove(link)\n"
        "        soc_from = link.from_socket\n"
        "        soc_to = link.to_socket\n"
        "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
        "        node.node_tree = bpy.data.node_groups['RetroUVMode8Node']\n"
        "        node.location[0] = link.from_node.location[0] + 50\n"
        "        node.location[1] = link.from_node.location[1] - 50\n"
        "        node.inputs[1].default_value = %f\n"
        "        node.inputs[2].default_value = %f\n"
        "        node.inputs[3].default_value = %f\n"
        "        node.inputs[4].default_value = %f\n"
        "        node.inputs[5].default_value = %f\n"
        "        node.inputs[6].default_value = %f\n"
        "        node.inputs[7].default_value = %f\n"
        "        node.inputs[8].default_value = %f\n"
        "        node.inputs[9].default_value = %f\n"
        "        new_nodetree.links.remove(link)\n"
        "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
        "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
        idx, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8]);
    break;
  default:
    break;
  }
}

void Material::AddKcolor(Stream& out, const GX::Color& col, unsigned idx) {
  out.format(
      "kcolors[%d] = (%f, %f, %f, %f)\n"
      "kalphas[%d] = %f\n"
      "\n",
      idx, (float)col.color[0] / (float)0xff, (float)col.color[1] / (float)0xff, (float)col.color[2] / (float)0xff,
      (float)col.color[3] / (float)0xff, idx, (float)col.color[3] / (float)0xff);
}

template <class MAT>
static uint32_t _HashTextureConfig(const MAT& mat) {
  XXH32_state_t xxHash;
  XXH32_reset(&xxHash, 0);
  for (int i = 0; i < mat.tevStageCount; ++i) {
    const auto& stage = mat.tevStages[i];
    XXH32_update(&xxHash, &stage.ciFlags, sizeof(stage.ciFlags));
    XXH32_update(&xxHash, &stage.aiFlags, sizeof(stage.aiFlags));
    XXH32_update(&xxHash, &stage.ccFlags, sizeof(stage.ccFlags));
    XXH32_update(&xxHash, &stage.acFlags, sizeof(stage.acFlags));
    XXH32_update(&xxHash, &stage.kaInput, sizeof(stage.kaInput));
    XXH32_update(&xxHash, &stage.kcInput, sizeof(stage.kcInput));
    XXH32_update(&xxHash, &stage.rascInput, sizeof(stage.rascInput));
  }
  bool hasInd = mat.flags.samusReflectionIndirectTexture();
  XXH32_update(&xxHash, &hasInd, sizeof(hasInd));
  bool hasLm = mat.flags.lightmap();
  XXH32_update(&xxHash, &hasLm, sizeof(hasLm));
  return XXH32_digest(&xxHash);
}

static const char* ToString(GX::TevColorArg arg) {
  switch (arg) {
  case GX::CC_CPREV:
    return "CC_CPREV";
  case GX::CC_APREV:
    return "CC_APREV";
  case GX::CC_C0:
    return "CC_C0";
  case GX::CC_A0:
    return "CC_A0";
  case GX::CC_C1:
    return "CC_C1";
  case GX::CC_A1:
    return "CC_A1";
  case GX::CC_C2:
    return "CC_C2";
  case GX::CC_A2:
    return "CC_A2";
  case GX::CC_TEXC:
    return "CC_TEXC";
  case GX::CC_TEXA:
    return "CC_TEXA";
  case GX::CC_RASC:
    return "CC_RASC";
  case GX::CC_RASA:
    return "CC_RASA";
  case GX::CC_ONE:
    return "CC_ONE";
  case GX::CC_HALF:
    return "CC_HALF";
  case GX::CC_KONST:
    return "CC_KONST";
  case GX::CC_ZERO:
    return "CC_ZERO";
  default:
    return "UNKNOWN";
  }
}

static const char* ToString(GX::TevAlphaArg arg) {
  switch (arg) {
  case GX::CA_APREV:
    return "CA_APREV";
  case GX::CA_A0:
    return "CA_A0";
  case GX::CA_A1:
    return "CA_A1";
  case GX::CA_A2:
    return "CA_A2";
  case GX::CA_TEXA:
    return "CA_TEXA";
  case GX::CA_RASA:
    return "CA_RASA";
  case GX::CA_KONST:
    return "CA_KONST";
  case GX::CA_ZERO:
    return "CA_ZERO";
  default:
    return "UNKNOWN";
  }
}

static const char* ToString(GX::TevRegID arg) {
  switch (arg) {
  case GX::TEVPREV:
    return "TEVPREV";
  case GX::TEVREG0:
    return "TEVREG0";
  case GX::TEVREG1:
    return "TEVREG1";
  case GX::TEVREG2:
    return "TEVREG2";
  default:
    return "UNKNOWN";
  }
}

template <class MAT>
static void _DescribeTEV(const MAT& mat) {
  for (int i = 0; i < mat.tevStageCount; ++i) {
    const auto& stage = mat.tevStages[i];
    fprintf(stderr, "A:%s B:%s C:%s D:%s -> %s | A:%s B:%s C:%s D:%s -> %s\n",
            ToString(stage.colorInA()), ToString(stage.colorInB()),
            ToString(stage.colorInC()), ToString(stage.colorInD()), ToString(stage.colorOpOutReg()),
            ToString(stage.alphaInA()), ToString(stage.alphaInB()),
            ToString(stage.alphaInC()), ToString(stage.alphaInD()), ToString(stage.alphaOpOutReg()));
  }
  bool hasInd = mat.flags.samusReflectionIndirectTexture();
  bool hasLm = mat.flags.lightmap();
  fprintf(stderr, "HasIndirect: %d HasLightmap: %d\n", hasInd, hasLm);
}

struct TexLink {
  const char* shaderInput;
  int texidx;
  bool alpha;
  TexLink(const char* shaderInput, int texidx = -1, bool alpha = false)
  : shaderInput(shaderInput), texidx(texidx), alpha(alpha) {}
};

struct ExtendedSpecularLink {
  int texidx;
  ExtendedSpecularLink(int texidx = -1) : texidx(texidx) {}
};

struct KColLink {
  const char* shaderInput;
  int kcidx;
  bool alpha;
  KColLink(const char* shaderInput, int kcidx = 0, bool alpha = false)
  : shaderInput(shaderInput), kcidx(kcidx), alpha(alpha) {}
};

struct WhiteColorLink {
  const char* shaderInput;
  explicit WhiteColorLink(const char* shaderInput)
  : shaderInput(shaderInput) {}
};

static void _GenerateRootShader(Stream& out, int) {
  /* End of shader links */
}

template <typename... Targs>
static void _GenerateRootShader(Stream& out, int tidx, TexLink tex, Targs... args) {
  int texIdx = tex.texidx == -1 ? tidx : tex.texidx;
  out << "texture_nodes[" << texIdx << "].name = '" << tex.shaderInput << "'\n";
  out << "texture_nodes[" << texIdx << "].label = '" << tex.shaderInput << "'\n";
  out << "new_nodetree.links.new(texture_nodes[" << texIdx << "].outputs['" <<
    (tex.alpha ? "Alpha" : "Color") << "'], node.inputs['" << tex.shaderInput << "'])\n";
  if (tex.texidx == -1)
    ++tidx;
  _GenerateRootShader(out, tidx, args...);
}

template <typename... Targs>
static void _GenerateRootShader(Stream& out, int tidx, ExtendedSpecularLink tex, Targs... args) {
  int texIdx = tex.texidx == -1 ? tidx : tex.texidx;
  out << "texture_nodes[" << texIdx << "].name = 'Specular'\n";
  out << "texture_nodes[" << texIdx << "].label = 'Specular'\n";
  out << "new_nodetree.links.new(texture_nodes[" << texIdx << "].outputs['Color'], node.inputs['Specular'])\n";
  out << "new_nodetree.links.new(texture_nodes[" << texIdx << "].outputs['Alpha'], node.inputs['ExtendedSpecular'])\n";
  if (tex.texidx == -1)
    ++tidx;
  _GenerateRootShader(out, tidx, args...);
}

template <typename... Targs>
static void _GenerateRootShader(Stream& out, int tidx, KColLink kcol, Targs... args) {
  out << "node.inputs['" << kcol.shaderInput << "'].default_value = " <<
    (kcol.alpha ? "kalphas[" : "kcolors[") << kcol.kcidx << "]\n";
  _GenerateRootShader(out, tidx, args...);
}

template <typename... Targs>
static void _GenerateRootShader(Stream& out, int tidx, WhiteColorLink wcol, Targs... args) {
  out << "node.inputs['" << wcol.shaderInput << "'].default_value = (1.0, 1.0, 1.0, 1.0)\n";
  _GenerateRootShader(out, tidx, args...);
}

template <typename... Targs>
static void _GenerateRootShader(Stream& out, const char* type, Targs... args) {
  out << "node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
         "node.name = 'Output'\n"
         "node.node_tree = bpy.data.node_groups['" << type << "']\n"
         "gridder.place_node(node, 1)\n";
  _GenerateRootShader(out, 0, args...);
}

static TexLink operator "" _tex(const char* str, size_t) { return TexLink(str); }
static TexLink operator "" _texa(const char* str, size_t) { return TexLink(str, -1, true); }
static KColLink operator "" _kcol(const char* str, size_t) { return KColLink(str); }
static KColLink operator "" _kcola(const char* str, size_t) { return KColLink(str, 0, true); }

template <class MAT>
static void _ConstructMaterial(Stream& out, const MAT& material, unsigned groupIdx, unsigned matIdx) {
  unsigned i;

  out.format("new_material = bpy.data.materials.new('MAT_%u_%u')\n", groupIdx, matIdx);
  out << "new_material.use_fake_user = True\n"
         "new_material.use_nodes = True\n"
         "new_nodetree = new_material.node_tree\n"
         "for n in new_nodetree.nodes:\n"
         "    new_nodetree.nodes.remove(n)\n"
         "\n"
         "gridder = hecl.Nodegrid(new_nodetree)\n"
         "\n"
         "texture_nodes = []\n"
         "kcolors = {}\n"
         "kalphas = {}\n"
         "tex_links = []\n"
         "\n";

  /* Material Flags */
  out.format(
      "new_material.retro_depth_sort = %s\n"
      "new_material.retro_alpha_test = %s\n"
      "new_material.retro_samus_reflection = %s\n"
      "new_material.retro_depth_write = %s\n"
      "new_material.retro_samus_reflection_persp = %s\n"
      "new_material.retro_shadow_occluder = %s\n"
      "new_material.retro_samus_reflection_indirect = %s\n"
      "new_material.retro_lightmapped = %s\n"
      "new_material.diffuse_color = (1, 1, 1, %s)\n",
      material.flags.depthSorting() ? "True" : "False", material.flags.alphaTest() ? "True" : "False",
      material.flags.samusReflection() ? "True" : "False", material.flags.depthWrite() ? "True" : "False",
      material.flags.samusReflectionSurfaceEye() ? "True" : "False",
      material.flags.shadowOccluderMesh() ? "True" : "False",
      material.flags.samusReflectionIndirectTexture() ? "True" : "False", material.flags.lightmap() ? "True" : "False",
      material.flags.shadowOccluderMesh() ? "0" : "1");

  /* Texture Indices */
  out << "tex_maps = []\n";
  for (atUint32 idx : material.textureIdxs)
    out.format("tex_maps.append(texmap_list[%u])\n", idx);

  /* KColor entries */
  if (material.flags.konstValuesEnabled()) {
    unsigned i = 0;
    for (const GX::Color& col : material.konstColors)
      Material::AddKcolor(out, col, i++);
  }

  /* Blend factors */
  using BlendFactor = Material::BlendFactor;
  if (material.blendDstFac != BlendFactor::BL_ZERO) {
    if (material.blendDstFac == BlendFactor::BL_ONE)
      out << "new_material.blend_method = 'ADD'\n";
    else
      out << "new_material.blend_method = 'BLEND'\n";
  }

  /* Add texture maps/tcgs */
  unsigned addedTcgs = 0;
  bool diffuseStage = false;
  for (i = 0; i < material.tevStageCount; ++i) {
    if (material.tevStageTexInfo[i].tcgSlot != 0xff && !(addedTcgs >> material.tevStageTexInfo[i].tcgSlot & 1)) {
      const Material::TexCoordGen& tcg = material.tcgs[material.tevStageTexInfo[i].tcgSlot];
      GX::TexMtx mtx = tcg.mtx();
      int mtxIdx = -1;
      if (mtx >= GX::TEXMTX0 && mtx <= GX::TEXMTX9)
        mtxIdx = (mtx - GX::TEXMTX0) / 3;
      Material::AddTexture(out, tcg.source(), mtxIdx, material.tevStageTexInfo[i].texSlot, diffuseStage);
      addedTcgs |= 1 << material.tevStageTexInfo[i].tcgSlot;
    }
    diffuseStage = material.tevStages[i].colorOpOutReg() == GX::TEVREG0;
  }

  /* Indirect texture node */
  if (material.flags.samusReflectionIndirectTexture())
    Material::AddTexture(out, GX::TexGenSrc::TG_POS, -1, material.indTexSlot[0], false);

  /* Select appropriate root shader and link textures */
  uint32_t hash = _HashTextureConfig(material);
  switch (hash) {
  case 0x0473AE40: /* RetroShader: Lightmap, Diffuse, Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Emissive"_tex); break;
  case 0x072D2CB3: /* RetroShader: Diffuse, Emissive, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, WhiteColorLink("Specular"), "Reflection"_tex); break;
  case 0x0879D346: /* RetroShader: KColorDiffuse, Alpha=Texture */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_kcol, "Alpha"_tex); break;
  case 0x0DA256BB: /* Lightmap, Diffuse, Specular, Reflection, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "Alpha"_kcola); break;
  case 0x11C41DA4: /* RetroDynamicCharacterShader: Diffuse, DynamicMaskTex, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroDynamicCharacterShader", "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0x1218F83E: /* RetroShader: ObjLightmap, Diffuse, ExtendedSpecular, Reflection, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, ExtendedSpecularLink(), "Reflection"_tex, TexLink("Alpha", 1, true)); break;
  case 0x129B8578: /* RetroShader: KColorDiffuse, Emissive, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_kcol, "Emissive"_tex, "Alpha"_kcola); break;
  case 0x15A3E6E5: /* RetroShader: Diffuse, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Alpha"_kcola); break;
  case 0x1BEB3E15: /* RetroShader: Diffuse, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, TexLink("Alpha", 0, true)); break;
  case 0x2261E0EB: /* RetroShader: Diffuse, Emissive, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0x239C7724: /* RetroDynamicShader: Diffuse*Dynamic, Emissive*Dynamic, Alpha=1.0 */
    _GenerateRootShader(out, "RetroDynamicShader", "Diffuse"_tex, "Emissive"_tex); break;
  case 0x240C4C84: /* RetroShader: Lightmap, KColorDiffuse, Specular, Reflection, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_kcol, "Specular"_tex, "Reflection"_tex, "Alpha"_kcola); break;
  case 0x2523A379: /* RetroDynamicShader: Emissive*Dynamic, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroDynamicShader", "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0x25E85017: /* RetroShader: Lightmap, KColorDiffuse, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_kcol, "Alpha"_kcola); break;
  case 0x27FD5C6C: /* RetroShader: ObjLightmap, Diffuse, Specular, Reflection, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, TexLink("Alpha", 1, true)); break;
  case 0x2AD9F535: /* RetroShader: Emissive, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Emissive"_tex, WhiteColorLink("Specular"), "Reflection"_tex); break;
  case 0x2C9F5104: /* RetroShader: Diffuse, Specular, Reflection, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "Alpha"_kcola); break;
  case 0x2D059429: /* RetroShader: Diffuse, Emissive, ExtendedSpecular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, ExtendedSpecularLink(), "Reflection"_tex); break;
  case 0x30AC64BB: /* RetroShader: Diffuse, Specular, Reflection, Alpha=KAlpha, IndirectTex */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "IndirectTex"_tex, "Alpha"_kcola); break;
  case 0x39BC4809: /* RetroDynamicShader: ObjLightmap*Dynamic, Diffuse*Dynamic, Emissive*Dynamic, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroDynamicShader", "Lightmap"_tex, "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0x3BF97299: /* RetroShader: Lightmap, Diffuse, Specular, Reflection, Alpha=KAlpha, IndirectTex */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "IndirectTex"_tex, "Alpha"_kcola); break;
  case 0x47ECF3ED: /* RetroShader: Diffuse, Specular, Reflection, Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "Emissive"_tex); break;
  case 0x4BBDFFA6: /* RetroShader: Diffuse, Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex); break;
  case 0x4D4127A3: /* RetroShader: Lightmap, Diffuse, Specular, Reflection, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, TexLink("Alpha", 1, true)); break;
  case 0x54A92F25: /* RetroShader: ObjLightmap, KColorDiffuse, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_kcol, "Alpha"_kcola); break;
  case 0x5A62D5F0: /* RetroShader: Lightmap, Diffuse, UnusedExtendedSpecular?, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, TexLink("Alpha", 1, true)); break;
  case 0x5CB59821: /* RetroShader: Diffuse, UnusedSpecular?, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Alpha"_kcola); break;
  case 0x5D0F0069: /* RetroShader: Diffuse, Emissive, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, TexLink("Alpha", 0, true)); break;
  case 0x5D80E53C: /* RetroShader: Emissive, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0x5F0AB0E9: /* RetroShader: Lightmap, Diffuse, UnusedSpecular?, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, TexLink("Alpha", 1, true)); break;
  case 0x5F189425: /* RetroShader: Lightmap, Diffuse, UnusedSpecular?, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Alpha"_kcola); break;
  case 0x6601D113: /* RetroShader: Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Emissive"_tex); break;
  case 0x694287FA: /* RetroShader: Diffuse, Emissive, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, WhiteColorLink("Specular"), "Reflection"_tex); break;
  case 0x6D98D689: /* RetroDynamicAlphaShader: Diffuse*Dynamic, Specular, Reflection, Alpha=KAlpha*Dynamic */
    _GenerateRootShader(out, "RetroDynamicAlphaShader", "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "Alpha"_kcola); break;
  case 0x7252CB90: /* RetroShader: Lightmap, Diffuse, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Alpha"_kcola); break;
  case 0x76BEA57E: /* RetroShader: Lightmap, Diffuse, Emissive, Specular, Reflection, Alpha=1.0, IndirectTex */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex, "IndirectTex"_tex); break;
  case 0x7D6A4487: /* RetroShader: Diffuse, Specular, Reflection, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, TexLink("Alpha", 0, true)); break;
  case 0x84319328: /* RetroShader: Reflection, UnusedSpecular?, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", WhiteColorLink("Specular"), "Reflection"_tex); break;
  case 0x846215DA: /* RetroShader: Diffuse, Specular, Reflection, Alpha=DiffuseAlpha, IndirectTex */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "IndirectTex"_tex, TexLink("Alpha", 0, true)); break;
  case 0x957709F8: /* RetroShader: Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Emissive"_tex); break;
  case 0x96ABB2D3: /* RetroShader: Lightmap, Diffuse, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, TexLink("Alpha", 1, true)); break;
  case 0x985A0B67: /* RetroShader: Diffuse, UnusedSpecular?, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, TexLink("Alpha", 0, true)); break;
  case 0x9B4453A2: /* RetroShader: Diffuse, Emissive, ExtendedSpecular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, ExtendedSpecularLink(), "Reflection"_tex); break;
  case 0xA187C630: /* RetroShader: Diffuse, Emissive, UnusedReflection?, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex); break;
  case 0xC138DCFA: /* RetroShader: Diffuse, Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex); break;
  case 0xC3C8B1C8: /* RetroShader: KColorDiffuse, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_kcol, "Alpha"_kcola); break;
  case 0xC689C8C6: /* RetroShader: Diffuse, ExtendedSpecular, Reflection, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, ExtendedSpecularLink(), "Reflection"_tex, TexLink("Alpha", 0, true)); break;
  case 0xC6B18B28: /* RetroShader: Diffuse, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, TexLink("Alpha", 0, true)); break;
  case 0xCD92D4C5: /* RetroShader: Diffuse, Reflection, Alpha=KAlpha */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, WhiteColorLink("Specular"), "Reflection"_tex, "Alpha"_kcola); break;
  case 0xD73E7728: /* RetroShader: ObjLightmap, Diffuse, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, TexLink("Alpha", 1, true)); break;
  case 0xDB8F01AD: /* RetroDynamicShader: Diffuse*Dynamic, Emissive*Dynamic, UnusedSpecular?, Alpha=1.0 */
    _GenerateRootShader(out, "RetroDynamicShader", "Diffuse"_tex, "Emissive"_tex); break;
  case 0xE6784B10: /* RetroShader: Lightmap, Diffuse, Specular, Reflection, Alpha=DiffuseAlpha, IndirectTex */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Specular"_tex, "Reflection"_tex, "IndirectTex"_tex, TexLink("Alpha", 1, true)); break;
  case 0xE68FF182: /* RetroShader: Diffuse, Emissive, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0xEB4645CF: /* RetroDynamicAlphaShader: Diffuse*Dynamic, Alpha=DiffuseAlpha*Dynamic */
    _GenerateRootShader(out, "RetroDynamicAlphaShader", "Diffuse"_tex, TexLink("Alpha", 0, true)); break;
  case 0xECEF8D1F: /* RetroDynamicShader: Diffuse*Dynamic, Emissive*Dynamic, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroDynamicShader", "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0xF1C26570: /* RetroShader: Lightmap, Diffuse, Specular, ExtendedSpecular, Reflection, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Specular"_tex, "ExtendedSpecular"_tex, "Reflection"_tex, TexLink("Alpha", 1, true)); break;
  case 0xF559DB08: /* RetroShader: Lightmap, Diffuse, Emissive, Specular, Reflection, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Emissive"_tex, "Specular"_tex, "Reflection"_tex); break;
  case 0xF9324367: /* RetroShader: Lightmap, Diffuse, Emissive, Alpha=1.0 */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, "Emissive"_tex); break;
  case 0xFD95D7FD: /* RetroShader: ObjLightmap, Diffuse, Alpha=DiffuseAlpha */
    _GenerateRootShader(out, "RetroShader", "Lightmap"_tex, "Diffuse"_tex, TexLink("Alpha", 1, true)); break;
  default:
    _DescribeTEV(material);
    Log.report(logvisor::Fatal, "Unable to resolve shader hash %08X\n", hash); break;
  }

  /* Has Lightmap? */
  if (material.flags.lightmap()) {
    if (material.tevStageTexInfo[0].texSlot != 0xff)
      out << "new_material.hecl_lightmap = tex_maps[0].name\n"
             "tex_maps[0].use_fake_user = True\n";
  }

  /* Texmtx Animation Section */
  i = 0;
  for (const Material::UVAnimation& anim : material.uvAnims)
    Material::AddTextureAnim(out, anim.mode, i++, anim.vals);
}

void MaterialSet::ConstructMaterial(Stream& out, const MaterialSet::Material& material, unsigned groupIdx,
                                    unsigned matIdx) {
  _ConstructMaterial(out, material, groupIdx, matIdx);
}

MaterialSet::Material::Material(const hecl::blender::Material& mat,
                                std::vector<hecl::ProjectPath>& texPathsOut,
                                int colorCount, bool lightmapUVs, bool matrixSkinning) {
  /* TODO: Rewrite for new shader rep */
  XXH32_state_t xxHash;
  XXH32_reset(&xxHash, 0);

#if 0
  if (gx.m_kcolorCount) {
    flags.setKonstValuesEnabled(true);
    konstCount.push_back(gx.m_kcolorCount);
  }
#endif

  auto search = mat.iprops.find("retro_depth_sort");
  if (search != mat.iprops.end())
    flags.setDepthSorting(search->second != 0);

  search = mat.iprops.find("retro_alpha_test");
  if (search != mat.iprops.end())
    flags.setAlphaTest(search->second != 0);

  search = mat.iprops.find("retro_samus_reflection");
  if (search != mat.iprops.end())
    flags.setSamusReflection(search->second != 0);

  search = mat.iprops.find("retro_depth_write");
  if (search != mat.iprops.end())
    flags.setDepthWrite(search->second != 0);

  search = mat.iprops.find("retro_samus_reflection_persp");
  if (search != mat.iprops.end())
    flags.setSamusReflectionSurfaceEye(search->second != 0);

  search = mat.iprops.find("retro_shadow_occluder");
  if (search != mat.iprops.end())
    flags.setShadowOccluderMesh(search->second != 0);

  search = mat.iprops.find("retro_samus_reflection_indirect");
  if (search != mat.iprops.end())
    flags.setSamusReflectionIndirectTexture(search->second != 0);

  search = mat.iprops.find("retro_lightmapped");
  if (search != mat.iprops.end())
    flags.setLightmap(search->second != 0);

  flags.setLightmapUVArray(lightmapUVs);

#if 0
  atUint16 texFlags = 0;
  atUint16 tcgFlags = 0;
  tevStageTexInfo.reserve(gx.m_tevCount);
  textureIdxs.reserve(gx.m_tevCount);
  for (unsigned i = 0; i < gx.m_tevCount; ++i) {
    const hecl::Backend::GX::TEVStage& stage = gx.m_tevs[i];
    tevStageTexInfo.emplace_back();
    TEVStageTexInfo& texInfo = tevStageTexInfo.back();
    if (stage.m_texGenIdx != -1) {
      texInfo.tcgSlot = stage.m_texGenIdx;
      const hecl::Backend::GX::TexCoordGen& tcg = gx.m_tcgs[stage.m_texGenIdx];
      if (tcg.m_src >= hecl::Backend::GX::TG_TEX0 && tcg.m_src <= hecl::Backend::GX::TG_TEX6)
        tcgFlags |= 1 << (tcg.m_src - hecl::Backend::GX::TG_TEX0);
    }
    if (stage.m_texMapIdx != -1) {
      texInfo.texSlot = textureIdxs.size();
      const hecl::ProjectPath& texPath = texPathsIn.at(stage.m_texMapIdx);
      texFlags |= 1 << i;
      ++textureCount;
      bool found = false;
      for (size_t t = 0; t < texPathsOut.size(); ++t) {
        if (texPath == texPathsOut[t]) {
          found = true;
          textureIdxs.push_back(t);
          break;
        }
      }
      if (!found) {
        textureIdxs.push_back(texPathsOut.size());
        texPathsOut.push_back(texPath);
      }
    }
  }
  flags.setTextureSlots(texFlags);

  XXH32_update(&xxHash, &flags.flags, sizeof(flags.flags));

  vaFlags.setPosition(GX::INDEX16);
  vaFlags.setNormal(GX::INDEX16);

  if (0 < colorCount)
    vaFlags.setColor0(GX::INDEX16);
  if (1 < colorCount)
    vaFlags.setColor1(GX::INDEX16);

  if (tcgFlags & (1 << 0))
    vaFlags.setTex0(GX::INDEX16);
  if (tcgFlags & (1 << 1))
    vaFlags.setTex1(GX::INDEX16);
  if (tcgFlags & (1 << 2))
    vaFlags.setTex2(GX::INDEX16);
  if (tcgFlags & (1 << 3))
    vaFlags.setTex3(GX::INDEX16);
  if (tcgFlags & (1 << 4))
    vaFlags.setTex4(GX::INDEX16);
  if (tcgFlags & (1 << 5))
    vaFlags.setTex5(GX::INDEX16);
  if (tcgFlags & (1 << 6))
    vaFlags.setTex6(GX::INDEX16);

  if (matrixSkinning) {
    vaFlags.setPnMatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 0))
      vaFlags.setTex0MatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 1))
      vaFlags.setTex1MatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 2))
      vaFlags.setTex2MatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 3))
      vaFlags.setTex3MatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 4))
      vaFlags.setTex4MatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 5))
      vaFlags.setTex5MatIdx(GX::DIRECT);
    if (tcgFlags & (1 << 6))
      vaFlags.setTex6MatIdx(GX::DIRECT);
  }

  XXH32_update(&xxHash, &vaFlags.vaFlags, sizeof(vaFlags.vaFlags));

  XXH32_update(&xxHash, &gx.m_kcolorCount, sizeof(gx.m_kcolorCount));
  for (unsigned i = 0; i < gx.m_kcolorCount; ++i) {
    konstColors.emplace_back(gx.m_kcolors[i]);
    XXH32_update(&xxHash, &gx.m_kcolors[i].num, sizeof(gx.m_kcolors[i].num));
  }

  blendDstFac = BlendFactor(gx.m_blendDst);
  XXH32_update(&xxHash, &gx.m_blendDst, sizeof(gx.m_blendDst));
  blendSrcFac = BlendFactor(gx.m_blendSrc);
  XXH32_update(&xxHash, &gx.m_blendSrc, sizeof(gx.m_blendSrc));
  if (flags.samusReflectionIndirectTexture()) {
    indTexSlot.push_back(textureIdxs.size());
    XXH32_update(&xxHash, &indTexSlot.back(), sizeof(indTexSlot.back()));
  }

  colorChannelCount = 1;
  XXH32_update(&xxHash, &colorChannelCount, sizeof(colorChannelCount));
  colorChannels.emplace_back();
  ColorChannel& ch = colorChannels.back();
  for (unsigned i = 0; i < gx.m_tevCount; ++i) {
    const hecl::Backend::GX::TEVStage& stage = gx.m_tevs[i];
    for (int c = 0; c < 4; ++c)
      if (stage.m_color[c] == hecl::Backend::GX::CC_RASC || stage.m_color[c] == hecl::Backend::GX::CC_RASA ||
          stage.m_alpha[c] == hecl::Backend::GX::CA_RASA) {
        ch.setLighting(true);
        uint8_t one = 1;
        XXH32_update(&xxHash, &one, sizeof(one));
        break;
      }
    if (ch.lighting())
      break;
  }
  ch.setDiffuseFn(GX::DF_CLAMP);
  ch.setAttenuationFn(GX::AF_SPOT);

  tevStageCount = gx.m_tevCount;
  XXH32_update(&xxHash, &tevStageCount, sizeof(tevStageCount));
  tevStages.reserve(gx.m_tevCount);
  for (unsigned i = 0; i < gx.m_tevCount; ++i) {
    const hecl::Backend::GX::TEVStage& stage = gx.m_tevs[i];
    tevStages.emplace_back();
    TEVStage& target = tevStages.back();

    target.setColorInA(stage.m_color[0]);
    target.setColorInB(stage.m_color[1]);
    target.setColorInC(stage.m_color[2]);
    target.setColorInD(stage.m_color[3]);
    target.setAlphaInA(stage.m_alpha[0]);
    target.setAlphaInB(stage.m_alpha[1]);
    target.setAlphaInC(stage.m_alpha[2]);
    target.setAlphaInD(stage.m_alpha[3]);
    target.setColorOp(stage.m_cop);
    target.setColorOpBias(GX::TB_ZERO);
    target.setColorOpScale(GX::CS_SCALE_1);
    target.setColorOpClamp(true);
    target.setColorOpOutReg(stage.m_cRegOut);
    target.setAlphaOp(stage.m_aop);
    target.setAlphaOpBias(GX::TB_ZERO);
    target.setAlphaOpScale(GX::CS_SCALE_1);
    target.setAlphaOpClamp(true);
    target.setAlphaOpOutReg(stage.m_aRegOut);
    target.setKColorIn(stage.m_kColor);
    target.setKAlphaIn(stage.m_kAlpha);

    target.setRASIn(GX::GX_COLOR_NULL);
    for (int c = 0; c < 4; ++c)
      if (stage.m_color[c] == hecl::Backend::GX::CC_RASC || stage.m_color[c] == hecl::Backend::GX::CC_RASA ||
          stage.m_alpha[c] == hecl::Backend::GX::CA_RASA) {
        target.setRASIn(GX::GX_COLOR0A0);
        break;
      }

    XXH32_update(&xxHash, &target.ciFlags, sizeof(target.ciFlags));
    XXH32_update(&xxHash, &target.aiFlags, sizeof(target.aiFlags));
    XXH32_update(&xxHash, &target.ccFlags, sizeof(target.ccFlags));
    XXH32_update(&xxHash, &target.acFlags, sizeof(target.acFlags));
    XXH32_update(&xxHash, &target.kaInput, sizeof(target.kaInput));
    XXH32_update(&xxHash, &target.kcInput, sizeof(target.kcInput));
    XXH32_update(&xxHash, &target.rascInput, sizeof(target.rascInput));
  }

  tcgCount = gx.m_tcgCount;
  XXH32_update(&xxHash, &tcgCount, sizeof(tcgCount));
  for (unsigned i = 0; i < gx.m_tcgCount; ++i) {
    const hecl::Backend::GX::TexCoordGen& tcg = gx.m_tcgs[i];
    tcgs.emplace_back();
    TexCoordGen& target = tcgs.back();
    target.setType(GX::TG_MTX3x4);
    target.setSource(tcg.m_src);
    target.setMtx(tcg.m_mtx);
    target.setNormalize(tcg.m_norm);
    target.setPostMtx(tcg.m_pmtx);

    XXH32_update(&xxHash, &target.flags, sizeof(target.flags));
  }

  uvAnimsSize = 4;
  uvAnimsCount = 0;
  for (; uvAnimsCount < 8;) {
    bool found = false;
    for (unsigned t = 0; t < gx.m_tcgCount; ++t) {
      const hecl::Backend::GX::TexCoordGen& tcg = gx.m_tcgs[t];
      if (tcg.m_mtx == GX::IDENTITY)
        continue;
      if ((tcg.m_mtx - GX::TEXMTX0) / 3 == uvAnimsCount) {
        found = true;
        ++uvAnimsCount;
        uvAnims.emplace_back(tcg.m_gameFunction, tcg.m_gameArgs);
        XXH32_update(&xxHash, tcg.m_gameFunction.data(), sizeof(tcg.m_gameFunction.size()));
        for (const atVec4f& arg : tcg.m_gameArgs)
          XXH32_update(&xxHash, &arg, sizeof(arg));
        size_t tmpUvAnimsSize = uvAnimsSize;
        uvAnims.back().binarySize(tmpUvAnimsSize);
        uvAnimsSize = tmpUvAnimsSize;
        break;
      }
    }
    if (!found)
      break;
  }

  XXH32_update(&xxHash, &uvAnimsSize, sizeof(uvAnimsSize));
  XXH32_update(&xxHash, &uvAnimsCount, sizeof(uvAnimsCount));
#endif

  uniqueIdx = XXH32_digest(&xxHash);
}

HMDLMaterialSet::Material::Material(const hecl::blender::Material& mat) {
  auto search = mat.iprops.find("retro_depth_sort");
  if (search != mat.iprops.end())
    flags.setDepthSorting(search->second != 0);

  search = mat.iprops.find("retro_alpha_test");
  if (search != mat.iprops.end())
    flags.setAlphaTest(search->second != 0);

  search = mat.iprops.find("retro_samus_reflection");
  if (search != mat.iprops.end())
    flags.setSamusReflection(search->second != 0);

  search = mat.iprops.find("retro_depth_write");
  if (search != mat.iprops.end())
    flags.setDepthWrite(search->second != 0);

  search = mat.iprops.find("retro_samus_reflection_persp");
  if (search != mat.iprops.end())
    flags.setSamusReflectionSurfaceEye(search->second != 0);

  search = mat.iprops.find("retro_shadow_occluder");
  if (search != mat.iprops.end())
    flags.setShadowOccluderMesh(search->second != 0);

  search = mat.iprops.find("retro_samus_reflection_indirect");
  if (search != mat.iprops.end())
    flags.setSamusReflectionIndirectTexture(search->second != 0);

  search = mat.iprops.find("retro_lightmapped");
  if (search != mat.iprops.end())
    flags.setLightmap(search->second != 0);

  XXH64_state_t xxh;
  XXH64_reset(&xxh, 0);
  shaderType = mat.shaderType;
  XXH64_update(&xxh, &shaderType, sizeof(shaderType));
  chunkCount = 0;
  chunks.reserve(mat.chunks.size());
  for (const auto& chunk : mat.chunks) {
    chunk.visit([this, &xxh](const auto& var) {
      using T = std::decay_t<decltype(var)>;
      chunks.push_back(Chunk::Build(T::variant_type(), var));
      var.hash(&xxh);
      ++chunkCount;
    });
  }
  blendMode = mat.blendMode;
  XXH64_update(&xxh, &blendMode, sizeof(blendMode));
  int hashFlags = 0;
  if (flags.samusReflection())
    hashFlags |= 1;
  if (flags.samusReflectionIndirectTexture())
    hashFlags |= 2;
  if (flags.depthWrite())
    hashFlags |= 4;
  if (flags.alphaTest())
    hashFlags |= 8;
  XXH64_update(&xxh, &hashFlags, sizeof(hashFlags));
  hash = XXH64_digest(&xxh);
}

MaterialSet::Material::UVAnimation::UVAnimation(const std::string& gameFunction, const std::vector<atVec4f>& gameArgs) {
  if (!gameFunction.compare("RetroUVMode0NodeN"))
    mode = Mode::MvInvNoTranslation;
  else if (!gameFunction.compare("RetroUVMode1NodeN"))
    mode = Mode::MvInv;
  else if (!gameFunction.compare("RetroUVMode2Node")) {
    mode = Mode::Scroll;
    if (gameArgs.size() < 2)
      Log.report(logvisor::Fatal, "Mode2 UV anim requires 2 vector arguments");
    vals[0] = gameArgs[0].simd[0];
    vals[1] = gameArgs[0].simd[1];
    vals[2] = gameArgs[1].simd[0];
    vals[3] = gameArgs[1].simd[1];
  } else if (!gameFunction.compare("RetroUVMode3Node")) {
    mode = Mode::Rotation;
    if (gameArgs.size() < 2)
      Log.report(logvisor::Fatal, "Mode3 UV anim requires 2 arguments");
    vals[0] = gameArgs[0].simd[0];
    vals[1] = gameArgs[1].simd[0];
  } else if (!gameFunction.compare("RetroUVMode4Node")) {
    mode = Mode::HStrip;
    if (gameArgs.size() < 4)
      Log.report(logvisor::Fatal, "Mode4 UV anim requires 4 arguments");
    vals[0] = gameArgs[0].simd[0];
    vals[1] = gameArgs[1].simd[0];
    vals[2] = gameArgs[2].simd[0];
    vals[3] = gameArgs[3].simd[0];
  } else if (!gameFunction.compare("RetroUVMode5Node")) {
    mode = Mode::VStrip;
    if (gameArgs.size() < 4)
      Log.report(logvisor::Fatal, "Mode5 UV anim requires 4 arguments");
    vals[0] = gameArgs[0].simd[0];
    vals[1] = gameArgs[1].simd[0];
    vals[2] = gameArgs[2].simd[0];
    vals[3] = gameArgs[3].simd[0];
  } else if (!gameFunction.compare("RetroUVMode6NodeN"))
    mode = Mode::Model;
  else if (!gameFunction.compare("RetroUVMode7NodeN")) {
    mode = Mode::CylinderEnvironment;
    if (gameArgs.size() < 2)
      Log.report(logvisor::Fatal, "Mode7 UV anim requires 2 arguments");
    vals[0] = gameArgs[0].simd[0];
    vals[1] = gameArgs[1].simd[0];
  } else
    Log.report(logvisor::Fatal, "unsupported UV anim '%s'", gameFunction.c_str());
}

template <class Op>
void MaterialSet::Material::UVAnimation::Enumerate(typename Op::StreamT& s) {
  Do<Op>({}, mode, s);
  switch (mode) {
  case Mode::MvInvNoTranslation:
  case Mode::MvInv:
  case Mode::Model:
    break;
  case Mode::Scroll:
  case Mode::HStrip:
  case Mode::VStrip:
    for (int i = 0; i < 4; ++i)
      Do<Op>({}, vals[i], s);
    break;
  case Mode::Rotation:
  case Mode::CylinderEnvironment:
    for (int i = 0; i < 2; ++i)
      Do<Op>({}, vals[i], s);
    break;
  case Mode::Eight:
    for (int i = 0; i < 9; ++i)
      Do<Op>({}, vals[i], s);
    break;
  }
}

AT_SPECIALIZE_DNA(MaterialSet::Material::UVAnimation)

template <class Op>
void HMDLMaterialSet::Material::PASS::Enumerate(typename Op::StreamT& s) {
  Do<Op>({"type"}, type, s);
  Do<Op>({"texId"}, texId, s);
  Do<Op>({"source"}, source, s);
  Do<Op>({"uvAnimType"}, uvAnimType, s);
  size_t uvParmCount = uvAnimParamsCount();
  for (size_t i = 0; i < uvParmCount; ++i)
    Do<Op>({}, uvAnimParms[i], s);
  Do<Op>({"alpha"}, alpha, s);
}

AT_SPECIALIZE_DNA(HMDLMaterialSet::Material::PASS)


const char* HMDLMaterialSet::Material::PASS::DNAType() {
  return "DataSpec::DNAMP1::HMDLMaterialSet::Material::PASS";
}

} // namespace DataSpec::DNAMP1

namespace DataSpec::DNAMP2 {

void MaterialSet::ConstructMaterial(Stream& out, const MaterialSet::Material& material, unsigned groupIdx,
                                    unsigned matIdx) {
  DataSpec::DNAMP1::_ConstructMaterial(out, material, groupIdx, matIdx);
}

} // namespace DataSpec::DNAMP2
