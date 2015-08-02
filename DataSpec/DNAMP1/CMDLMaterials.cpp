#include "CMDLMaterials.hpp"
#include "../DNAMP2/CMDLMaterials.hpp"

using Stream = HECL::BlenderConnection::PyOutStream;
using namespace GX;

namespace Retro
{
namespace DNAMP1
{

void MaterialSet::RegisterMaterialProps(Stream& out)
{
    out << "bpy.types.Material.retro_depth_sort = bpy.props.BoolProperty(name='Retro: Transparent Depth Sort')\n"
           "bpy.types.Material.retro_punchthrough_alpha = bpy.props.BoolProperty(name='Retro: Punchthrough Alpha')\n"
           "bpy.types.Material.retro_samus_reflection = bpy.props.BoolProperty(name='Retro: Samus Reflection')\n"
           "bpy.types.Material.retro_depth_write = bpy.props.BoolProperty(name='Retro: Depth Write')\n"
           "bpy.types.Material.retro_samus_reflection_persp = bpy.props.BoolProperty(name='Retro: Samus Reflection Perspective')\n"
           "bpy.types.Material.retro_shadow_occluder = bpy.props.BoolProperty(name='Retro: Shadow Occluder')\n"
           "bpy.types.Material.retro_samus_reflection_indirect = bpy.props.BoolProperty(name='Retro: Samus Reflection Indirect Texture')\n"
           "bpy.types.Material.retro_lightmapped = bpy.props.BoolProperty(name='Retro: Lightmapped')\n"
           "\n";
}

static void AddTexture(Stream& out, TexGenSrc type, int mtxIdx, uint32_t texIdx)
{
    char mtxLabel[64];
    if (mtxIdx == -1)
        strncpy(mtxLabel, "IDENTITY", 64);
    else
        snprintf(mtxLabel, 64, "MTX_%u", mtxIdx);

    out.format("# Texture\n"
               "tex_uv_node = new_nodetree.nodes.new('ShaderNodeGeometry')\n"
               "tex_uv_node.label = '%s'\n"
               "tex_node = new_nodetree.nodes.new('ShaderNodeTexture')\n"
               "tex_node.label = 'Texture %u'\n"
               "texture_nodes.append(tex_node)\n"
               "gridder.place_node(tex_uv_node, 1)\n"
               "gridder.place_node(tex_node, 1)\n"
               "tex_uv_node.location[0] -= 120\n"
               "tex_node.location[0] += 120\n"
               "tex_node.location[1] += 176\n", mtxLabel, texIdx);

    if (texIdx != 0xff)
        out.format("tex_node.texture = tex_maps[%u]\n",
                   texIdx);

    if (type == GX_TG_POS)
        out.format("tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['View'], tex_node.inputs['Vector']))\n");
    else if (type == GX_TG_NRM)
        out.format("tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['Normal'], tex_node.inputs['Vector']))\n");
    else if (type >= GX_TG_TEX0 && type <= GX_TG_TEX7) {
        uint8_t texIdx = type - GX_TG_TEX0;
        out.format("tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['UV'], tex_node.inputs['Vector']))\n"
                   "tex_uv_node.uv_layer = 'UV_%u'\n", texIdx);
    }

    out << "\n";

}

static void AddTextureAnim(Stream& out,
                           MaterialSet::Material::UVAnimation::Mode type,
                           unsigned idx, const float* vals)
{
    switch (type)
    {
    case MaterialSet::Material::UVAnimation::ANIM_MV_INV_NOTRANS:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode0Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx);
        break;
    case MaterialSet::Material::UVAnimation::ANIM_MV_INV:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode1Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx);
        break;
    case MaterialSet::Material::UVAnimation::ANIM_SCROLL:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode2Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        node.inputs[1].default_value = (%f,%f,0)\n"
                   "        node.inputs[2].default_value = (%f,%f,0)\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx, vals[0], vals[1], vals[2], vals[3]);
        break;
    case MaterialSet::Material::UVAnimation::ANIM_ROTATION:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode3Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        node.inputs[1].default_value = %f\n"
                   "        node.inputs[2].default_value = %f\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx, vals[0], vals[1]);
        break;
    case MaterialSet::Material::UVAnimation::ANIM_HSTRIP:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode4Node']\n"
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
    case MaterialSet::Material::UVAnimation::ANIM_VSTRIP:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode5Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        node.inputs[1].default_value = %f\n"
                   "        node.inputs[2].default_value = %f\n"
                   "        node.inputs[3].default_value = %f\n"
                   "        node.inputs[4].default_value = %f\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx, vals[0], vals[1], vals[3], vals[2]);
        break;
    case MaterialSet::Material::UVAnimation::ANIM_MODEL:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode6Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx);
        break;
    case MaterialSet::Material::UVAnimation::ANIM_MODE_WHO_MUST_NOT_BE_NAMED:
        out.format("for link in list(tex_links):\n"
                   "    if link.from_node.label == 'MTX_%u':\n"
                   "        tex_links.remove(link)\n"
                   "        soc_from = link.from_socket\n"
                   "        soc_to = link.to_socket\n"
                   "        node = new_nodetree.nodes.new('ShaderNodeGroup')\n"
                   "        node.node_tree = bpy.data.node_groups['RWKUVMode7Node']\n"
                   "        node.location[0] = link.from_node.location[0] + 50\n"
                   "        node.location[1] = link.from_node.location[1] - 50\n"
                   "        node.inputs[1].default_value = %f\n"
                   "        node.inputs[2].default_value = %f\n"
                   "        new_nodetree.links.remove(link)\n"
                   "        new_nodetree.links.new(soc_from, node.inputs[0])\n"
                   "        new_nodetree.links.new(node.outputs[0], soc_to)\n\n",
                   idx, vals[0], vals[1]);
        break;
    default:
        break;
    }
}

