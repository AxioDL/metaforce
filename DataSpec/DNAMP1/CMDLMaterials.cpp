#include "CMDLMaterials.hpp"
#include "../DNAMP2/CMDLMaterials.hpp"
#include "hecl/Blender/Connection.hpp"

using Stream = hecl::blender::PyOutStream;

namespace DataSpec::DNAMP1
{
using Material = MaterialSet::Material;

void MaterialSet::RegisterMaterialProps(Stream& out)
{
    out << "bpy.types.Material.retro_depth_sort = bpy.props.BoolProperty(name='Retro: Transparent Depth Sort')\n"
           "bpy.types.Material.retro_alpha_test = bpy.props.BoolProperty(name='Retro: Punchthrough Alpha')\n"
           "bpy.types.Material.retro_samus_reflection = bpy.props.BoolProperty(name='Retro: Samus Reflection')\n"
           "bpy.types.Material.retro_depth_write = bpy.props.BoolProperty(name='Retro: Depth Write')\n"
           "bpy.types.Material.retro_samus_reflection_persp = bpy.props.BoolProperty(name='Retro: Samus Reflection Perspective')\n"
           "bpy.types.Material.retro_shadow_occluder = bpy.props.BoolProperty(name='Retro: Shadow Occluder')\n"
           "bpy.types.Material.retro_samus_reflection_indirect = bpy.props.BoolProperty(name='Retro: Samus Reflection Indirect Texture')\n"
           "bpy.types.Material.retro_lightmapped = bpy.props.BoolProperty(name='Retro: Lightmapped')\n"
           "\n";
}

void Material::AddTexture(Stream& out, GX::TexGenSrc type, int mtxIdx, uint32_t texIdx)
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
        out.format("tex_node.texture = tex_maps[%u]\n", texIdx);

    if (type == GX::TG_POS)
        out.format("tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['View'], tex_node.inputs['Vector']))\n");
    else if (type == GX::TG_NRM)
        out.format("tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['Normal'], tex_node.inputs['Vector']))\n");
    else if (type >= GX::TG_TEX0 && type <= GX::TG_TEX7)
    {
        uint8_t texIdx = type - GX::TG_TEX0;
        out.format("tex_links.append(new_nodetree.links.new(tex_uv_node.outputs['UV'], tex_node.inputs['Vector']))\n"
                   "tex_uv_node.uv_layer = 'UV_%u'\n", texIdx);
    }

    out << "\n";

}

void Material::AddTextureAnim(Stream& out,
                              UVAnimation::Mode type,
                              unsigned idx, const float* vals)
{
    switch (type)
    {
    case UVAnimation::Mode::MvInvNoTranslation:
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
        out.format("for link in list(tex_links):\n"
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
                   idx, vals[0], vals[1], vals[2], vals[3],
                   vals[4], vals[5], vals[6], vals[7], vals[8]);
        break;
    default: break;
    }
}

void Material::AddKcolor(Stream& out, const GX::Color& col, unsigned idx)
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
               (float)col.color[0] / (float)0xff, (float)col.color[1] / (float)0xff,
               (float)col.color[2] / (float)0xff, (float)col.color[3] / (float)0xff,
               idx,
               (float)col.color[3] / (float)0xff);
}

void Material::AddDynamicColor(Stream& out, unsigned idx)
{
    out.format("# Dynamic Color\n"
               "node_name = 'DYNAMIC_C_%u'\n"
               "if node_name not in new_nodetree.nodes:\n"
               "    dyn_c_node = new_nodetree.nodes.new('ShaderNodeRGB')\n"
               "    dyn_c_node.name = node_name\n"
               "    dyn_c_node.label = 'DYNAMIC_%u'\n"
               "    dyn_c_node.outputs['Color'].default_value = (1.0,1.0,1.0,1.0)\n"
               "    gridder.place_node(dyn_c_node, 1)\n"
               "\n",
               idx, idx);
}

void Material::AddDynamicAlpha(Stream& out, unsigned idx)
{
    out.format("# Dynamic Alpha\n"
               "node_name = 'DYNAMIC_A_%u'\n"
               "if node_name not in new_nodetree.nodes:\n"
               "    dyn_a_node = new_nodetree.nodes.new('ShaderNodeValue')\n"
               "    dyn_a_node.name = node_name\n"
               "    dyn_a_node.label = 'DYNAMIC_%u'\n"
               "    dyn_a_node.outputs['Value'].default_value = 1.0\n"
               "    gridder.place_node(dyn_a_node, 1)\n"
               "\n",
               idx, idx);
}


