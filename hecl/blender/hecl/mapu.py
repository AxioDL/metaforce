import bpy

def cook(writebuf):
    found_lib = False
    for obj in bpy.context.scene.objects:
        if obj.library:
            path = os.path.normpath(bpy.path.abspath(obj.library.filepath))
            writebuf(struct.pack('I', len(path)))
            writebuf(path.encode())
            found_lib = True
            break
    if not found_lib:
        raise RuntimeError('No hexagon segments present')

    for obj in bpy.context.scene.objects:
        if not obj.parent and obj.type == 'EMPTY':
            writebuf(struct.pack('I', len(obj.name)))
            writebuf(obj.name.encode())
            writebuf(struct.pack('ffffffffffffffff',
                                 obj.matrix_local[0][0], obj.matrix_local[0][1], obj.matrix_local[0][2], obj.matrix_local[0][3],
                                 obj.matrix_local[1][0], obj.matrix_local[1][1], obj.matrix_local[1][2], obj.matrix_local[1][3],
                                 obj.matrix_local[2][0], obj.matrix_local[2][1], obj.matrix_local[2][2], obj.matrix_local[2][3],
                                 obj.matrix_local[3][0], obj.matrix_local[3][1], obj.matrix_local[3][2], obj.matrix_local[3][3]))
            writebuf(struct.pack('I', len(obj.children)))
            for child in obj.children:
                writebuf(struct.pack('ffffffffffffffff',
                                     child.matrix_local[0][0], child.matrix_local[0][1], child.matrix_local[0][2], child.matrix_local[0][3],
                                     child.matrix_local[1][0], child.matrix_local[1][1], child.matrix_local[1][2], child.matrix_local[1][3],
                                     child.matrix_local[2][0], child.matrix_local[2][1], child.matrix_local[2][2], child.matrix_local[2][3],
                                     child.matrix_local[3][0], child.matrix_local[3][1], child.matrix_local[3][2], child.matrix_local[3][3]))
            writebuf(struct.pack('ffff', obj.retro_mapworld_color[0], obj.retro_mapworld_color[1],
                                         obj.retro_mapworld_color[2], obj.retro_mapworld_color[3]))
            writebuf(struct.pack('I', len(obj.retro_mapworld_path)))
            writebuf(obj.retro_mapworld_path.encode())

def draw(layout, context):
    obj = context.active_object
    if not obj:
        return
    while obj.parent:
        obj = obj.parent
    layout.prop(obj, 'retro_mapworld_color', text='Color')
    layout.prop(obj, 'retro_mapworld_path', text='Path')

# Registration
def register():
    bpy.types.Object.retro_mapworld_color = bpy.props.FloatVectorProperty(name='Retro: MapWorld Color',\
                                            description='Sets map world color', subtype='COLOR', size=4, min=0.0, max=1.0)
    bpy.types.Object.retro_mapworld_path = bpy.props.StringProperty(name='Retro: MapWorld Path', description='Sets path to World root')
