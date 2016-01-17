#include "CMDLMaterials.hpp"

using Stream = HECL::BlenderConnection::PyOutStream;

namespace Retro
{
namespace DNAMP3
{
using Material = MaterialSet::Material;

void MaterialSet::RegisterMaterialProps(Stream& out)
{
    out << "bpy.types.Material.retro_punchthrough_alpha = bpy.props.BoolProperty(name='Retro: Punchthrough Alpha')\n"
           "bpy.types.Material.retro_shadow_occluder = bpy.props.BoolProperty(name='Retro: Shadow Occluder')\n"
           "bpy.types.Material.retro_lightmapped = bpy.props.BoolProperty(name='Retro: Lightmapped')\n"
           "bpy.types.Material.retro_opac = bpy.props.IntProperty(name='Retro: OPAC')\n"
           "bpy.types.Material.retro_blod = bpy.props.IntProperty(name='Retro: BLOD')\n"
           "bpy.types.Material.retro_bloi = bpy.props.IntProperty(name='Retro: BLOI')\n"
           "bpy.types.Material.retro_bnif = bpy.props.IntProperty(name='Retro: BNIF')\n"
           "bpy.types.Material.retro_xrbr = bpy.props.IntProperty(name='Retro: XRBR')\n"
           "\n";
}

void MaterialSet::ConstructMaterial(Stream& out,
                                    const PAKRouter<PAKBridge>& pakRouter,
                                    const PAK::Entry& entry,
                                    const Material& material,
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
               "color_combiner_nodes = []\n"
               "alpha_combiner_nodes = []\n"
               "tex_links = []\n"
               "tev_reg_sockets = [None]*4\n"
               "\n", groupIdx, matIdx);

    /* Material Flags */
    out.format("new_material.retro_punchthrough_alpha = %s\n"
               "new_material.retro_shadow_occluder = %s\n"
               "new_material.game_settings.invisible = %s\n",
               material.header.flags.punchthroughAlpha() ? "True" : "False",
               material.header.flags.shadowOccluderMesh() ? "True" : "False",
               material.header.flags.shadowOccluderMesh() ? "True" : "False");


    /* TODO: Some models enable both of these flags at once, why?
     * And how do we handle this properly?
     */
    /* Blend factors */
    if (material.header.flags.additiveBlending())
        out << "new_material.game_settings.alpha_blend = 'ADD'\n"
               "new_material.use_transparency = True\n"
               "new_material.transparency_method = 'RAYTRACE'\n"
               "new_material.alpha = 1.0\n";
    else if (material.header.flags.alphaBlending())
        out << "new_material.game_settings.alpha_blend = 'ALPHA'\n"
               "new_material.use_transparency = True\n"
               "new_material.transparency_method = 'RAYTRACE'\n"
               "new_material.alpha = 1.0\n";

    /* Texmap list */
    out << "tex_maps = []\n"
           "pnode = None\n"
           "rflv_tex_node = None\n";

    /* Add PASSes */
    i=0;
    unsigned texMapIdx = 0;
    unsigned texMtxIdx = 0;
    unsigned kColorIdx = 0;
    Material::ISection* prevSection = nullptr;
    for (const Material::SectionFactory& factory : material.sections)
    {
        factory.section->constructNode(out, pakRouter, entry, prevSection, i++, texMapIdx, texMtxIdx, kColorIdx);
        Material::SectionPASS* pass = dynamic_cast<Material::SectionPASS*>(factory.section.get());
        if (!pass || (pass && Material::SectionPASS::Subtype(pass->subtype.toUint32()) != Material::SectionPASS::Subtype::RFLV))
            prevSection = factory.section.get();
    }