enum class Combiner
{
    Add,
    Sub,
    Mult
};
static void AddColorCombiner(Stream& out, Combiner type,
                             const char* a, const char* b, const char* v)
{
    out << "combiner_node = new_nodetree.nodes.new('ShaderNodeMixRGB')\n"
           "combiner_node.inputs[0].default_value = 1.0\n"
           "gridder.place_node_right(combiner_node, 2, 0)\n";
    if (type == Combiner::Add)
        out << "combiner_node.blend_type = 'ADD'\n";
    else if (type == Combiner::Sub)
        out << "combiner_node.blend_type = 'SUBTRACT'\n";
    else if (type == Combiner::Mult)
        out << "combiner_node.blend_type = 'MULTIPLY'\n";

    if (a)
    {
        if (!strcmp(a, "ZERO"))
            out << "combiner_node.inputs['Color1'].default_value = (0.0, 0.0, 0.0, 0.0)\n";
        else if (!strcmp(a, "HALF"))
            out << "combiner_node.inputs['Color1'].default_value = (0.5, 0.5, 0.5, 0.5)\n";
        else if (!strcmp(a, "ONE"))
            out << "combiner_node.inputs['Color1'].default_value = (1.0, 1.0, 1.0, 1.0)\n";
        else if (!strcmp(a, "D0"))
        {
            Material::AddDynamicColor(out, 0);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_C_0'].outputs['Color'], combiner_node.inputs['Color1'])\n";
        }
        else if (!strcmp(a, "D1"))
        {
            Material::AddDynamicColor(out, 1);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_C_1'].outputs['Color'], combiner_node.inputs['Color1'])\n";
        }
        else if (!strcmp(a, "D2"))
        {
            Material::AddDynamicColor(out, 2);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_C_2'].outputs['Color'], combiner_node.inputs['Color1'])\n";
        }
        else if (!strlen(a))
        {}
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs['Color1'])\n", a);
    }

    if (b)
    {
        if (!strcmp(b, "ZERO"))
            out << "combiner_node.inputs['Color2'].default_value = (0.0, 0.0, 0.0, 0.0)\n";
        else if (!strcmp(b, "HALF"))
            out << "combiner_node.inputs['Color2'].default_value = (0.5, 0.5, 0.5, 0.5)\n";
        else if (!strcmp(b, "ONE"))
            out << "combiner_node.inputs['Color2'].default_value = (1.0, 1.0, 1.0, 1.0)\n";
        else if (!strcmp(b, "D0"))
        {
            Material::AddDynamicColor(out, 0);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_C_0'].outputs['Color'], combiner_node.inputs['Color2'])\n";
        }
        else if (!strcmp(b, "D1"))
        {
            Material::AddDynamicColor(out, 1);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_C_1'].outputs['Color'], combiner_node.inputs['Color2'])\n";
        }
        else if (!strcmp(b, "D2"))
        {
            Material::AddDynamicColor(out, 2);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_C_2'].outputs['Color'], combiner_node.inputs['Color2'])\n";
        }
        else if (!strlen(b))
        {}
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs['Color2'])\n", b);
    }

    if (v)
        out.format("new_nodetree.links.new(combiner_node.outputs['Color'], %s)\n", v);

    out << "color_combiner_sockets.append(combiner_node.outputs['Color'])\n\n";
}

static void AddAlphaCombiner(Stream& out, Combiner type,
                             const char* a, const char* b, const char* v)
{
    out << "combiner_node = new_nodetree.nodes.new('ShaderNodeMath')\n"
           "gridder.place_node_right(combiner_node, 2, 1)\n";
    if (type == Combiner::Add)
        out << "combiner_node.operation = 'ADD'\n";
    else if (type == Combiner::Sub)
        out << "combiner_node.operation = 'SUBTRACT'\n";
    else if (type == Combiner::Mult)
        out << "combiner_node.operation = 'MULTIPLY'\n";

    if (a)
    {
        if (!strcmp(a, "ZERO"))
            out << "combiner_node.inputs[0].default_value = 0.0\n";
        else if (!strcmp(a, "HALF"))
            out << "combiner_node.inputs[0].default_value = 0.5\n";
        else if (!strcmp(a, "ONE"))
            out << "combiner_node.inputs[0].default_value = 1.0\n";
        else if (!strcmp(a, "D0"))
        {
            Material::AddDynamicAlpha(out, 0);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_A_0'].outputs[0], combiner_node.inputs[0])\n";
        }
        else if (!strcmp(a, "D1"))
        {
            Material::AddDynamicAlpha(out, 1);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_A_1'].outputs[0], combiner_node.inputs[0])\n";
        }
        else if (!strcmp(a, "D2"))
        {
            Material::AddDynamicAlpha(out, 2);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_A_2'].outputs[0], combiner_node.inputs[0])\n";
        }
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs[0])\n", a);
    }

    if (b)
    {
        if (!strcmp(b, "ZERO"))
            out << "combiner_node.inputs[1].default_value = 0.0\n";
        else if (!strcmp(b, "HALF"))
            out << "combiner_node.inputs[1].default_value = 0.5\n";
        else if (!strcmp(b, "ONE"))
            out << "combiner_node.inputs[1].default_value = 1.0\n";
        else if (!strcmp(b, "D0"))
        {
            Material::AddDynamicAlpha(out, 0);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_A_0'].outputs[0], combiner_node.inputs[1])\n";
        }
        else if (!strcmp(b, "D1"))
        {
            Material::AddDynamicAlpha(out, 1);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_A_1'].outputs[0], combiner_node.inputs[1])\n";
        }
        else if (!strcmp(b, "D2"))
        {
            Material::AddDynamicAlpha(out, 2);
            out << "new_nodetree.links.new(new_nodetree.nodes['DYNAMIC_A_2'].outputs[0], combiner_node.inputs[1])\n";
        }
        else
            out.format("new_nodetree.links.new(%s, combiner_node.inputs[1])\n", b);
    }

    if (v)
        out.format("new_nodetree.links.new(combiner_node.outputs[0], %s)\n", v);

    out << "alpha_combiner_sockets.append(combiner_node.outputs[0])\n\n";
}

