#include "FRME.hpp"
#include "../DNACommon/TXTR.hpp"

namespace DataSpec
{
namespace DNAMP1
{
void FRME::read(athena::io::IStreamReader& __dna_reader)
{
    /* version */
    version = __dna_reader.readUint32Big();
    /* unk1 */
    unk1 = __dna_reader.readUint32Big();
    /* modelCount */
    modelCount = __dna_reader.readUint32Big();
    /* unk3 */
    unk3 = __dna_reader.readUint32Big();
    /* widgetCount */
    widgetCount = __dna_reader.readUint32Big();
    /* widgets */
    __dna_reader.enumerate<Widget>(widgets, widgetCount, [this](athena::io::IStreamReader& reader, Widget& w) {
        w.owner = this;
        w.read(reader);
    });
}

void FRME::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* version */
    __dna_writer.writeUint32Big(version);
    /* unk1 */
    __dna_writer.writeUint32Big(unk1);
    /* modelCount */
    __dna_writer.writeUint32Big(modelCount);
    /* unk3 */
    __dna_writer.writeUint32Big(unk3);
    /* widgetCount */
    __dna_writer.writeUint32Big(widgetCount);
    /* widgets */
    __dna_writer.enumerate(widgets);
}

size_t FRME::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, widgets);
    return __isz + 20;
}

void FRME::Widget::read(athena::io::IStreamReader& __dna_reader)
{
    /* type */
    type.read(__dna_reader);
    /* header */
    header.read(__dna_reader);
    switch(type)
    {
    case SBIG('BWIG'): widgetInfo.reset(new BWIGInfo); break;
    case SBIG('HWIG'): widgetInfo.reset(new HWIGInfo); break;
    case SBIG('CAMR'): widgetInfo.reset(new CAMRInfo); break;
    case SBIG('LITE'): widgetInfo.reset(new LITEInfo); break;
    case SBIG('ENRG'): widgetInfo.reset(new ENRGInfo); break;
    case SBIG('MODL'): widgetInfo.reset(new MODLInfo); break;
    case SBIG('METR'): widgetInfo.reset(new METRInfo); break;
    case SBIG('GRUP'): widgetInfo.reset(new GRUPInfo); break;
    case SBIG('PANE'): widgetInfo.reset(new PANEInfo); break;
    case SBIG('TXPN'): widgetInfo.reset(new TXPNInfo(owner->version)); break;
    case SBIG('IMGP'): widgetInfo.reset(new IMGPInfo); break;
    case SBIG('TBGP'): widgetInfo.reset(new TBGPInfo); break;
    case SBIG('SLGP'): widgetInfo.reset(new SLGPInfo); break;
    default:
        Log.report(logvisor::Fatal, _S("Unsupported FRME widget type %.8X"), type.toUint32());
    }

    /* widgetInfo */
    widgetInfo->read(__dna_reader);

    /* isWorker */
    isWorker = __dna_reader.readBool();
    if (isWorker)
    {
        /* workerId */
        workerId = __dna_reader.readUint16Big();
    }
    /* origin */
    origin = __dna_reader.readVec3fBig();
    /* basis[0] */
    basis[0] = __dna_reader.readVec3fBig();
    /* basis[1] */
    basis[1] = __dna_reader.readVec3fBig();
    /* basis[2] */
    basis[2] = __dna_reader.readVec3fBig();
    /* rotationCenter */
    rotationCenter = __dna_reader.readVec3fBig();
    /* unk1 */
    unk1 = __dna_reader.readInt32Big();
    /* unk2 */
    unk2 = __dna_reader.readInt16Big();
}

void FRME::Widget::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* type */
    DNAFourCC _type = widgetInfo ? widgetInfo->fourcc() : FOURCC('BWIG');
    _type.write(__dna_writer);
    /* header */
    header.write(__dna_writer);

    /* widgetInfo */
    if (widgetInfo)
        widgetInfo->write(__dna_writer);

    /* isWorker */
    __dna_writer.writeBool(isWorker);
    if (isWorker)
    {
        /* workerId */
        __dna_writer.writeUint16Big(workerId);
    }
    /* origin */
    __dna_writer.writeVec3fBig(origin);
    /* basis[0] */
    __dna_writer.writeVec3fBig(basis[0]);
    /* basis[1] */
    __dna_writer.writeVec3fBig(basis[1]);
    /* basis[2] */
    __dna_writer.writeVec3fBig(basis[2]);
    /* rotationCenter */
    __dna_writer.writeVec3fBig(rotationCenter);
    /* unk1 */
    __dna_writer.writeInt32Big(unk1);
    /* unk2 */
    __dna_writer.writeInt16Big(unk2);
}