static void AddKcolor(Stream& out, const Color& col, unsigned idx)
{
    out.format("# KColor\n"
               "kc_node = new_nodetree.nodes.new('ShaderNodeRGB')\n"
               "kc_node.label = 'KColor %u'\n"
               "kc_node.outputs['Color'].default_value[0] = %f\n"
               "kc_node.outputs['Color'].default_value[1] = %f\n"
               "kc_node.outputs['Color'].default_value[2] = %f\n"
               "kc_node.outputs['Color'].default_value[3] = %f\n"
               "gridder.place_node(kc_node, 1)\n"
               "\n"
               "ka_node = new_nodetree.nodes.new('ShaderNodeValue')\n"
               "ka_node.label = 'KAlpha %u'\n"
               "ka_node.outputs['Value'].default_value = %f\n"
               "gridder.place_node(ka_node, 1)\n"
               "\n"
               "kcolor_nodes.append((kc_node,ka_node))\n"
               "\n",
               idx,
               (float)col.r / (float)0xff, (float)col.g / (float)0xff,
               (float)col.b / (float)0xff, (float)col.a / (float)0xff,
               idx,
               (float)col.a / (float)0xff);

}

static void add_link(Stream& out, const char* a, const char* b)
{
    out.format("new_nodetree.links.new(%s, %s)\n", a, b);
}

enum CombinerType
{
    COMB_ADD,
    COMB_SUB,
    COMB_MULT
};
static void AddColorCombiner(Stream& out, CombinerType type,
                             const char* a, const char* b, const char* v)
{
    out << "combiner_node = new_nodetree.nodes.new('ShaderNodeMixRGB')\n"
           "combiner_node.inputs[0].default_value = 1.0\n"
           "gridder.place_node_right(combiner_node, 2, 0)\n";
    if (type == COMB_ADD)
        out << "combiner_node.blend_type = 'ADD'\n";
    else if (type == COMB_SUB)
        out << "combiner_node.blend_type = 'SUBTRACT'\n";
    else if (type == COMB_MULT)
        out << "combiner_node.blend_type = 'MULTIPLY'\n";

    if (a) {
        if (!strcmp(a, "ZERO"))
            out << "combiner_node.inputs['Color1'].default_value = (0.0, 0.0, 0.0, 0.0)\n";
        else if (!strcmp(a, "HALF"))
            out << "combiner_node.inputs['Color1'].default_value = (0.5, 0.5, 0.5, 0.5)\n";
        else if (!strcmp(a, "ONE"))
            out << "combiner_node.inputs['Color1'].default_value = (1.0, 1.0, 1.0, 1.0)\n";
        else if (!strlen(a))
        {}
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs['Color1'])\n", a);
    }

    if (b) {
        if (!strcmp(b, "ZERO"))
            out << "combiner_node.inputs['Color2'].default_value = (0.0, 0.0, 0.0, 0.0)\n";
        else if (!strcmp(b, "HALF"))
            out << "combiner_node.inputs['Color2'].default_value = (0.5, 0.5, 0.5, 0.5)\n";
        else if (!strcmp(b, "ONE"))
            out << "combiner_node.inputs['Color2'].default_value = (1.0, 1.0, 1.0, 1.0)\n";
        else if (!strlen(b))
        {}
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs['Color2'])\n", b);
    }

    if (v)
        out.format("new_nodetree.links.new(combiner_node.outputs['Color'], %s)\n", v);

    out << "color_combiner_nodes.append(combiner_node)\n\n";
}