static void TranslateColorSocket(char* socketOut, GX::TevColorArg arg,
                                 GX::TevKColorSel kcolor,
                                 const MaterialSet::Material::TEVStageTexInfo& stageTex,
                                 char c_regs[4][64], char a_regs[4][64]) {
    if (arg == GX::CC_ZERO)
        strcpy(socketOut, "ZERO");
    else if (arg == GX::CC_HALF)
        strcpy(socketOut, "HALF");
    else if (arg == GX::CC_ONE)
        strcpy(socketOut, "ONE");
    else if (arg == GX::CC_TEXC)
    {
        if (stageTex.tcgSlot == 0xff)
            strcpy(socketOut, "ONE");
        else
            sprintf(socketOut, "texture_nodes[%u].outputs['Color']", stageTex.tcgSlot);
    }
    else if (arg == GX::CC_TEXA) {
        if (stageTex.tcgSlot == 0xff)
            strcpy(socketOut, "ONE");
        else
            sprintf(socketOut, "texture_nodes[%u].outputs['Value']", stageTex.tcgSlot);
    }
    else if (arg == GX::CC_RASC)
        strcpy(socketOut, "material_node.outputs['Color']");
    else if (arg == GX::CC_RASA)
        strcpy(socketOut, "material_node.outputs['Alpha']");
    else if (arg == GX::CC_KONST)
    {
        int kreg = (kcolor - GX::TEV_KCSEL_K0) % 4;
        if (kcolor < GX::TEV_KCSEL_K0)
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
    }
    else if (arg == GX::CC_CPREV)
        strcpy(socketOut, c_regs[GX::TEVPREV]);
    else if (arg == GX::CC_APREV)
        strcpy(socketOut, a_regs[GX::TEVPREV]); 
    else if (arg == GX::CC_C0)
        strcpy(socketOut, c_regs[GX::TEVREG0]);
    else if (arg == GX::CC_A0)
        strcpy(socketOut, a_regs[GX::TEVREG0]);
    else if (arg == GX::CC_C1)
        strcpy(socketOut, c_regs[GX::TEVREG1]);
    else if (arg == GX::CC_A1)
        strcpy(socketOut, a_regs[GX::TEVREG1]);
    else if (arg == GX::CC_C2)
        strcpy(socketOut, c_regs[GX::TEVREG2]);
    else if (arg == GX::CC_A2)
        strcpy(socketOut, a_regs[GX::TEVREG2]);
}