size_t FRME::Widget::binarySize(size_t __isz) const
{
    __isz = type.binarySize(__isz);
    __isz = header.binarySize(__isz);
    if (widgetInfo)
        __isz = widgetInfo->binarySize(__isz);
    if (isWorker)
        __isz += 4;
    return __isz + 67;
}

void FRME::Widget::CAMRInfo::read(athena::io::IStreamReader& __dna_reader)
{
    projectionType = ProjectionType(__dna_reader.readUint32Big());
    if (projectionType == ProjectionType::Perspective)
        projection.reset(new PerspectiveProjection);
    else if (projectionType == ProjectionType::Orthographic)
        projection.reset(new OrthographicProjection);
    else
        Log.report(logvisor::Fatal, _S("Invalid CAMR projection mode! %i"), int(projectionType));

    projection->read(__dna_reader);
}

void FRME::Widget::CAMRInfo::write(athena::io::IStreamWriter& __dna_writer) const
{
    if (!projection)
        Log.report(logvisor::Fatal, _S("Invalid CAMR projection object!"));
    if (projection->type != projectionType)
        Log.report(logvisor::Fatal, _S("CAMR projection type does not match actual projection type!"));

    __dna_writer.writeUint32Big(atUint32(projectionType));
    projection->write(__dna_writer);
}

size_t FRME::Widget::CAMRInfo::binarySize(size_t __isz) const
{
    __isz = projection->binarySize(__isz);
    return __isz + 4;
}

void FRME::Widget::LITEInfo::read(athena::io::IStreamReader& __dna_reader)
{
    IWidgetInfo::read(__dna_reader);
    /* type */
    type = ELightType(__dna_reader.readUint32Big());
    /* distC */
    distC = __dna_reader.readFloatBig();
    /* distL */
    distL = __dna_reader.readFloatBig();
    /* distQ */
    distQ = __dna_reader.readFloatBig();
    /* angC */
    angC = __dna_reader.readFloatBig();
    /* angL */
    angL = __dna_reader.readFloatBig();
    /* angQ */
    angQ = __dna_reader.readFloatBig();
    /* loadedIdx */
    loadedIdx = __dna_reader.readUint32Big();

    /* cutoff */
    if (type == ELightType::Spot)
        cutoff = __dna_reader.readFloatBig();
}

void FRME::Widget::LITEInfo::write(athena::io::IStreamWriter& __dna_writer) const
{
    IWidgetInfo::write(__dna_writer);
    /* type */
    __dna_writer.writeUint32Big(atUint32(type));
    /* distC */
    __dna_writer.writeFloatBig(distC);
    /* distL */
    __dna_writer.writeFloatBig(distL);
    /* distQ */
    __dna_writer.writeFloatBig(distQ);
    /* angC */
    __dna_writer.writeFloatBig(angC);
    /* angL */
    __dna_writer.writeFloatBig(angL);
    /* angQ */
    __dna_writer.writeFloatBig(angQ);
    /* loadedIdx */
    __dna_writer.writeUint32Big(loadedIdx);

    /* cutoff */
    if (type == ELightType::Spot)
        __dna_writer.writeFloatBig(cutoff);
}

size_t FRME::Widget::LITEInfo::binarySize(size_t __isz) const
{
    __isz = IWidgetInfo::binarySize(__isz);
    return __isz + ((type == ELightType::Spot) ? 36 : 32);
}