static void AddAlphaCombiner(Stream& out, enum CombinerType type,
                             const char* a, const char* b, const char* v)
{
    out << "combiner_node = new_nodetree.nodes.new('ShaderNodeMath')\n"
           "gridder.place_node_right(combiner_node, 2, 1)\n";
    if (type == COMB_ADD)
        out << "combiner_node.operation = 'ADD'\n";
    else if (type == COMB_SUB)
        out << "combiner_node.operation = 'SUBTRACT'\n";
    else if (type == COMB_MULT)
        out << "combiner_node.operation = 'MULTIPLY'\n";

    if (a) {
        if (!strcmp(a, "ZERO"))
            out << "combiner_node.inputs[0].default_value = 0.0\n";
        else if (!strcmp(a, "HALF"))
            out << "combiner_node.inputs[0].default_value = 0.5\n";
        else if (!strcmp(a, "ONE"))
            out << "combiner_node.inputs[0].default_value = 1.0\n";
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs[0])\n", a);
    }

    if (b) {
        if (!strcmp(b, "ZERO"))
            out << "combiner_node.inputs[1].default_value = 0.0\n";
        else if (!strcmp(b, "HALF"))
            out << "combiner_node.inputs[1].default_value = 0.5\n";
        else if (!strcmp(b, "ONE"))
            out << "combiner_node.inputs[1].default_value = 1.0\n";
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs[1])\n", b);
    }

    if (v)
        out.format("new_nodetree.links.new(combiner_node.outputs[0], %s)\n", v);

    out << "alpha_combiner_nodes.append(combiner_node)\n\n";
}

static void AddLightmap(Stream& out, const char* tex, unsigned& c_combiner_idx)
{
    out << "world_light_node = new_nodetree.nodes.new('ShaderNodeRGB')\n"
           "gridder.place_node(world_light_node, 1)\n"
           "world_light_node.label = 'WORLD_LIGHTING'\n"
           "world_light_node.outputs[0].default_value = (1.0,1.0,1.0,1.0)\n";
    AddColorCombiner(out, COMB_MULT, tex, "world_light_node.outputs[0]", nullptr);
    AddColorCombiner(out, COMB_ADD,
                       "color_combiner_nodes[-1].outputs[0]",
                       "material_node.outputs['Color']", nullptr);
    c_combiner_idx += 2;
}