static void TranslateAlphaSocket(char* socketOut, GX::TevAlphaArg arg,
                                 GX::TevKAlphaSel kalpha,
                                 const MaterialSet::Material::TEVStageTexInfo& stageTex,
                                 char a_regs[4][64]) {
    if (arg == GX::CA_ZERO)
        strcpy(socketOut, "ZERO");
    else if (arg == GX::CA_TEXA)
    {
        if (stageTex.tcgSlot == 0xff)
            strcpy(socketOut, "ONE");
        else
            sprintf(socketOut, "texture_nodes[%u].outputs['Value']", stageTex.tcgSlot);
    }
    else if (arg == GX::CA_RASA)
        strcpy(socketOut, "material_node.outputs['Alpha']");
    else if (arg == GX::CA_KONST) {
        int kreg = kalpha - GX::TEV_KASEL_K0_A;
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
    }
    else if (arg == GX::CA_APREV)
        strcpy(socketOut, a_regs[GX::TEVPREV]);
    else if (arg == GX::CA_A0)
        strcpy(socketOut, a_regs[GX::TEVREG0]);
    else if (arg == GX::CA_A1)
        strcpy(socketOut, a_regs[GX::TEVREG1]);
    else if (arg == GX::CA_A2)
        strcpy(socketOut, a_regs[GX::TEVREG2]);
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
    if (stage.colorInA() == GX::CC_ZERO || stage.colorInC() == GX::CC_ONE)
        c_tev_opts |= 1;
    if (stage.colorInB() == GX::CC_ZERO || stage.colorInC() == GX::CC_ZERO)
        c_tev_opts |= 2;
    if (c_tev_opts & 1 || c_tev_opts & 2)
        c_tev_opts |= 4;
    if (stage.colorInD() == GX::CC_ZERO || (c_tev_opts & 7) == 7)
        c_tev_opts |= 8;

    /* Special A/D (additive) optimization */
    if (stage.colorInA() != GX::CC_ZERO &&
        stage.colorInB() == GX::CC_ZERO &&
        stage.colorInC() == GX::CC_ZERO &&
        stage.colorInD() != GX::CC_ZERO)
    {
        c_tev_opts |= 0x1f;
        AddColorCombiner(out, Combiner::Add, cd, ca, nullptr);
        ++c_combiner_idx;
    }
    else if (stage.colorInA() != GX::CC_ZERO &&
             stage.colorInB() == GX::CC_ZERO &&
             stage.colorInC() == GX::CC_ZERO &&
             stage.colorInD() == GX::CC_ZERO)
    {
        c_tev_opts |= 0xf;
    }
    else if (stage.colorInA() == GX::CC_ZERO &&
             stage.colorInB() == GX::CC_ZERO &&
             stage.colorInC() == GX::CC_ZERO &&
             stage.colorInD() != GX::CC_ZERO)
    {
        c_tev_opts |= 0xf;
    }

    if (!(c_tev_opts & 1))
    {
        /* A nodes */
        AddColorCombiner(out, Combiner::Sub, "ONE", ca, nullptr);
        ++c_combiner_idx;
        if (strcmp(cc, "ONE"))
        {
            AddColorCombiner(out, Combiner::Mult, cc, "color_combiner_sockets[-1]", nullptr);
            ++c_combiner_idx;
        }
    }
    
    const char* c_soc_log[2] = {"color_combiner_sockets[-1]", "color_combiner_sockets[-2]"};

    if (!(c_tev_opts & 2))
    {
        /* B nodes */
        if (!strcmp(cc, "ONE"))
        {
            if (strcmp(cb, "ZERO") && strcmp(cb, "HALF") && strcmp(cb, "ONE") &&
                strcmp(cb, "D0") && strcmp(cb, "D1") && strcmp(cb, "D2"))
            {
                out.format("color_combiner_sockets.append(%s)\n", cb);
                ++c_combiner_idx;
            }
            else
            {
                c_soc_log[1] = c_soc_log[0];
                c_soc_log[0] = cb;
            }
        }
        else
        {
            AddColorCombiner(out, Combiner::Mult, cc, cb, nullptr);
            ++c_combiner_idx;
        }
    }

    if (!(c_tev_opts & 4))
    {
        /* A+B node */
        AddColorCombiner(out, Combiner::Add, c_soc_log[0], c_soc_log[1], nullptr);
        ++c_combiner_idx;
    }

    if (!(c_tev_opts & 8))
    {
        /* +D node */
        AddColorCombiner(out, Combiner::Add, cd, c_soc_log[0], nullptr);
        ++c_combiner_idx;
    }

    /* Apply alpha optimizations */
    unsigned a_tev_opts = 0;
    if (stage.alphaInA() == GX::CA_ZERO)
        a_tev_opts |= 1;
    if (stage.alphaInB() == GX::CA_ZERO || stage.alphaInC() == GX::CA_ZERO)
        a_tev_opts |= 2;
    if (a_tev_opts & 1 || a_tev_opts & 2)
        a_tev_opts |= 4;
    if (stage.alphaInD() == GX::CA_ZERO || (a_tev_opts & 7) == 7)
        a_tev_opts |= 8;

    /* Special A/D (additive) optimization */
    if (stage.alphaInA() != GX::CA_ZERO &&
        stage.alphaInB() == GX::CA_ZERO &&
        stage.alphaInC() == GX::CA_ZERO &&
        stage.alphaInD() != GX::CA_ZERO)
    {
        a_tev_opts |= 0x1f;
        AddAlphaCombiner(out, Combiner::Add, ad, aa, nullptr);
        ++a_combiner_idx;
    }
    else if (stage.alphaInA() != GX::CA_ZERO &&
             stage.alphaInB() == GX::CA_ZERO &&
             stage.alphaInC() == GX::CA_ZERO &&
             stage.alphaInD() == GX::CA_ZERO)
    {
        a_tev_opts |= 0xf;
    }
    else if (stage.alphaInA() == GX::CA_ZERO &&
             stage.alphaInB() == GX::CA_ZERO &&
             stage.alphaInC() == GX::CA_ZERO &&
             stage.alphaInD() != GX::CA_ZERO)
    {
        a_tev_opts |= 0xf;
    }

    if (!(a_tev_opts & 1))
    {
        /* A nodes */
        AddAlphaCombiner(out, Combiner::Sub, "ONE", aa, nullptr);
        ++a_combiner_idx;
        if (strcmp(ac, "ONE"))
        {
            AddAlphaCombiner(out, Combiner::Mult, ac, "alpha_combiner_sockets[-1]", nullptr);
            ++a_combiner_idx;
        }
    }
    
    const char* a_soc_log[2] = {"alpha_combiner_sockets[-1]", "alpha_combiner_sockets[-2]"};

    if (!(a_tev_opts & 2))
    {
        /* B nodes */
        if (!strcmp(ac, "ONE"))
        {
            if (strcmp(ab, "ZERO") && strcmp(ab, "HALF") && strcmp(ab, "ONE") &&
                strcmp(ab, "D0") && strcmp(ab, "D1") && strcmp(ab, "D2"))
            {
                out.format("alpha_combiner_sockets.append(%s)\n", ab);
                ++a_combiner_idx;
            }
            else
            {
                a_soc_log[1] = a_soc_log[0];
                a_soc_log[0] = ab;
            }
        }
        else
        {
            AddAlphaCombiner(out, Combiner::Mult, ac, ab, nullptr);
            ++a_combiner_idx;
        }
    }

    if (!(a_tev_opts & 4))
    {
        /* A+B node */
        AddAlphaCombiner(out, Combiner::Add, a_soc_log[0], a_soc_log[1], nullptr);
        ++a_combiner_idx;
    }

    if (!(a_tev_opts & 8))
    {
        /* +D node */
        AddAlphaCombiner(out, Combiner::Add, ad, a_soc_log[0], nullptr);
        ++a_combiner_idx;
    }

    /* Update TEV regs */
    if (c_tev_opts == 0xf)
    {
        if (stage.colorInD() != GX::CC_ZERO)
            strncpy(c_regs[stage.colorOpOutReg()], cd, 64);
        else if (stage.colorInA() != GX::CC_ZERO)
            strncpy(c_regs[stage.colorOpOutReg()], ca, 64);
    }
    else
        snprintf(c_regs[stage.colorOpOutReg()], 64, "color_combiner_sockets[%u]", c_combiner_idx - 1);
    if (a_tev_opts == 0xf)
    {
        if (stage.alphaInD() != GX::CA_ZERO)
            strncpy(a_regs[stage.alphaOpOutReg()], ad, 64);
        else if (stage.alphaInA() != GX::CA_ZERO)
            strncpy(a_regs[stage.alphaOpOutReg()], aa, 64);
    }
    else
        snprintf(a_regs[stage.alphaOpOutReg()], 64, "alpha_combiner_sockets[%u]", a_combiner_idx - 1);

    /* Row Break in gridder */
    out << "gridder.row_break(2)\n";

}

