import bpy

def draw(layout, context):
    layout.prop_search(context.scene, 'hecl_path_obj', context.scene, 'objects')
    layout.operator('view3d.toggle_path_height_visualization', text='Toggle Height Viz', icon='MANIPUL')
    layout.operator('view3d.toggle_path_background_wireframe', text='Toggle Background Wire', icon='WIRE')

def cook(writebuf, mesh_obj):
    pass

import bpy, bgl
from mathutils import Vector, Matrix
from mathutils.geometry import intersect_ray_tri, tessellate_polygon

def correlate_polygon_heights(context, obj):
    ret = {}
    if obj.type != 'MESH':
        return ret
    for p in obj.data.polygons:
        pl = [obj.matrix_world * obj.data.vertices[vert].co for vert in p.vertices]
        tpl = tessellate_polygon((pl,))
        found = False
        for eobj in context.scene.objects:
            for tri in tpl:
                if eobj.type == 'EMPTY' and eobj.library is None:
                    intersect = intersect_ray_tri(pl[tri[0]], pl[tri[1]], pl[tri[2]],
                                                  Vector((0.0, 0.0, -999.0)),
                                                  eobj.location)
                    if intersect is not None:
                        ret[p] = abs(intersect.z - eobj.location.z)
                        found = True
                        break
            if found:
                break
    return ret

def draw_line_3d(color, start, end):
    bgl.glColor4f(*color)
    bgl.glBegin(bgl.GL_LINES)
    bgl.glVertex3f(*start)
    bgl.glVertex3f(*end)

def draw_callback_3d(self, context):
    # object locations
    if context.scene.hecl_path_obj not in context.scene.objects:
        return
    obj = context.scene.objects[context.scene.hecl_path_obj]
    if obj.type != 'MESH':
        return
    heights = correlate_polygon_heights(context, obj)
    obj_mtx = obj.matrix_world

    for p in obj.data.polygons:
        height = 1.0
        if p in heights:
            height = heights[p]
        for ek in p.edge_keys:
            co0 = obj_mtx * obj.data.vertices[ek[0]].co
            co1 = obj_mtx * obj.data.vertices[ek[1]].co
            draw_line_3d((0.0, 0.0, 1.0, 0.7), co0 + Vector((0.0, 0.0, height)),
                         co1 + Vector((0.0, 0.0, height)))
        for vk in p.vertices:
            co = obj_mtx * obj.data.vertices[vk].co
            draw_line_3d((1.0, 0.0, 0.0, 0.7), co, co + Vector((0.0, 0.0, height)))

    bgl.glEnd()
    # restore opengl defaults
    bgl.glColor4f(0.0, 0.0, 0.0, 1.0)

class PathHeightDrawOperator(bpy.types.Operator):
    bl_idname = "view3d.toggle_path_height_visualization"
    bl_label = "Toggle PATH height visualization"
    _handle_3d = None

    def execute(self, context):
        #heights = correlate_polygon_heights(context, bpy.data.objects['Plane'])
        # the arguments we pass the the callback
        args = (self, context)
        # Add the region OpenGL drawing callback
        # draw in view space with 'POST_VIEW' and 'PRE_VIEW'
        if self._handle_3d is None:
            PathHeightDrawOperator._handle_3d = bpy.types.SpaceView3D.draw_handler_add(draw_callback_3d, args, 'WINDOW', 'POST_VIEW')
        else:
            bpy.types.SpaceView3D.draw_handler_remove(PathHeightDrawOperator._handle_3d, 'WINDOW')
            PathHeightDrawOperator._handle_3d = None

        for ar in bpy.context.screen.areas:
            ar.tag_redraw()
        return {'FINISHED'}

class PathBackgroundWireframeOperator(bpy.types.Operator):
    bl_idname = "view3d.toggle_path_background_wireframe"
    bl_label = "Toggle PATH background wireframe"
    _handle_3d = None

    def execute(self, context):
        if context.scene.background_set:
            to_wire = False
            for o in context.scene.background_set.objects:
                if o.draw_type != 'WIRE':
                    to_wire = True
                    break
            if to_wire:
                for o in context.scene.background_set.objects:
                    o.draw_type = 'WIRE'
            else:
                for o in context.scene.background_set.objects:
                    o.draw_type = 'TEXTURED'
        return {'FINISHED'}

# Registration
def register():
    bpy.types.Scene.hecl_path_obj = bpy.props.StringProperty(
        name='HECL Path Object',
        description='Blender Mesh Object to export during PATH\'s cook process')
    bpy.utils.register_class(PathHeightDrawOperator)
    bpy.utils.register_class(PathBackgroundWireframeOperator)

def unregister():
    bpy.utils.unregister_class(PathHeightDrawOperator)
    bpy.utils.unregister_class(PathBackgroundWireframeOperator)