static void TranslateColorSocket(char* socketOut, TevColorArg arg,
                                 TevKColorSel kcolor,
                                 const MaterialSet::Material::TEVStageTexInfo& stageTex,
                                 char c_regs[4][64], char a_regs[4][64]) {
    if (arg == GX_CC_ZERO)
        strcpy(socketOut, "ZERO");
    else if (arg == GX_CC_HALF)
        strcpy(socketOut, "HALF");
    else if (arg == GX_CC_ONE)
        strcpy(socketOut, "ONE");
    else if (arg == GX_CC_TEXC) {
        if (stageTex.tcgSlot == 0xff)
            strcpy(socketOut, "ONE");
        else
            sprintf(socketOut, "texture_nodes[%u].outputs['Color']", stageTex.tcgSlot);
    } else if (arg == GX_CC_TEXA) {
        if (stageTex.tcgSlot == 0xff)
            strcpy(socketOut, "ONE");
        else
            sprintf(socketOut, "texture_nodes[%u].outputs['Value']", stageTex.tcgSlot);
    } else if (arg == GX_CC_RASC)
        strcpy(socketOut, "material_node.outputs['Color']");
    else if (arg == GX_CC_RASA) {
        strcpy(socketOut, "material_node.outputs['Alpha']");
    } else if (arg == GX_CC_KONST) {
        int kreg = (kcolor - GX_TEV_KCSEL_K0) % 4;
        if (kcolor < GX_TEV_KCSEL_K0)
            strcpy(socketOut, "ONE");
        else if (kreg == 0)
            strcpy(socketOut, "kcolor_nodes[0][0].outputs[0]");
        else if (kreg == 1)
            strcpy(socketOut, "kcolor_nodes[1][0].outputs[0]");
        else if (kreg == 2)
            strcpy(socketOut, "kcolor_nodes[2][0].outputs[0]");
        else if (kreg == 3)
            strcpy(socketOut, "kcolor_nodes[3][0].outputs[0]");
        else
            strcpy(socketOut, "ONE");
    } else if (arg == GX_CC_CPREV)
        strcpy(socketOut, c_regs[GX_TEVPREV]);
    else if (arg == GX_CC_APREV) {
        strcpy(socketOut, a_regs[GX_TEVPREV]);
    } else if (arg == GX_CC_C0)
        strcpy(socketOut, c_regs[GX_TEVREG0]);
    else if (arg == GX_CC_A0) {
        strcpy(socketOut, a_regs[GX_TEVREG0]);
    } else if (arg == GX_CC_C1)
        strcpy(socketOut, c_regs[GX_TEVREG1]);
    else if (arg == GX_CC_A1) {
        strcpy(socketOut, a_regs[GX_TEVREG1]);
    } else if (arg == GX_CC_C2)
        strcpy(socketOut, c_regs[GX_TEVREG2]);
    else if (arg == GX_CC_A2) {
        strcpy(socketOut, a_regs[GX_TEVREG2]);
    }
}

static void TranslateAlphaSocket(char* socketOut, TevAlphaArg arg,
                                 TevKAlphaSel kalpha,
                                 const MaterialSet::Material::TEVStageTexInfo& stageTex,
                                 char a_regs[4][64]) {
    if (arg == GX_CA_ZERO)
        strcpy(socketOut, "ZERO");
    else if (arg == GX_CA_TEXA) {
        if (stageTex.tcgSlot == 0xff)
            strcpy(socketOut, "ONE");
        else
            sprintf(socketOut, "texture_nodes[%u].outputs['Value']", stageTex.tcgSlot);
    } else if (arg == GX_CA_RASA)
        strcpy(socketOut, "material_node.outputs['Alpha']");
    else if (arg == GX_CA_KONST) {
        int kreg = kalpha - GX_TEV_KASEL_K0_A;
        if (kreg == 0)
            strcpy(socketOut, "kcolor_nodes[0][1].outputs[0]");
        else if (kreg == 1)
            strcpy(socketOut, "kcolor_nodes[1][1].outputs[0]");
        else if (kreg == 2)
            strcpy(socketOut, "kcolor_nodes[2][1].outputs[0]");
        else if (kreg == 3)
            strcpy(socketOut, "kcolor_nodes[3][1].outputs[0]");
        else
            strcpy(socketOut, "ONE");
    } else if (arg == GX_CA_APREV)
        strcpy(socketOut, a_regs[GX_TEVPREV]);
    else if (arg == GX_CA_A0)
        strcpy(socketOut, a_regs[GX_TEVREG0]);
    else if (arg == GX_CA_A1)
        strcpy(socketOut, a_regs[GX_TEVREG1]);
    else if (arg == GX_CA_A2)
        strcpy(socketOut, a_regs[GX_TEVREG2]);
}