template <class MAT>
void _ConstructMaterial(Stream& out,
                        const MAT& material,
                        unsigned groupIdx,
                        unsigned matIdx)
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
               "gridder = hecl.Nodegrid(new_nodetree)\n"
               "gridder.place_node(final_node, 3)\n"
               "gridder.place_node(material_node, 0)\n"
               "material_node.material = new_material\n"
               "\n"
               "texture_nodes = []\n"
               "kcolor_nodes = []\n"
               "color_combiner_sockets = []\n"
               "alpha_combiner_sockets = []\n"
               "tex_links = []\n"
               "tev_reg_sockets = [None]*4\n"
               "\n", groupIdx, matIdx);

    /* Material Flags */
    out.format("new_material.retro_depth_sort = %s\n"
               "new_material.retro_alpha_test = %s\n"
               "new_material.retro_samus_reflection = %s\n"
               "new_material.retro_depth_write = %s\n"
               "new_material.retro_samus_reflection_persp = %s\n"
               "new_material.retro_shadow_occluder = %s\n"
               "new_material.retro_samus_reflection_indirect = %s\n"
               "new_material.retro_lightmapped = %s\n"
               "new_material.game_settings.invisible = %s\n",
               material.flags.depthSorting() ? "True" : "False",
               material.flags.alphaTest() ? "True" : "False",
               material.flags.samusReflection() ? "True" : "False",
               material.flags.depthWrite() ? "True" : "False",
               material.flags.samusReflectionSurfaceEye() ? "True" : "False",
               material.flags.shadowOccluderMesh() ? "True" : "False",
               material.flags.samusReflectionIndirectTexture() ? "True" : "False",
               material.flags.lightmap() ? "True" : "False",
               material.flags.shadowOccluderMesh() ? "True" : "False");


    /* Texture Indices */
    out << "tex_maps = []\n";
    for (atUint32 idx : material.textureIdxs)
        out.format("tex_maps.append(texmap_list[%u])\n", idx);

    /* KColor entries */
    if (material.flags.konstValuesEnabled())
    {
        unsigned i=0;
        for (const GX::Color& col : material.konstColors)
            Material::AddKcolor(out, col, i++);
    }

    /* Blend factors */
    using BlendFactor = Material::BlendFactor;
    if (material.blendDstFac != BlendFactor::BL_ZERO)
    {
        if (material.blendDstFac == BlendFactor::BL_ONE)
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
    for (const Material::ColorChannel& chan : material.colorChannels)
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
            const Material::TexCoordGen& tcg = material.tcgs[material.tevStageTexInfo[i].tcgSlot];
            GX::TexMtx mtx = tcg.mtx();
            int mtxIdx = -1;
            if (mtx >= GX::TEXMTX0 && mtx <= GX::TEXMTX9)
                mtxIdx = (mtx - GX::TEXMTX0) / 3;
            Material::AddTexture(out, tcg.source(), mtxIdx, material.tevStageTexInfo[i].texSlot);
            addedTcgs |= 1 << material.tevStageTexInfo[i].tcgSlot;
        }
    }

    /* Indirect texture node */
    if (material.flags.samusReflectionIndirectTexture())
    {
        Material::AddTexture(out, GX::TexGenSrc::TG_POS, -1, material.indTexSlot[0]);
        out << "# Indirect Texture\n"
               "ind_out = new_nodetree.nodes.new('ShaderNodeOutput')\n"
               "gridder.place_node(ind_out, 3)\n"
               "ind_out.name = 'IndirectOutput'\n"
               "ind_out.label = 'Indirect'\n"
               "new_nodetree.links.new(tex_node.outputs['Color'], ind_out.inputs['Color'])\n";
    }

    /* TEV-emulation combiner-node index context */
    unsigned c_combiner_idx = 0;
    unsigned a_combiner_idx = 0;

    /* Initialze TEV register sockets */
    char c_regs[4][64] = {"ONE", "D0", "D1", "D2"};
    char a_regs[4][64] = {"ONE", "D0", "D1", "D2"};

    /* Has Lightmap? */
    if (material.flags.lightmap())
    {
        if (material.tevStageTexInfo[0].texSlot != 0xff)
            out << "new_material.hecl_lightmap = tex_maps[0].name\n"
                   "tex_maps[0].image.use_fake_user = True\n";
    }

    /* Add TEV stages */
    for (i=0 ; i<material.tevStageCount ; ++i)
    {
        const Material::TEVStage& stage = material.tevStages[i];
        const Material::TEVStageTexInfo& stage_tex = material.tevStageTexInfo[i];
        AddTEVStage(out, stage, stage_tex, c_regs, a_regs, c_combiner_idx, a_combiner_idx);
    }

    /* Connect final prev register */
    if (!strcmp(c_regs[GX::TEVPREV], "ONE"))
        out << "final_node.inputs['Color'].default_value = (1.0,1.0,1.0,1.0)\n";
    else
        out.format("new_nodetree.links.new(%s, final_node.inputs['Color'])\n", c_regs[GX::TEVPREV]);

    if (!strcmp(a_regs[GX::TEVPREV], "ONE"))
        out << "final_node.inputs['Alpha'].default_value = 1.0\n";
    else
        out.format("new_nodetree.links.new(%s, final_node.inputs['Alpha'])\n", a_regs[GX::TEVPREV]);

    /* Texmtx Animation Section */
    i=0;
    for (const Material::UVAnimation& anim : material.uvAnims)
        Material::AddTextureAnim(out, anim.mode, i++, anim.vals);
}

