"Defines a node group for all game shaders to provide individual outputs to"

import bpy

# UV modifier nodes:
# http://www.metroid2002.com/retromodding/wiki/Materials_(Metroid_Prime)#UV_Animations

# 0 - Modelview Inverse (zero translation)
def make_uvm0():
    new_grp = bpy.data.node_groups.new('RWKUVMode0Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (500, 0)
    
    # UV vertical-flip (to match GameCube's UV-coordinate space)
    v_flip = new_grp.nodes.new('ShaderNodeMapping')
    v_flip.location = (100, 0)
    v_flip.vector_type = 'TEXTURE'
    v_flip.scale[1] = -1.0
    
    # Links
    new_grp.links.new(grp_in.outputs[0], v_flip.inputs[0])
    new_grp.links.new(v_flip.outputs[0], grp_out.inputs[0])

# 1 - Modelview Inverse
def make_uvm1():
    new_grp = bpy.data.node_groups.new('RWKUVMode1Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-300, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (500, 0)
    
    # Geometry input
    geom_in = new_grp.nodes.new('ShaderNodeGeometry')
    geom_in.location = (-700, 0)
    
    # View flip
    view_flip = new_grp.nodes.new('ShaderNodeMapping')
    view_flip.location = (-500, -100)
    view_flip.vector_type = 'TEXTURE'
    view_flip.scale = (-1.0, -1.0, 1.0)

    # Normal/translation add
    adder = new_grp.nodes.new('ShaderNodeVectorMath')
    adder.location = (-100, 0)
    adder.operation = 'ADD'
    
    # UV vertical-flip (to match GameCube's UV-coordinate space)
    v_flip = new_grp.nodes.new('ShaderNodeMapping')
    v_flip.location = (100, 0)
    v_flip.vector_type = 'TEXTURE'
    v_flip.scale[1] = -1.0
    
    # Links
    new_grp.links.new(grp_in.outputs[0], adder.inputs[0])
    new_grp.links.new(geom_in.outputs['View'], view_flip.inputs[0])
    new_grp.links.new(view_flip.outputs[0], adder.inputs[1])
    new_grp.links.new(adder.outputs[0], v_flip.inputs[0])
    new_grp.links.new(v_flip.outputs[0], grp_out.inputs[0])

# 2 - UV Scroll
def make_uvm2():
    new_grp = bpy.data.node_groups.new('RWKUVMode2Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketVector', 'Offset')
    new_grp.inputs.new('NodeSocketVector', 'Scale')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (500, 0)
    
    # Adder1
    adder1 = new_grp.nodes.new('ShaderNodeVectorMath')
    adder1.operation = 'ADD'
    adder1.location = (100, 0)
    
    # Adder2
    adder2 = new_grp.nodes.new('ShaderNodeVectorMath')
    adder2.operation = 'ADD'
    adder2.location = (100, 200)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], adder2.inputs[0])
    new_grp.links.new(grp_in.outputs[1], adder1.inputs[0])
    new_grp.links.new(grp_in.outputs[2], adder1.inputs[1])
    new_grp.links.new(adder1.outputs[0], adder2.inputs[1])
    new_grp.links.new(adder2.outputs[0], grp_out.inputs[0])

# 3 - Rotation
def make_uvm3():
    new_grp = bpy.data.node_groups.new('RWKUVMode3Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Offset')
    new_grp.inputs.new('NodeSocketFloat', 'Scale')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (700, 0)
    
    # Adder1
    add1 = new_grp.nodes.new('ShaderNodeMath')
    add1.operation = 'ADD'
    add1.location = (500, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], grp_out.inputs[0])
    new_grp.links.new(grp_in.outputs[1], add1.inputs[0])
    new_grp.links.new(grp_in.outputs[2], add1.inputs[1])