static void AddTEVStage(Stream& out, const MaterialSet::Material::TEVStage& stage,
                        const MaterialSet::Material::TEVStageTexInfo& stageTex,
                        char c_regs[4][64], char a_regs[4][64],
                        unsigned& c_combiner_idx, unsigned& a_combiner_idx)
{
    char ca[64];
    char cb[64];
    char cc[64];
    char cd[64];
    TranslateColorSocket(ca, stage.colorInA(), stage.kColorIn(), stageTex, c_regs, a_regs);
    TranslateColorSocket(cb, stage.colorInB(), stage.kColorIn(), stageTex, c_regs, a_regs);
    TranslateColorSocket(cc, stage.colorInC(), stage.kColorIn(), stageTex, c_regs, a_regs);
    TranslateColorSocket(cd, stage.colorInD(), stage.kColorIn(), stageTex, c_regs, a_regs);

    char aa[64];
    char ab[64];
    char ac[64];
    char ad[64];
    TranslateAlphaSocket(aa, stage.alphaInA(), stage.kAlphaIn(), stageTex, a_regs);
    TranslateAlphaSocket(ab, stage.alphaInB(), stage.kAlphaIn(), stageTex, a_regs);
    TranslateAlphaSocket(ac, stage.alphaInC(), stage.kAlphaIn(), stageTex, a_regs);
    TranslateAlphaSocket(ad, stage.alphaInD(), stage.kAlphaIn(), stageTex, a_regs);

    /* Apply color optimizations */
    unsigned c_tev_opts = 0;
    if (stage.colorInA() == GX_CC_ZERO || stage.colorInC() == GX_CC_ONE)
        c_tev_opts |= 1;
    if (stage.colorInB() == GX_CC_ZERO || stage.colorInC() == GX_CC_ZERO)
        c_tev_opts |= 2;
    if (c_tev_opts & 1 || c_tev_opts & 2)
        c_tev_opts |= 4;
    if (stage.colorInD() == GX_CC_ZERO || (c_tev_opts & 7) == 7)
        c_tev_opts |= 8;

    if (!(c_tev_opts & 1))
    {
        /* A nodes */
        AddColorCombiner(out, COMB_SUB, "ONE", ca, NULL);
        ++c_combiner_idx;
        AddColorCombiner(out, COMB_MULT, "color_combiner_nodes[-1].outputs[0]", cc, NULL);
        ++c_combiner_idx;
    }

    if (!(c_tev_opts & 2))
    {
        /* B nodes */
        AddColorCombiner(out, COMB_MULT, cb, cc, NULL);
        ++c_combiner_idx;
    }

    if (!(c_tev_opts & 4))
    {
        /* A+B node */
        AddColorCombiner(out, COMB_ADD, "color_combiner_nodes[-2].outputs[0]", "color_combiner_nodes[-1].outputs[0]", NULL);
        ++c_combiner_idx;
    }

    if (!(c_tev_opts & 8))
    {
        /* +D node */
        AddColorCombiner(out, COMB_ADD, "color_combiner_nodes[-1].outputs[0]", cd, NULL);
        ++c_combiner_idx;
    }

    /* Apply alpha optimizations */
    unsigned a_tev_opts = 0;
    if (stage.alphaInA() == GX_CA_ZERO)
        a_tev_opts |= 1;
    if (stage.alphaInB() == GX_CA_ZERO || stage.alphaInC() == GX_CA_ZERO)
        a_tev_opts |= 2;
    if (a_tev_opts & 1 || a_tev_opts & 2)
        a_tev_opts |= 4;
    if (stage.alphaInD() == GX_CA_ZERO || (a_tev_opts & 7) == 7)
        a_tev_opts |= 8;

    if (!(a_tev_opts & 1))
    {
        /* A nodes */
        AddAlphaCombiner(out, COMB_SUB, "ONE", aa, NULL);
        ++a_combiner_idx;
        AddAlphaCombiner(out, COMB_MULT, "alpha_combiner_nodes[-1].outputs[0]", ac, NULL);
        ++a_combiner_idx;
    }

    if (!(a_tev_opts & 2))
    {
        /* B nodes */
        AddAlphaCombiner(out, COMB_MULT, ab, ac, NULL);
        ++a_combiner_idx;
    }

    if (!(a_tev_opts & 4))
    {
        /* A+B node */
        AddAlphaCombiner(out, COMB_ADD, "alpha_combiner_nodes[-2].outputs[0]", "alpha_combiner_nodes[-1].outputs[0]", NULL);
        ++a_combiner_idx;
    }

    if (!(a_tev_opts & 8)) {
        /* +D node */
        AddAlphaCombiner(out, COMB_ADD, "alpha_combiner_nodes[-1].outputs[0]", ad, NULL);
        ++a_combiner_idx;
    }

    /* Update TEV regs */
    if (c_tev_opts == 0xf)
    {
        if (stage.colorInD() != GX_CC_ZERO)
            strncpy(c_regs[stage.colorOpOutReg()], cd, 64);
    } else
        snprintf(c_regs[stage.colorOpOutReg()], 64, "color_combiner_nodes[%u].outputs[0]", c_combiner_idx - 1);
    if (a_tev_opts == 0xf)
    {
        if (stage.alphaInD() != GX_CA_ZERO)
            strncpy(a_regs[stage.alphaOpOutReg()], ad, 64);
    } else
        snprintf(a_regs[stage.alphaOpOutReg()], 64, "alpha_combiner_nodes[%u].outputs[0]", a_combiner_idx - 1);

    /* Row Break in gridder */
    out << "gridder.row_break(2)\n";

}