void FRME::Widget::TXPNInfo::read(athena::io::IStreamReader& __dna_reader)
{
    IWidgetInfo::read(__dna_reader);
    /* xDim */
    xDim = __dna_reader.readFloatBig();
    /* zDim */
    zDim = __dna_reader.readFloatBig();
    /* scaleCenter */
    scaleCenter = __dna_reader.readVec3fBig();
    /* font */
    font.read(__dna_reader);
    /* unk1 */
    wordWrap = __dna_reader.readBool();
    /* unk2 */
    vertical = __dna_reader.readBool();
    /* justification */
    justification = Justification(__dna_reader.readUint32Big());
    /* verticalJustification */
    verticalJustification = VerticalJustification(__dna_reader.readUint32Big());
    /* fillColor */
    fillColor = __dna_reader.readVec4fBig();
    /* outlineColor */
    outlineColor = __dna_reader.readVec4fBig();
    /* pointScale */
    blockExtent = __dna_reader.readVec2fBig();
    if (version == 1)
    {
        /* jpnFont */
        jpnFont.read(__dna_reader);
        /* jpnPointScale[0] */
        jpnPointScale[0] = __dna_reader.readInt32Big();
        /* jpnPointScale[0] */
        jpnPointScale[1] = __dna_reader.readInt32Big();
    }
}

void FRME::Widget::TXPNInfo::write(athena::io::IStreamWriter& __dna_writer) const
{
    IWidgetInfo::write(__dna_writer);
    /* xDim */
    __dna_writer.writeFloatBig(xDim);
    /* zDim */
    __dna_writer.writeFloatBig(zDim);
    /* scaleCenter */
    __dna_writer.writeVec3fBig(scaleCenter);
    /* font */
    font.write(__dna_writer);
    /* unk1 */
    __dna_writer.writeBool(wordWrap);
    /* unk2 */
    __dna_writer.writeBool(vertical);
    /* justification */
    __dna_writer.writeUint32Big(atUint32(justification));
    /* verticalJustification */
    __dna_writer.writeUint32Big(atUint32(verticalJustification));
    /* fillColor */
    __dna_writer.writeVec4fBig(fillColor);
    /* outlineColor */
    __dna_writer.writeVec4fBig(outlineColor);
    /* pointScale */
    __dna_writer.writeVec2fBig(blockExtent);
    if (version == 1)
    {
        /* jpnFont */
        jpnFont.write(__dna_writer);
        /* jpnPointScale[0] */
        __dna_writer.writeInt32Big(jpnPointScale[0]);
        /* jpnPointScale[1] */
        __dna_writer.writeInt32Big(jpnPointScale[1]);
    }
}

size_t FRME::Widget::TXPNInfo::binarySize(size_t __isz) const
{
    __isz = IWidgetInfo::binarySize(__isz);
    __isz = font.binarySize(__isz);
    if (version == 1)
        __isz = jpnFont.binarySize(__isz);

    return __isz + (version == 1 ? 78 : 66);
}