# 4 - Horizontal Filmstrip Animation
def make_uvm4():
    new_grp = bpy.data.node_groups.new('RWKUVMode4Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Scale')
    new_grp.inputs.new('NodeSocketFloat', 'NumFrames')
    new_grp.inputs.new('NodeSocketFloat', 'Step')
    new_grp.inputs.new('NodeSocketFloat', 'Offset')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1000, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (800, 0)
    
    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMath')
    mult1.operation = 'MULTIPLY'
    mult1.location = (-800, 0)
    
    # Multiply2
    mult2 = new_grp.nodes.new('ShaderNodeMath')
    mult2.operation = 'MULTIPLY'
    mult2.location = (-600, 0)
    
    # Modulo
    mod1 = new_grp.nodes.new('ShaderNodeMath')
    mod1.operation = 'MODULO'
    mod1.inputs[1].default_value = 1.0
    mod1.location = (-400, 0)
    
    # Multiply3
    mult3 = new_grp.nodes.new('ShaderNodeMath')
    mult3.operation = 'MULTIPLY'
    mult3.location = (-200, 0)
    
    # Multiply4
    mult4 = new_grp.nodes.new('ShaderNodeMath')
    mult4.operation = 'MULTIPLY'
    mult4.location = (0, 0)
    
    # Mapping
    map1 = new_grp.nodes.new('ShaderNodeMapping')
    map1.scale = (1.0, 0.0, 0.0)
    map1.location = (200, 0)
    
    # Add
    add1 = new_grp.nodes.new('ShaderNodeVectorMath')
    add1.operation = 'ADD'
    add1.location = (600, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], add1.inputs[1])
    new_grp.links.new(grp_in.outputs[1], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult3.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult4.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult1.inputs[0])
    new_grp.links.new(grp_in.outputs[4], mult2.inputs[1])
    new_grp.links.new(mult1.outputs[0], mult2.inputs[0])
    new_grp.links.new(mult2.outputs[0], mod1.inputs[0])
    new_grp.links.new(mod1.outputs[0], mult3.inputs[0])
    new_grp.links.new(mult3.outputs[0], mult4.inputs[0])
    new_grp.links.new(mult4.outputs[0], map1.inputs[0])
    new_grp.links.new(map1.outputs[0], add1.inputs[0])
    new_grp.links.new(add1.outputs[0], grp_out.inputs[0])

# 5 - Vertical Filmstrip Animation
def make_uvm5():
    new_grp = bpy.data.node_groups.new('RWKUVMode5Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'Scale')
    new_grp.inputs.new('NodeSocketFloat', 'NumFrames')
    new_grp.inputs.new('NodeSocketFloat', 'Step')
    new_grp.inputs.new('NodeSocketFloat', 'Offset')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-1000, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (800, 0)
    
    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMath')
    mult1.operation = 'MULTIPLY'
    mult1.location = (-800, 0)
    
    # Multiply2
    mult2 = new_grp.nodes.new('ShaderNodeMath')
    mult2.operation = 'MULTIPLY'
    mult2.location = (-600, 0)
    
    # Modulo
    mod1 = new_grp.nodes.new('ShaderNodeMath')
    mod1.operation = 'MODULO'
    mod1.inputs[1].default_value = 1.0
    mod1.location = (-400, 0)
    
    # Multiply3
    mult3 = new_grp.nodes.new('ShaderNodeMath')
    mult3.operation = 'MULTIPLY'
    mult3.location = (-200, 0)
    
    # Multiply4
    mult4 = new_grp.nodes.new('ShaderNodeMath')
    mult4.operation = 'MULTIPLY'
    mult4.location = (0, 0)
    
    # Mapping
    map1 = new_grp.nodes.new('ShaderNodeMapping')
    map1.scale = (0.0, 1.0, 0.0)
    map1.location = (200, 0)
    
    # Add
    add1 = new_grp.nodes.new('ShaderNodeVectorMath')
    add1.operation = 'ADD'
    add1.location = (600, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], add1.inputs[1])
    new_grp.links.new(grp_in.outputs[1], mult1.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult3.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult4.inputs[1])
    new_grp.links.new(grp_in.outputs[3], mult1.inputs[0])
    new_grp.links.new(grp_in.outputs[4], mult2.inputs[1])
    new_grp.links.new(mult1.outputs[0], mult2.inputs[0])
    new_grp.links.new(mult2.outputs[0], mod1.inputs[0])
    new_grp.links.new(mod1.outputs[0], mult3.inputs[0])
    new_grp.links.new(mult3.outputs[0], mult4.inputs[0])
    new_grp.links.new(mult4.outputs[0], map1.inputs[0])
    new_grp.links.new(map1.outputs[0], add1.inputs[0])
    new_grp.links.new(add1.outputs[0], grp_out.inputs[0])

# 6 - Model Matrix
def make_uvm6():
    new_grp = bpy.data.node_groups.new('RWKUVMode6Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-100, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (300, 0)
    
    # Geometry input
    geom_in = new_grp.nodes.new('ShaderNodeGeometry')
    geom_in.location = (-300, 0)
    
    # Adder1
    adder1 = new_grp.nodes.new('ShaderNodeVectorMath')
    adder1.operation = 'ADD'
    adder1.location = (100, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], adder1.inputs[0])
    new_grp.links.new(geom_in.outputs['Global'], adder1.inputs[1])
    new_grp.links.new(adder1.outputs[0], grp_out.inputs[0])