template <class MAT>
void _ConstructMaterial(Stream& out,
                        const MAT& material,
                        unsigned groupIdx,
                        unsigned matIdx,
                        unsigned& uvCountOut)
{
    unsigned i;

    out.format("new_material = bpy.data.materials.new('MAT_%u_%u')\n"
               "new_material.use_shadows = True\n"
               "new_material.use_transparent_shadows = True\n"
               "new_material.diffuse_color = (1.0,1.0,1.0)\n"
               "new_material.diffuse_intensity = 1.0\n"
               "new_material.specular_intensity = 0.0\n"
               "new_material.use_nodes = True\n"
               "new_nodetree = new_material.node_tree\n"
               "material_node = new_nodetree.nodes['Material']\n"
               "final_node = new_nodetree.nodes['Output']\n"
               "\n"
               "gridder = nodegrid(new_nodetree)\n"
               "gridder.place_node(final_node, 3)\n"
               "gridder.place_node(material_node, 0)\n"
               "material_node.material = new_material\n"
               "\n"
               "texture_nodes = []\n"
               "kcolor_nodes = []\n"
               "color_combiner_nodes = []\n"
               "alpha_combiner_nodes = []\n"
               "tex_links = []\n"
               "tev_reg_sockets = [None]*4\n"
               "\n", groupIdx, matIdx);

    /* Material Flags */
    out.format("new_material.retro_depth_sort = %s\n"
               "new_material.retro_punchthrough_alpha = %s\n"
               "new_material.retro_samus_reflection = %s\n"
               "new_material.retro_depth_write = %s\n"
               "new_material.retro_samus_reflection_persp = %s\n"
               "new_material.retro_shadow_occluder = %s\n"
               "new_material.retro_samus_reflection_indirect = %s\n"
               "new_material.retro_lightmapped = %s\n"
               "new_material.game_settings.invisible = %s\n",
               material.flags.depthSorting() ? "True" : "False",
               material.flags.punchthroughAlpha() ? "True" : "False",
               material.flags.samusReflection() ? "True" : "False",
               material.flags.depthWrite() ? "True" : "False",
               material.flags.samusReflectionSurfaceEye() ? "True" : "False",
               material.flags.shadowOccluderMesh() ? "True" : "False",
               material.flags.samusReflectionIndirectTexture() ? "True" : "False",
               material.flags.lightmap() ? "True" : "False",
               material.flags.shadowOccluderMesh() ? "True" : "False");


    /* Texture Indices */
    out << "tex_maps = []\n";
    for (atUint32 idx : material.texureIdxs)
        out.format("tex_maps.append(texmap_list[%u])\n", idx);

    /* Vertex Attribute Flags */
    uvCountOut = 0;
    if (material.vaFlags.tex0())
        ++uvCountOut;
    if (material.vaFlags.tex1())
        ++uvCountOut;
    if (material.vaFlags.tex2())
        ++uvCountOut;
    if (material.vaFlags.tex3())
        ++uvCountOut;
    if (material.vaFlags.tex4())
        ++uvCountOut;
    if (material.vaFlags.tex5())
        ++uvCountOut;
    if (material.vaFlags.tex6())
        ++uvCountOut;

    /* KColor entries */
    if (material.flags.konstValuesEnabled())
    {
        unsigned i=0;
        for (const Color& col : material.konstColors)
            AddKcolor(out, col, i++);
    }

    /* Blend factors */
    using BlendFactor = MaterialSet::Material::BlendFactor;
    if (material.blendDestFactor() != BlendFactor::GX_BL_ZERO)
    {
        if (material.blendDestFactor() == BlendFactor::GX_BL_ZERO)
            out << "new_material.game_settings.alpha_blend = 'ADD'\n"
                   "new_material.use_transparency = True\n"
                   "new_material.transparency_method = 'RAYTRACE'\n"
                   "new_material.alpha = 1.0\n";
        else
            out << "new_material.game_settings.alpha_blend = 'ALPHA'\n"
                   "new_material.use_transparency = True\n"
                   "new_material.transparency_method = 'RAYTRACE'\n"
                   "new_material.alpha = 1.0\n";
    }

    /* Color channels (for combining dynamic lighting) */
    for (const MaterialSet::Material::ColorChannel& chan : material.colorChannels)
    {
        if (!chan.lighting())
            out << "new_material.use_shadeless = True\n";
    }

    /* Add texture maps/tcgs */
    unsigned addedTcgs = 0;
    for (i=0 ; i<material.tevStageCount ; ++i)
    {
        if (material.tevStageTexInfo[i].tcgSlot != 0xff &&
            !(addedTcgs >> material.tevStageTexInfo[i].tcgSlot & 1))
        {
            const MaterialSet::Material::TexCoordGen& tcg = material.tcgs[material.tevStageTexInfo[i].tcgSlot];
            TexMtx mtx = tcg.mtx();
            int mtxIdx = -1;
            if (mtx >= GX_TEXMTX0 && mtx <= GX_TEXMTX9)
                mtxIdx = (mtx - GX_TEXMTX0) / 3;
            AddTexture(out, tcg.source(), mtxIdx, material.tevStageTexInfo[i].texSlot);
            addedTcgs |= 1 << material.tevStageTexInfo[i].tcgSlot;
        }
    }

    /* TEV-emulation combiner-node index context */
    unsigned c_combiner_idx = 0;
    unsigned a_combiner_idx = 0;

    /* Initialze TEV register sockets */
    char c_regs[4][64] = {"ONE", "ONE", "ONE", "ONE"};
    char a_regs[4][64] = {"ONE", "ONE", "ONE", "ONE"};

    /* Has Lightmap? */
    if (material.tevStages[0].colorInB() == GX_CC_C1)
    {
        if (material.tevStageTexInfo[0].texSlot != 0xff)
            out << "new_material.rwk_lightmap = tex_maps[0].name\n"
                   "tex_maps[0].image.use_fake_user = True\n";
        AddLightmap(out, "texture_nodes[0].outputs['Color']", c_combiner_idx);
        strncpy(c_regs[GX_TEVREG1], "world_light_node.outputs[0]", 64);
    }

    /* Add TEV stages */
    for (i=0 ; i<material.tevStageCount ; ++i)
    {
        const MaterialSet::Material::TEVStage& stage = material.tevStages[i];
        const MaterialSet::Material::TEVStageTexInfo& stage_tex = material.tevStageTexInfo[i];
        AddTEVStage(out, stage, stage_tex, c_regs, a_regs, c_combiner_idx, a_combiner_idx);
    }

    /* Connect final prev register */
    out.format("new_nodetree.links.new(%s, final_node.inputs['Color'])\n", c_regs[GX_TEVPREV]);
    if (!strcmp(a_regs[GX_TEVPREV], "ONE"))
        out << "final_node.inputs['Alpha'].default_value = 1.0\n";
    else
        out.format("new_nodetree.links.new(%s, final_node.inputs['Alpha'])\n", a_regs[GX_TEVPREV]);

    /* Texmtx Animation Section */
    i=0;
    for (const MaterialSet::Material::UVAnimation& anim : material.uvAnims)
        AddTextureAnim(out, anim.mode, i++, anim.vals);
}

void MaterialSet::ConstructMaterial(Stream& out,
                                    const MaterialSet::Material& material,
                                    unsigned groupIdx,
                                    unsigned matIdx,
                                    unsigned& uvCountOut)
{_ConstructMaterial(out, material, groupIdx, matIdx, uvCountOut);}

}
}

namespace Retro
{
namespace DNAMP2
{

void MaterialSet::ConstructMaterial(Stream& out,
                                    const MaterialSet::Material& material,
                                    unsigned groupIdx,
                                    unsigned matIdx,
                                    unsigned& uvCountOut)
{Retro::DNAMP1::_ConstructMaterial(out, material, groupIdx, matIdx, uvCountOut);}

}
}
