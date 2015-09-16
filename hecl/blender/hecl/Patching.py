import bpy
import os
import os.path
from pathlib import Path

def _mkdir(path):
    try:
        os.mkdir(path)
    except:
        pass

def path_components(path):
    retval = []
    base, end = os.path.split(path)
    while end != '':
        retval.insert(0, end)
        base, end = os.path.split(base)
    return retval

def find_project_root():
    if bpy.data.filepath == '':
        return None
    path = os.path.split(bpy.data.filepath)
    test_path = os.path.join(path[0], '.hecl')
    while not os.path.exists(test_path):
        path = os.path.split(path[0])
        test_path = os.path.join(path[0], '.hecl')
    if os.path.exists(test_path):
        return path[0]
    return None

def get_patching_dir(make_dirs=False):
    proj_root = find_project_root()
    if not proj_root:
        return None
    rel_to_blend = os.path.relpath(bpy.data.filepath, start=proj_root)
    rel_to_blend_comps = path_components(rel_to_blend)
    trace_dir = os.path.join(proj_root, '.hecl', 'patches')
    if not make_dirs and not os.path.exists(trace_dir):
        return None
    _mkdir(trace_dir)
    for comp in rel_to_blend_comps:
        ext_pair = os.path.splitext(comp)
        if ext_pair[1] == '.blend':
            trace_dir = os.path.join(trace_dir, ext_pair[0])
            if not make_dirs and not os.path.exists(trace_dir):
                return None
            _mkdir(trace_dir)
            return trace_dir
        trace_dir = os.path.join(trace_dir, comp)
        if not make_dirs and not os.path.exists(trace_dir):
            return None
        _mkdir(trace_dir)


class FILE_OT_hecl_patching_save(bpy.types.Operator):
    '''Save text datablocks to hecl patching directory'''
    bl_idname = "file.hecl_patching_save"
    bl_label = "Save HECL Patches"
    bl_options = {'REGISTER'}

    def execute(self, context):
        patching_dir = get_patching_dir(make_dirs=True)
        if not patching_dir:
            self.report({'ERROR'}, 'Unable to save patches for ' + bpy.data.filepath)
            return {'CANCELLED'}
        count = 0
        for text in bpy.data.texts:
            if not text.name.endswith('.py'):
                continue
            text_abspath = os.path.join(patching_dir, text.name)
            text_file = open(text_abspath, 'w')
            text_file.write(text.as_string())
            text_file.close()
            count += 1
        if count == 1:
            self.report({'INFO'}, 'saved 1 patch')
        else:
            self.report({'INFO'}, 'saved %d patches' % count)
        return {'FINISHED'}

class FILE_OT_hecl_patching_load(bpy.types.Operator):
    '''Load text datablocks from hecl patching directory'''
    bl_idname = "file.hecl_patching_load"
    bl_label = "Load HECL Patches"
    bl_options = {'REGISTER'}

    def execute(self, context):
        patching_dir = get_patching_dir()
        if not patching_dir:
            self.report({'ERROR'}, 'Unable to load patches for ' + bpy.data.filepath)
            return {'CANCELLED'}
        p = Path(patching_dir)
        count = 0
        for path in p.glob('*.py'):
            path = path.name
            text_abspath = os.path.join(patching_dir, path)
            text_file = open(text_abspath, 'r')
            if path in bpy.data.texts:
                text = bpy.data.texts[path]
            else:
                text = bpy.data.texts.new(path)
            text.from_string(text_file.read())
            text_file.close()
            count += 1
        if count == 1:
            self.report({'INFO'}, 'loaded 1 patch')
        else:
            self.report({'INFO'}, 'loaded %d patches' % count)
        return {'FINISHED'}

def save_func(self, context):
    self.layout.operator("file.hecl_patching_save", text="Save HECL Patches")

def load_func(self, context):
    self.layout.operator("file.hecl_patching_load", text="Load HECL Patches")

def register():
    bpy.utils.register_class(FILE_OT_hecl_patching_save)
    bpy.utils.register_class(FILE_OT_hecl_patching_load)
    bpy.types.INFO_MT_file_external_data.append(load_func)
    bpy.types.INFO_MT_file_external_data.append(save_func)

def unregister():
    bpy.utils.unregister_class(FILE_OT_hecl_patching_save)
    bpy.utils.unregister_class(FILE_OT_hecl_patching_load)
    bpy.types.INFO_MT_file_external_data.remove(load_func)
    bpy.types.INFO_MT_file_external_data.remove(save_func)