bool FRME::Extract(const SpecBase &dataSpec,
                   PAKEntryReadStream &rs,
                   const hecl::ProjectPath &outPath,
                   PAKRouter<PAKBridge> &pakRouter,
                   const PAK::Entry &entry,
                   bool force,
                   hecl::BlenderToken& btok,
                   std::function<void (const hecl::SystemChar *)> fileChanged)
{
    FRME frme;
    frme.read(rs);

    hecl::BlenderConnection& conn = btok.getBlenderConnection();

#if 0
    if (!force && outPath.isFile())
        return true;
#endif

    if (!conn.createBlend(outPath, hecl::BlenderConnection::BlendType::Frame))
        return false;

    hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    os << "import bpy, math, bmesh\n"
          "from mathutils import Matrix, Quaternion\n"
          "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    if ob.type != 'CAMERA':\n"
          "        bpy.context.scene.objects.unlink(ob)\n"
          "        bpy.data.objects.remove(ob)\n"
          "\n"
          "def duplicateObject(copy_obj):\n"
          "    # Create new mesh\n"
          "    mesh = bpy.data.meshes.new(copy_obj.name)\n"
          "    # Create new object associated with the mesh\n"
          "    ob_new = bpy.data.objects.new(copy_obj.name, mesh)\n"
          "    # Copy data block from the old object into the new object\n"
          "    ob_new.data = copy_obj.data\n"
          "    ob_new.scale = copy_obj.scale\n"
          "    ob_new.location = copy_obj.location\n"
          "    # Link new object to the given scene and select it\n"
          "    bpy.context.scene.objects.link(ob_new)\n"
          "    return ob_new\n";

    os.format("bpy.context.scene.name = '%s'\n"
              "bpy.context.scene.render.resolution_x = 640\n"
              "bpy.context.scene.render.resolution_y = 480\n"
              "bpy.context.scene.render.engine = 'CYCLES'\n"
              "bpy.context.scene.world.use_nodes = True\n"
              "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
              "bg_node = bpy.context.scene.world.node_tree.nodes['Background']\n",
              pakRouter.getBestEntryName(entry).c_str());

    for (const FRME::Widget& w : frme.widgets)
    {
        os << "binding = None\n"
              "angle = Quaternion((0.0, 0.0, 0.0), 0)\n";
        if (w.type == SBIG('CAMR'))
        {
            using CAMRInfo = Widget::CAMRInfo;
            os.format("cam = bpy.data.cameras.new(name='%s')\n"
                      "binding = cam\n", w.header.name.c_str());
            if (CAMRInfo* info = static_cast<CAMRInfo*>(w.widgetInfo.get()))
            {
                if (info->projectionType == CAMRInfo::ProjectionType::Orthographic)
                {
                    CAMRInfo::OrthographicProjection* proj = static_cast<CAMRInfo::OrthographicProjection*>(info->projection.get());
                    os.format("cam.type = 'ORTHO'\n"
                              "cam.ortho_scale = %f\n"
                              "cam.clip_start = %f\n"
                              "cam.clip_end = %f\n",
                              std::fabs(proj->right - proj->left),
                              proj->znear, proj->zfar);
                }
                else if (info->projectionType == CAMRInfo::ProjectionType::Perspective)
                {
                    CAMRInfo::PerspectiveProjection* proj = static_cast<CAMRInfo::PerspectiveProjection*>(info->projection.get());
                    os.format("cam.type = 'PERSP'\n"
                              "cam.lens_unit = 'FOV'\n"
                              "cam.clip_start = %f\n"
                              "cam.clip_end = %f\n"
                              "bpy.context.scene.render.resolution_x = 480 * %f\n",
                              proj->znear, proj->zfar, proj->aspect);
                    if (proj->aspect > 1.f)
                        os.format("cam.angle = math.atan2(%f, 1.0 / math.tan(math.radians(%f / 2.0))) * 2.0\n",
                                  proj->aspect, proj->fov);
                    else
                        os.format("cam.angle = math.radians(%f)\n", proj->fov);
                }
            }
            os << "angle = Quaternion((1.0, 0.0, 0.0), math.radians(90.0))\n";
        }
        else if (w.type == SBIG('LITE'))
        {
            using LITEInfo = Widget::LITEInfo;
            if (LITEInfo* info = static_cast<LITEInfo*>(w.widgetInfo.get()))
            {
                switch(info->type)
                {
                case LITEInfo::ELightType::LocalAmbient:
                    os.format("bg_node.inputs[0].default_value = (%f,%f,%f,1.0)\n"
                              "bg_node.inputs[1].default_value = %f\n",
                              w.header.color.vec[0], w.header.color.vec[1], w.header.color.vec[2],
                              info->distQ / 8.0);
                    break;
                case LITEInfo::ELightType::Spot:
                case LITEInfo::ELightType::Directional:
                    os << "angle = Quaternion((1.0, 0.0, 0.0), math.radians(90.0))\n";
                default:
                    os.format("lamp = bpy.data.lamps.new(name='%s', type='POINT')\n"
                              "lamp.color = (%f, %f, %f)\n"
                              "lamp.falloff_type = 'INVERSE_COEFFICIENTS'\n"
                              "lamp.constant_coefficient = %f\n"
                              "lamp.linear_coefficient = %f\n"
                              "lamp.quadratic_coefficient = %f\n"
                              "lamp.retro_light_angle_constant = %f\n"
                              "lamp.retro_light_angle_linear = %f\n"
                              "lamp.retro_light_angle_quadratic = %f\n"
                              "lamp.retro_light_index = %d\n"
                              "binding = lamp\n",
                              w.header.name.c_str(),
                              w.header.color.vec[0], w.header.color.vec[1], w.header.color.vec[2],
                              info->distC, info->distL, info->distQ,
                              info->angC, info->angL, info->angQ, info->loadedIdx);
                    if (info->type == LITEInfo::ELightType::Spot)
                        os.format("lamp.type = 'SPOT'\n"
                                  "lamp.spot_size = %f\n",
                                  info->cutoff);
                    else if (info->type == LITEInfo::ELightType::Directional)
                        os << "lamp.type = 'HEMI'\n";
                }
            }
        }
        else if (w.type == SBIG('IMGP'))
        {
            using IMGPInfo = Widget::IMGPInfo;
            if (IMGPInfo* info = static_cast<IMGPInfo*>(w.widgetInfo.get()))
            {
                std::string texName;
                hecl::SystemString resPath;
                if (info->texture)
                {
                    texName = pakRouter.getBestEntryName(info->texture);
                    const nod::Node* node;
                    const PAKRouter<PAKBridge>::EntryType* texEntry = pakRouter.lookupEntry(info->texture, &node);
                    hecl::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
                    if (txtrPath.isNone())
                    {
                        txtrPath.makeDirChain(false);
                        PAKEntryReadStream rs = texEntry->beginReadStream(*node);
                        TXTR::Extract(rs, txtrPath);
                    }
                    resPath = pakRouter.getResourceRelativePath(entry, info->texture);
                }

                if (resPath.size())
                {
                    hecl::SystemUTF8View resPathView(resPath);
                    os.format("if '%s' in bpy.data.images:\n"
                              "    image = bpy.data.images['%s']\n"
                              "else:\n"
                              "    image = bpy.data.images.load('''//%s''')\n"
                              "    image.name = '%s'\n",
                              texName.c_str(), texName.c_str(),
                              resPathView.str().c_str(), texName.c_str());
                }
                else
                {
                    os << "image = None\n";
                }

                os.format("material = bpy.data.materials.new('%s')\n"
                          "material.specular_intensity = 0.0\n"
                          "tex_slot = material.texture_slots.add()\n"
                          "tex_slot.texture = bpy.data.textures.new('%s', 'IMAGE')\n"
                          "tex_slot.texture.image = image\n"
                          "material.active_texture = tex_slot.texture\n"
                          "bm = bmesh.new()\n"
                          "verts = []\n",
                          w.header.name.c_str(), w.header.name.c_str());

                for (int i=0 ; i<info->quadCoordCount ; ++i)
                {
                    int ti;
                    if (i == 2)
                        ti = 3;
                    else if (i == 3)
                        ti = 2;
                    else
                        ti = i;
                    os.format("verts.append(bm.verts.new((%f,%f,%f)))\n",
                              info->quadCoords[ti].vec[0],
                              info->quadCoords[ti].vec[1],
                              info->quadCoords[ti].vec[2]);
                }
                os << "bm.faces.new(verts)\n"
                      "bm.loops.layers.uv.new('UV')\n"
                      "bm.verts.ensure_lookup_table()\n";
                for (int i=0 ; i<info->uvCoordCount ; ++i)
                {
                    int ti;
                    if (i == 2)
                        ti = 3;
                    else if (i == 3)
                        ti = 2;
                    else
                        ti = i;
                    os.format("bm.verts[%d].link_loops[0][bm.loops.layers.uv[0]].uv = (%f,%f)\n", i,
                              info->uvCoords[ti].vec[0],
                              info->uvCoords[ti].vec[1]);
                }
                os.format("binding = bpy.data.meshes.new('%s')\n"
                          "bm.to_mesh(binding)\n"
                          "bm.free()\n"
                          "binding.materials.append(material)\n",
                          w.header.name.c_str());
            }
        }

        os.format("frme_obj = bpy.data.objects.new(name='%s', object_data=binding)\n"
                  "parentName = '%s'\n"
                  "frme_obj.retro_widget_type = 'RETRO_%s'\n"
                  "frme_obj.retro_widget_use_anim_controller = %s\n"
                  "frme_obj.retro_widget_default_visible = %s\n"
                  "frme_obj.retro_widget_default_active = %s\n"
                  "frme_obj.retro_widget_cull_faces = %s\n"
                  "frme_obj.retro_widget_color = (%f,%f,%f,%f)\n"
                  "frme_obj.retro_widget_model_draw_flags = bpy.types.Object.retro_widget_model_draw_flags[1]['items'][%i][0]\n"
                  "frme_obj.retro_widget_is_worker = %s\n"
                  "frme_obj.retro_widget_worker_id = %d\n"
                  "if parentName not in bpy.data.objects:\n"
                  "    frme_obj.retro_widget_parent = parentName\n"
                  "else:\n"
                  "    frme_obj.parent = bpy.data.objects[parentName]\n",
                  w.header.name.c_str(), w.header.parent.c_str(),
                  w.type.toString().c_str(),
                  w.header.useAnimController ? "True" : "False",
                  w.header.defaultVisible ? "True" : "False",
                  w.header.defaultActive ? "True" : "False",
                  w.header.cullFaces ? "True" : "False",
                  w.header.color.vec[0], w.header.color.vec[1], w.header.color.vec[2], w.header.color.vec[3],
                  w.header.modelDrawFlags,
                  w.isWorker ? "True" : "False",
                  w.workerId);

        if (w.type == SBIG('MODL'))
        {
            using MODLInfo = FRME::Widget::MODLInfo;
            MODLInfo* info = static_cast<MODLInfo*>(w.widgetInfo.get());
            hecl::ProjectPath modelPath = pakRouter.getWorking(info->model);
            const PAKRouter<PAKBridge>::EntryType* cmdlE = pakRouter.lookupEntry(info->model, nullptr, true, true);

            os.linkBlend(modelPath.getAbsolutePathUTF8().c_str(),
                         pakRouter.getBestEntryName(*cmdlE).c_str(), true);

            os.format("frme_obj.retro_model_light_mask = %d\n", info->lightMask);
            os << "print(obj.name)\n"
                  "copy_obj = duplicateObject(obj)\n"
                  "copy_obj.parent = frme_obj\n"
                  "copy_obj.hide = False\n";
        }
        else if (w.type == SBIG('CAMR'))
        {
            os << "bpy.context.scene.camera = frme_obj\n"
                  "if 'Camera' in bpy.data.objects:\n"
                  "    cam = bpy.data.objects['Camera']\n"
                  "    bpy.context.scene.objects.unlink(cam)\n"
                  "    bpy.data.objects.remove(cam)\n";
        }
        else if (w.type == SBIG('PANE'))
        {
            using PANEInfo = Widget::PANEInfo;
            if (PANEInfo* info = static_cast<PANEInfo*>(w.widgetInfo.get()))
            {
                os.format("frme_obj.retro_pane_dimensions = (%f,%f)\n"
                          "frme_obj.retro_pane_scale_center = (%f,%f,%f)\n",
                          info->xDim, info->zDim,
                          info->scaleCenter.vec[0],
                          info->scaleCenter.vec[1],
                          info->scaleCenter.vec[2]);
            }
        }
        else if (w.type == SBIG('TXPN'))
        {
            using TXPNInfo = Widget::TXPNInfo;
            if (TXPNInfo* info = static_cast<TXPNInfo*>(w.widgetInfo.get()))
            {
                hecl::ProjectPath fontPath = pakRouter.getWorking(info->font, true);
                hecl::ProjectPath jpFontPath;
                if (frme.version >= 1)
                    jpFontPath = pakRouter.getWorking(info->jpnFont, true);

                os.format("frme_obj.retro_pane_dimensions = (%f,%f)\n"
                          "frme_obj.retro_pane_scale_center = (%f,%f,%f)\n"
                          "frme_obj.retro_textpane_font_path = '%s'\n"
                          "frme_obj.retro_textpane_word_wrap = %s\n"
                          "frme_obj.retro_textpane_vertical = %s\n"
                          "frme_obj.retro_textpane_fill_color = (%f,%f,%f,%f)\n"
                          "frme_obj.retro_textpane_outline_color = (%f,%f,%f,%f)\n"
                          "frme_obj.retro_textpane_block_extent = (%f,%f)\n"
                          "frme_obj.retro_textpane_jp_font_path = '%s'\n"
                          "frme_obj.retro_textpane_jp_font_scale = (%d,%d)\n"
                          "frme_obj.retro_textpane_hjustification = bpy.types.Object.retro_textpane_hjustification[1]['items'][%d][0]\n"
                          "frme_obj.retro_textpane_vjustification = bpy.types.Object.retro_textpane_vjustification[1]['items'][%d][0]\n",
                          info->xDim, info->zDim,
                          info->scaleCenter.vec[0],
                          info->scaleCenter.vec[1],
                          info->scaleCenter.vec[2],
                          fontPath.getRelativePathUTF8().c_str(),
                          info->wordWrap ? "True" : "False",
                          info->vertical ? "True" : "False",
                          info->fillColor.vec[0],
                          info->fillColor.vec[1],
                          info->fillColor.vec[2],
                          info->fillColor.vec[3],
                          info->outlineColor.vec[0],
                          info->outlineColor.vec[1],
                          info->outlineColor.vec[2],
                          info->outlineColor.vec[3],
                          info->blockExtent.vec[0],
                          info->blockExtent.vec[1],
                          jpFontPath.getRelativePathUTF8().c_str(),
                          info->jpnPointScale[0],
                          info->jpnPointScale[1],
                          int(info->justification),
                          int(info->verticalJustification));
            }
        }
        else if (w.type == SBIG('TBGP'))
        {
            using TBGPInfo = Widget::TBGPInfo;
            if (TBGPInfo* info = static_cast<TBGPInfo*>(w.widgetInfo.get()))
            {
                os.format("frme_obj.retro_tablegroup_elem_count = %d\n"
                          "frme_obj.retro_tablegroup_elem_default = %d\n"
                          "frme_obj.retro_tablegroup_wraparound = %s\n",
                          info->elementCount, info->defaultSelection,
                          info->selectWraparound ? "True" : "False");
            }
        }
        else if (w.type == SBIG('GRUP'))
        {
            using GRUPInfo = Widget::GRUPInfo;
            if (GRUPInfo* info = static_cast<GRUPInfo*>(w.widgetInfo.get()))
            {
                os.format("frme_obj.retro_group_default_worker = %d\n",
                          info->defaultWorker);
            }
        }
        else if (w.type == SBIG('SLGP'))
        {
            using SLGPInfo = Widget::SLGPInfo;
            if (SLGPInfo* info = static_cast<SLGPInfo*>(w.widgetInfo.get()))
            {
                os.format("frme_obj.retro_slider_min = %f\n"
                          "frme_obj.retro_slider_max = %f\n"
                          "frme_obj.retro_slider_default = %f\n"
                          "frme_obj.retro_slider_increment = %f\n",
                          info->min, info->max, info->cur, info->increment);
            }
        }
        else if (w.type == SBIG('ENRG'))
        {
            using ENRGInfo = Widget::ENRGInfo;
            if (ENRGInfo* info = static_cast<ENRGInfo*>(w.widgetInfo.get()))
            {
                hecl::ProjectPath txtrPath = pakRouter.getWorking(info->texture);
                if (txtrPath)
                    os.format("frme_obj.retro_energybar_texture_path = '%s'\n",
                              txtrPath.getRelativePathUTF8().c_str());
            }
        }
        else if (w.type == SBIG('METR'))
        {
            using METRInfo = Widget::METRInfo;
            if (METRInfo* info = static_cast<METRInfo*>(w.widgetInfo.get()))
            {
                os.format("frme_obj.retro_meter_no_round_up = %s\n"
                          "frme_obj.retro_meter_max_capacity = %d\n"
                          "frme_obj.retro_meter_worker_count = %d\n",
                          info->noRoundUp ? "True" : "False",
                          info->maxCapacity, info->workerCount);
            }
        }

        os.format("mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                  "mtxd = mtx.decompose()\n"
                  "frme_obj.rotation_mode = 'QUATERNION'\n"
                  "frme_obj.location = mtxd[0]\n"
                  "frme_obj.rotation_quaternion = mtxd[1] * angle\n"
                  "frme_obj.scale = mtxd[2]\n"
                  "bpy.context.scene.objects.link(frme_obj)\n",
                  w.basis[0].vec[0], w.basis[0].vec[1], w.basis[0].vec[2], w.origin.vec[0],
                  w.basis[1].vec[0], w.basis[1].vec[1], w.basis[1].vec[2], w.origin.vec[1],
                  w.basis[2].vec[0], w.basis[2].vec[1], w.basis[2].vec[2], w.origin.vec[2]);
    }

    os.centerView();
    os.close();
    conn.saveBlend();
    return true;
}

}
}