void MaterialSet::ConstructMaterial(Stream& out,
                                    const MaterialSet::Material& material,
                                    unsigned groupIdx,
                                    unsigned matIdx)
{_ConstructMaterial(out, material, groupIdx, matIdx);}

MaterialSet::Material::Material(const hecl::Backend::GX& gx,
                                const std::unordered_map<std::string, int32_t>& iprops,
                                const std::vector<hecl::ProjectPath>& texPathsIn,
                                std::vector<hecl::ProjectPath>& texPathsOut,
                                int colorCount,
                                bool lightmapUVs,
                                bool matrixSkinning)
{
    XXH32_state_t xxHash;
    XXH32_reset(&xxHash, 0);

    if (gx.m_kcolorCount)
    {
        flags.setKonstValuesEnabled(true);
        konstCount.push_back(gx.m_kcolorCount);
    }

    auto search = iprops.find("retro_depth_sort");
    if (search != iprops.end())
        flags.setDepthSorting(search->second != 0);

    search = iprops.find("retro_alpha_test");
    if (search != iprops.end())
        flags.setPunchthroughAlpha(search->second != 0);

    search = iprops.find("retro_samus_reflection");
    if (search != iprops.end())
        flags.setSamusReflection(search->second != 0);

    search = iprops.find("retro_depth_write");
    if (search != iprops.end())
        flags.setDepthWrite(search->second != 0);

    search = iprops.find("retro_samus_reflection_persp");
    if (search != iprops.end())
        flags.setSamusReflectionSurfaceEye(search->second != 0);

    search = iprops.find("retro_shadow_occluder");
    if (search != iprops.end())
        flags.setShadowOccluderMesh(search->second != 0);

    search = iprops.find("retro_samus_reflection_indirect");
    if (search != iprops.end())
        flags.setSamusReflectionIndirectTexture(search->second != 0);

    search = iprops.find("retro_lightmapped");
    if (search != iprops.end())
        flags.setLightmap(search->second != 0);

    flags.setLightmapUVArray(lightmapUVs);

    atUint16 texFlags = 0;
    atUint16 tcgFlags = 0;
    tevStageTexInfo.reserve(gx.m_tevCount);
    textureIdxs.reserve(gx.m_tevCount);
    for (unsigned i=0 ; i<gx.m_tevCount ; ++i)
    {
        const hecl::Backend::GX::TEVStage& stage = gx.m_tevs[i];
        tevStageTexInfo.emplace_back();
        TEVStageTexInfo& texInfo = tevStageTexInfo.back();
        if (stage.m_texGenIdx != -1)
        {
            texInfo.tcgSlot = stage.m_texGenIdx;
            const hecl::Backend::GX::TexCoordGen& tcg = gx.m_tcgs[stage.m_texGenIdx];
            if (tcg.m_src >= hecl::Backend::GX::TG_TEX0 && tcg.m_src <= hecl::Backend::GX::TG_TEX6)
                tcgFlags |= 1 << (tcg.m_src - hecl::Backend::GX::TG_TEX0);
        }
        if (stage.m_texMapIdx != -1)
        {
            texInfo.texSlot = textureIdxs.size();
            const hecl::ProjectPath& texPath = texPathsIn.at(stage.m_texMapIdx);
            texFlags |= 1 << i;
            ++textureCount;
            bool found = false;
            for (size_t t=0 ; t<texPathsOut.size() ; ++t)
            {
                if (texPath == texPathsOut[t])
                {
                    found = true;
                    textureIdxs.push_back(t);
                    break;
                }
            }
            if (!found)
            {
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

    if (matrixSkinning)
    {
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
    for (unsigned i=0 ; i<gx.m_kcolorCount ; ++i)
    {
        konstColors.emplace_back(gx.m_kcolors[i]);
        XXH32_update(&xxHash, &gx.m_kcolors[i].num, sizeof(gx.m_kcolors[i].num));
    }

    blendDstFac = BlendFactor(gx.m_blendDst);
    XXH32_update(&xxHash, &gx.m_blendDst, sizeof(gx.m_blendDst));
    blendSrcFac = BlendFactor(gx.m_blendSrc);
    XXH32_update(&xxHash, &gx.m_blendSrc, sizeof(gx.m_blendSrc));
    if (flags.samusReflectionIndirectTexture())
    {
        indTexSlot.push_back(textureIdxs.size());
        XXH32_update(&xxHash, &indTexSlot.back(), sizeof(indTexSlot.back()));
    }

    colorChannelCount = 1;
    XXH32_update(&xxHash, &colorChannelCount, sizeof(colorChannelCount));
    colorChannels.emplace_back();
    ColorChannel& ch = colorChannels.back();
    for (unsigned i=0 ; i<gx.m_tevCount ; ++i)
    {
        const hecl::Backend::GX::TEVStage& stage = gx.m_tevs[i];
        for (int c=0 ; c<4 ; ++c)
            if (stage.m_color[c] == hecl::Backend::GX::CC_RASC ||
                stage.m_color[c] == hecl::Backend::GX::CC_RASA ||
                stage.m_alpha[c] == hecl::Backend::GX::CA_RASA)
            {
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
    for (unsigned i=0 ; i<gx.m_tevCount ; ++i)
    {
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
        for (int c=0 ; c<4 ; ++c)
            if (stage.m_color[c] == hecl::Backend::GX::CC_RASC ||
                stage.m_color[c] == hecl::Backend::GX::CC_RASA ||
                stage.m_alpha[c] == hecl::Backend::GX::CA_RASA)
            {
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
    for (unsigned i=0 ; i<gx.m_tcgCount ; ++i)
    {
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
    for (; uvAnimsCount<8 ;)
    {
        bool found = false;
        for (unsigned t=0 ; t<gx.m_tcgCount ; ++t)
        {
            const hecl::Backend::GX::TexCoordGen& tcg = gx.m_tcgs[t];
            if (tcg.m_mtx == GX::IDENTITY)
                continue;
            if ((tcg.m_mtx - GX::TEXMTX0) / 3 == uvAnimsCount)
            {
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

    uniqueIdx = XXH32_digest(&xxHash);
}

HMDLMaterialSet::Material::Material(hecl::Frontend::Frontend& FE,
                                    const std::string& diagName,
                                    const hecl::blender::Material& mat,
                                    const std::unordered_map<std::string, int32_t>& iprops,
                                    const std::vector<hecl::ProjectPath>& texPaths)
{
    auto search = iprops.find("retro_depth_sort");
    if (search != iprops.end())
        flags.setDepthSorting(search->second != 0);

    search = iprops.find("retro_alpha_test");
    if (search != iprops.end())
        flags.setPunchthroughAlpha(search->second != 0);

    search = iprops.find("retro_samus_reflection");
    if (search != iprops.end())
        flags.setSamusReflection(search->second != 0);

    search = iprops.find("retro_depth_write");
    if (search != iprops.end())
        flags.setDepthWrite(search->second != 0);

    search = iprops.find("retro_samus_reflection_persp");
    if (search != iprops.end())
        flags.setSamusReflectionSurfaceEye(search->second != 0);

    search = iprops.find("retro_shadow_occluder");
    if (search != iprops.end())
        flags.setShadowOccluderMesh(search->second != 0);

    search = iprops.find("retro_samus_reflection_indirect");
    if (search != iprops.end())
        flags.setSamusReflectionIndirectTexture(search->second != 0);

    search = iprops.find("retro_lightmapped");
    if (search != iprops.end())
        flags.setLightmap(search->second != 0);

    for (const hecl::ProjectPath& path : mat.texs)
    {
        size_t idx = 0;
        for (const hecl::ProjectPath& tPath : texPaths)
        {
            if (path == tPath)
            {
                textureIdxs.push_back(idx);
                ++textureCount;
                break;
            }
            ++idx;
        }
    }

    if (flags.samusReflectionIndirectTexture())
        indTexSlot.push_back(textureIdxs.size());

    heclSource = mat.source;
    heclIr = FE.compileSource(mat.source, diagName);

    uvAnimsSize = 4;
    uvAnimsCount = 0;
    for (const hecl::Frontend::IR::Instruction& inst : heclIr.m_instructions)
    {
        if (inst.m_op != hecl::Frontend::IR::OpType::Call)
            continue;
        if (inst.m_call.m_name.compare("Texture"))
            continue;

        const hecl::Frontend::IR::Instruction& sourceInst = inst.getChildInst(heclIr, 1);
        if (sourceInst.m_op != hecl::Frontend::IR::OpType::Call)
            continue;
        if (sourceInst.m_call.m_name.compare(0, 11, "RetroUVMode"))
            continue;

        std::vector<atVec4f> gameArgs;
        gameArgs.reserve(sourceInst.getChildCount() - 1);
        for (int i=1 ; i<sourceInst.getChildCount() ; ++i)
        {
            const hecl::Frontend::IR::Instruction& ci = sourceInst.getChildInst(heclIr, i);
            gameArgs.push_back(ci.getImmVec());
        }

        ++uvAnimsCount;
        uvAnims.emplace_back(sourceInst.m_call.m_name, gameArgs);
        size_t tmpUvAnimsSize = uvAnimsSize;
        uvAnims.back().binarySize(tmpUvAnimsSize);
        uvAnimsSize = tmpUvAnimsSize;
    }
}

MaterialSet::Material::UVAnimation::UVAnimation(const std::string& gameFunction,
                                                const std::vector<atVec4f>& gameArgs)
{
    if (!gameFunction.compare("RetroUVMode0NodeN"))
        mode = Mode::MvInvNoTranslation;
    else if (!gameFunction.compare("RetroUVMode1NodeN"))
        mode = Mode::MvInv;
    else if (!gameFunction.compare("RetroUVMode2Node"))
    {
        mode = Mode::Scroll;
        if (gameArgs.size() < 2)
            Log.report(logvisor::Fatal, "Mode2 UV anim requires 2 vector arguments");
        vals[0] = gameArgs[0].simd[0];
        vals[1] = gameArgs[0].simd[1];
        vals[2] = gameArgs[1].simd[0];
        vals[3] = gameArgs[1].simd[1];
    }
    else if (!gameFunction.compare("RetroUVMode3Node"))
    {
        mode = Mode::Rotation;
        if (gameArgs.size() < 2)
            Log.report(logvisor::Fatal, "Mode3 UV anim requires 2 arguments");
        vals[0] = gameArgs[0].simd[0];
        vals[1] = gameArgs[1].simd[0];
    }
    else if (!gameFunction.compare("RetroUVMode4Node"))
    {
        mode = Mode::HStrip;
        if (gameArgs.size() < 4)
            Log.report(logvisor::Fatal, "Mode4 UV anim requires 4 arguments");
        vals[0] = gameArgs[0].simd[0];
        vals[1] = gameArgs[1].simd[0];
        vals[2] = gameArgs[2].simd[0];
        vals[3] = gameArgs[3].simd[0];
    }
    else if (!gameFunction.compare("RetroUVMode5Node"))
    {
        mode = Mode::VStrip;
        if (gameArgs.size() < 4)
            Log.report(logvisor::Fatal, "Mode5 UV anim requires 4 arguments");
        vals[0] = gameArgs[0].simd[0];
        vals[1] = gameArgs[1].simd[0];
        vals[2] = gameArgs[2].simd[0];
        vals[3] = gameArgs[3].simd[0];
    }
    else if (!gameFunction.compare("RetroUVMode6NodeN"))
        mode = Mode::Model;
    else if (!gameFunction.compare("RetroUVMode7NodeN"))
    {
        mode = Mode::CylinderEnvironment;
        if (gameArgs.size() < 2)
            Log.report(logvisor::Fatal, "Mode7 UV anim requires 2 arguments");
        vals[0] = gameArgs[0].simd[0];
        vals[1] = gameArgs[1].simd[0];
    }
    else
        Log.report(logvisor::Fatal, "unsupported UV anim '%s'", gameFunction.c_str());
}

template <class Op>
void MaterialSet::Material::UVAnimation::Enumerate(typename Op::StreamT& s)
{
    Do<Op>({}, mode, s);
    switch (mode)
    {
    case Mode::MvInvNoTranslation:
    case Mode::MvInv:
    case Mode::Model:
        break;
    case Mode::Scroll:
    case Mode::HStrip:
    case Mode::VStrip:
        for (int i=0 ; i<4 ; ++i)
            Do<Op>({}, vals[i], s);
        break;
    case Mode::Rotation:
    case Mode::CylinderEnvironment:
        for (int i=0 ; i<2 ; ++i)
            Do<Op>({}, vals[i], s);
        break;
    case Mode::Eight:
        for (int i=0 ; i<9 ; ++i)
            Do<Op>({}, vals[i], s);
        break;
    }
}

AT_SPECIALIZE_DNA(MaterialSet::Material::UVAnimation)

}

namespace DataSpec::DNAMP2
{

void MaterialSet::ConstructMaterial(Stream& out,
                                    const MaterialSet::Material& material,
                                    unsigned groupIdx,
                                    unsigned matIdx)
{DataSpec::DNAMP1::_ConstructMaterial(out, material, groupIdx, matIdx);}

}
