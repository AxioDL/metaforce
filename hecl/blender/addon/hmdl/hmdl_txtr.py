'''
HMDL Export Blender Addon
By Jack Andersen <jackoalan@gmail.com>

This file provides the means to generate an RGBA TXTR resource
buffer for packaging into an .hlpk (yes, I know this is slow,
but it's very flexible and supports Blender's procedural textures)
'''

from mathutils import Vector
import struct

def count_bits(num):
    accum = 0
    index = 0
    for i in range(32):
        if ((num >> i) & 1):
            accum += 1
            index = i
    return accum, index

def make_txtr(tex, size=(512,512)):
    
    if tex.type == 'IMAGE':
        size = tex.image.size
    
    # Validate image for mipmapping
    can_mipmap = False
    w_bits, w_idx = count_bits(size[0])
    h_bits, h_idx = count_bits(size[1])
    if w_bits == 1 and h_bits == 1 and tex.use_mipmap:
        can_mipmap = True

    # Main image 2D array
    main_array = []
    for y in range(size[1]):
        row = []
        main_array.append(row)
        for x in range(size[0]):
            texel = tex.evaluate((x * 2 / size[0] - 1.0, y * 2 / size[1] - 1.0, 0))
            row.append(texel)

    # Count potential mipmap levels
    series_count = 1
    if can_mipmap:
        if size[0] > size[1]:
            series_count = w_idx + 1
        else:
            series_count = h_idx + 1

    # Make header
    tex_bytes = struct.pack('IHHI', 0, size[0], size[1], series_count)

    # Initial mipmap level
    for y in main_array:
        for x in y:
            tex_bytes += struct.pack('BBBB',
                                     min(255, int(x[0]*256)),
                                     min(255, int(x[1]*256)),
                                     min(255, int(x[2]*256)),
                                     min(255, int(x[3]*256)))

    # Prepare mipmap maker
    if can_mipmap:

        # Box filter
        prev_array = main_array
        for i in range(series_count - 1):
            new_array = []
            for y in range(max(len(prev_array) // 2, 1)):
                y1 = prev_array[y*2]
                if len(prev_array) > 1:
                    y2 = prev_array[y*2+1]
                else:
                    y2 = prev_array[y*2]
                new_row = []
                new_array.append(new_row)
                for x in range(max(len(y1) // 2, 1)):
                    texel_val = Vector((0,0,0,0))
                    texel_val += y1[x*2]
                    texel_val += y2[x*2]
                    if len(y1) > 1:
                        texel_val += y1[x*2+1]
                        texel_val += y2[x*2+1]
                    else:
                        texel_val += y1[x*2]
                        texel_val += y2[x*2]
                    texel_val /= 4
                    new_row.append(texel_val)
                    tex_bytes += struct.pack('BBBB',
                                             min(255, int(texel_val[0]*256)),
                                             min(255, int(texel_val[1]*256)),
                                             min(255, int(texel_val[2]*256)),
                                             min(255, int(texel_val[3]*256)))

            prev_array = new_array

    return tex_bytes