    /* Connect final PASS */
    out << "if pnode:\n"
           "    new_nodetree.links.new(pnode.outputs['Next Color'], final_node.inputs['Color'])\n"
           "    new_nodetree.links.new(pnode.outputs['Next Alpha'], final_node.inputs['Alpha'])\n"
           "else:\n"
           "    new_nodetree.links.new(kcolor_nodes[-1][0].outputs[0], final_node.inputs['Color'])\n"
           "    new_nodetree.links.new(kcolor_nodes[-1][1].outputs[0], final_node.inputs['Alpha'])\n";
}

void Material::SectionPASS::constructNode(HECL::BlenderConnection::PyOutStream& out,
                                          const PAKRouter<PAKBridge>& pakRouter,
                                          const PAK::Entry& entry,
                                          const Material::ISection* prevSection,
                                          unsigned idx,
                                          unsigned& texMapIdx,
                                          unsigned& texMtxIdx,
                                          unsigned& kColorIdx) const
{
    /* Add Texture nodes */
    if (txtrId)
    {
        std::string texName = pakRouter.getBestEntryName(txtrId);
        const NOD::Node* node;
        const PAK::Entry* texEntry = pakRouter.lookupEntry(txtrId, &node);
        HECL::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
        if (txtrPath.getPathType() == HECL::ProjectPath::Type::None)
        {
            PAKEntryReadStream rs = texEntry->beginReadStream(*node);
            TXTR::Extract(rs, txtrPath);
        }
        HECL::SystemString resPath = pakRouter.getResourceRelativePath(entry, txtrId);
        HECL::SystemUTF8View resPathView(resPath);
        out.format("if '%s' in bpy.data.textures:\n"
                   "    image = bpy.data.images['%s']\n"
                   "    texture = bpy.data.textures[image.name]\n"
                   "else:\n"
                   "    image = bpy.data.images.load('''//%s''')\n"
                   "    image.name = '%s'\n"
                   "    texture = bpy.data.textures.new(image.name, 'IMAGE')\n"
                   "    texture.image = image\n"
                   "tex_maps.append(texture)\n"
                   "\n", texName.c_str(), texName.c_str(),
                   resPathView.str().c_str(), texName.c_str());
        if (uvAnim.size())
        {
            const UVAnimation& uva = uvAnim[0];
            DNAMP1::MaterialSet::Material::AddTexture(out, GX::TexGenSrc(uva.unk1 + (uva.unk1 < 2 ? 0 : 2)), texMtxIdx, texMapIdx++);
            DNAMP1::MaterialSet::Material::AddTextureAnim(out, uva.anim.mode, texMtxIdx++, uva.anim.vals);
        }
        else
            DNAMP1::MaterialSet::Material::AddTexture(out, GX::TexGenSrc(uvSrc + 4), -1, texMapIdx++);
    }

    /* Special case for RFLV (environment UV mask) */
    if (Subtype(subtype.toUint32()) == Subtype::RFLV)
    {
        if (txtrId)
            out << "rflv_tex_node = texture_nodes[-1]\n";
        return;
    }

    /* Add PASS node */
    bool linkRAS = false;
    out << "prev_pnode = pnode\n"
           "pnode = new_nodetree.nodes.new('ShaderNodeGroup')\n";
    switch (Subtype(subtype.toUint32()))
    {
    case Subtype::DIFF:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassDIFF']\n";
        linkRAS = true;
        break;
    case Subtype::RIML:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassRIML']\n";
        if (idx == 0)
            linkRAS = true;
        break;
    case Subtype::BLOL:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassBLOL']\n";
        if (idx == 0)
            linkRAS = true;
        break;
    case Subtype::BLOD:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassBLOD']\n";
        if (idx == 0)
            linkRAS = true;
        break;
    case Subtype::CLR:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassCLR']\n";
        if (idx == 0)
            linkRAS = true;
        break;
    case Subtype::TRAN:
        if (flags.TRANInvert())
            out << "pnode.node_tree = bpy.data.node_groups['RetroPassTRANInv']\n";
        else
            out << "pnode.node_tree = bpy.data.node_groups['RetroPassTRAN']\n";
        break;
    case Subtype::INCA:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassINCA']\n";
        break;
    case Subtype::RFLV:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassRFLV']\n";
        break;
    case Subtype::RFLD:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassRFLD']\n"
               "if rflv_tex_node:\n"
               "    new_nodetree.links.new(rflv_tex_node.outputs['Color'], pnode.inputs['Mask Color'])\n"
               "    new_nodetree.links.new(rflv_tex_node.outputs['Value'], pnode.inputs['Mask Alpha'])\n";
        break;
    case Subtype::LRLD:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassLRLD']\n";
        break;
    case Subtype::LURD:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassLURD']\n";
        break;
    case Subtype::BLOI:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassBLOI']\n";
        break;
    case Subtype::XRAY:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassXRAY']\n";
        break;
    case Subtype::TOON:
        out << "pnode.node_tree = bpy.data.node_groups['RetroPassTOON']\n";
        break;
    default: break;
    }
    out << "gridder.place_node(pnode, 2)\n";

    if (txtrId)
    {
        out << "new_nodetree.links.new(texture_nodes[-1].outputs['Color'], pnode.inputs['Tex Color'])\n"
               "new_nodetree.links.new(texture_nodes[-1].outputs['Value'], pnode.inputs['Tex Alpha'])\n";
    }

    if (linkRAS)
        out << "new_nodetree.links.new(material_node.outputs['Color'], pnode.inputs['Prev Color'])\n"
               "new_nodetree.links.new(material_node.outputs['Alpha'], pnode.inputs['Prev Alpha'])\n";
    else if (prevSection)
    {
        if (prevSection->m_type == ISection::Type::PASS &&
            Subtype(static_cast<const SectionPASS*>(prevSection)->subtype.toUint32()) != Subtype::RFLV)
            out << "new_nodetree.links.new(prev_pnode.outputs['Next Color'], pnode.inputs['Prev Color'])\n"
                   "new_nodetree.links.new(prev_pnode.outputs['Next Alpha'], pnode.inputs['Prev Alpha'])\n";
        else if (prevSection->m_type == ISection::Type::CLR)
            out << "new_nodetree.links.new(kcolor_nodes[-1][0].outputs[0], pnode.inputs['Prev Color'])\n"
                   "new_nodetree.links.new(kcolor_nodes[-1][1].outputs[0], pnode.inputs['Prev Alpha'])\n";
    }

    /* Row Break in gridder */
    out << "gridder.row_break(2)\n";
}

void Material::SectionCLR::constructNode(HECL::BlenderConnection::PyOutStream& out,
                                         const PAKRouter<PAKBridge>& pakRouter,
                                         const PAK::Entry& entry,
                                         const Material::ISection* prevSection,
                                         unsigned idx,
                                         unsigned& texMapIdx,
                                         unsigned& texMtxIdx,
                                         unsigned& kColorIdx) const
{
    DNAMP1::MaterialSet::Material::AddKcolor(out, color, kColorIdx++);
    switch (Subtype(subtype.toUint32()))
    {
    case Subtype::DIFB:
        out << "kc_node.label += ' DIFB'\n"
               "ka_node.label += ' DIFB'\n";
        break;
    default: break;
    }
}

void Material::SectionINT::constructNode(HECL::BlenderConnection::PyOutStream& out,
                                         const PAKRouter<PAKBridge>& pakRouter,
                                         const PAK::Entry& entry,
                                         const Material::ISection* prevSection,
                                         unsigned idx,
                                         unsigned& texMapIdx,
                                         unsigned& texMtxIdx,
                                         unsigned& kColorIdx) const
{
    switch (Subtype(subtype.toUint32()))
    {
    case Subtype::OPAC:
        out.format("new_material.retro_opac = %d\n", value);
        break;
    case Subtype::BLOD:
        out.format("new_material.retro_blod = %d\n", value);
        break;
    case Subtype::BLOI:
        out.format("new_material.retro_bloi = %d\n", value);
        break;
    case Subtype::BNIF:
        out.format("new_material.retro_bnif = %d\n", value);
        break;
    case Subtype::XRBR:
        out.format("new_material.retro_xrbr = %d\n", value);
        break;
    default: break;
    }
}

}
}