# 7 - Mode Who Must Not Be Named
def make_uvm7():
    new_grp = bpy.data.node_groups.new('RWKUVMode7Node', 'ShaderNodeTree')
    new_grp.inputs.new('NodeSocketVector', 'UV In')
    new_grp.inputs.new('NodeSocketFloat', 'ParamA')
    new_grp.inputs.new('NodeSocketFloat', 'ParamB')
    new_grp.outputs.new('NodeSocketVector', 'UV Out')
    new_grp.use_fake_user = True
    
    # Group inputs
    grp_in = new_grp.nodes.new('NodeGroupInput')
    grp_in.location = (-800, 0)
    
    # Group outputs
    grp_out = new_grp.nodes.new('NodeGroupOutput')
    grp_out.location = (0, 0)
    
    # Geometry input
    geom_in = new_grp.nodes.new('ShaderNodeGeometry')
    geom_in.location = (-1000, 0)
    
    # View flip
    view_flip = new_grp.nodes.new('ShaderNodeMapping')
    view_flip.location = (-800, -150)
    view_flip.vector_type = 'TEXTURE'
    view_flip.scale = (-1.0, -1.0, 1.0)

    # Separate
    sep1 = new_grp.nodes.new('ShaderNodeSeparateRGB')
    sep1.location = (-400, -200)

    # Add1
    add1 = new_grp.nodes.new('ShaderNodeMath')
    add1.operation = 'ADD'
    add1.location = (-200, -200)

    # Multiply1
    mult1 = new_grp.nodes.new('ShaderNodeMath')
    mult1.operation = 'MULTIPLY'
    mult1.inputs[1].default_value = 0.025
    mult1.location = (0, -200)

    # Multiply2
    mult2 = new_grp.nodes.new('ShaderNodeMath')
    mult2.operation = 'MULTIPLY'
    mult2.location = (200, -200)

    # Multiply3
    mult3 = new_grp.nodes.new('ShaderNodeMath')
    mult3.operation = 'MULTIPLY'
    mult3.inputs[1].default_value = 0.05
    mult3.location = (0, -400)

    # Multiply4
    mult4 = new_grp.nodes.new('ShaderNodeMath')
    mult4.operation = 'MULTIPLY'
    mult4.location = (200, -400)

    # Combine1
    comb1 = new_grp.nodes.new('ShaderNodeCombineRGB')
    comb1.location = (400, -300)

    # Combine2
    comb2 = new_grp.nodes.new('ShaderNodeCombineRGB')
    comb2.location = (-600, 0)

    # Multiply5
    mult5 = new_grp.nodes.new('ShaderNodeMixRGB')
    mult5.blend_type = 'MULTIPLY'
    mult5.inputs[0].default_value = 1.0
    mult5.location = (-400, 0)

    # Add2
    add2 = new_grp.nodes.new('ShaderNodeVectorMath')
    add2.operation = 'ADD'
    add2.location = (-200, 0)
    
    # Links
    new_grp.links.new(grp_in.outputs[0], add2.inputs[0])
    new_grp.links.new(geom_in.outputs['View'], view_flip.inputs[0])
    new_grp.links.new(view_flip.outputs[0], sep1.inputs[0])
    new_grp.links.new(grp_in.outputs[1], comb2.inputs[0])
    new_grp.links.new(grp_in.outputs[1], comb2.inputs[1])
    new_grp.links.new(grp_in.outputs[1], comb2.inputs[2])
    new_grp.links.new(comb2.outputs[0], mult5.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult2.inputs[1])
    new_grp.links.new(grp_in.outputs[2], mult4.inputs[1])
    new_grp.links.new(sep1.outputs[0], add1.inputs[0])
    new_grp.links.new(sep1.outputs[1], add1.inputs[1])
    new_grp.links.new(sep1.outputs[2], mult3.inputs[0])
    new_grp.links.new(add1.outputs[0], mult1.inputs[0])
    new_grp.links.new(mult1.outputs[0], mult2.inputs[0])
    new_grp.links.new(mult2.outputs[0], comb1.inputs[0])
    new_grp.links.new(mult3.outputs[0], mult4.inputs[0])
    new_grp.links.new(mult4.outputs[0], comb1.inputs[1])
    new_grp.links.new(comb1.outputs[0], mult5.inputs[2])
    new_grp.links.new(mult5.outputs[0], add2.inputs[1])
    new_grp.links.new(add2.outputs[0], grp_out.inputs[0])

UV_MODIFIER_GROUPS = [
    make_uvm0,
    make_uvm1,
    make_uvm2,
    make_uvm3,
    make_uvm4,
    make_uvm5,
    make_uvm6,
    make_uvm7
]

def make_master_shader_library():
    for uvm in UV_MODIFIER_GROUPS:
        uvm()

