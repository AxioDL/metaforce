# Node Grid Arranger Class
NODE_PADDING = 80
FRAME_NAMES = ['Dynamics','Textures','Combiners','Output']
FRAME_WIDTHS = [250, 250, 800, 180]
TOTAL_WIDTH = 0.0
for width in FRAME_WIDTHS:
    TOTAL_WIDTH += width + NODE_PADDING
FRAME_COLORS = [(0.6,0.46,0.6),(0.6,0.48,0.44),(0.33,0.48,0.6),(0.53,0.6,0.47)]
class Nodegrid:

    def __init__(self, nodetree):
        self.ncol = len(FRAME_NAMES)
        self.heights = []
        self.frames = []
        self.col_roffs = [[0.0,0.0]] * self.ncol
        for i in range(self.ncol):
            self.heights.append(0.0)
            frame_node = nodetree.nodes.new('NodeFrame')
            frame_node.label = FRAME_NAMES[i]
            frame_node.use_custom_color = True
            frame_node.color = FRAME_COLORS[i]
            self.frames.append(frame_node)

    def place_node(self, node, col):
        if col < 0 or col >= self.ncol:
            return False

        x_pos = NODE_PADDING
        for i in range(col):
            x_pos += FRAME_WIDTHS[i] + NODE_PADDING*2
        node.location[0] = x_pos - TOTAL_WIDTH/2
        node.location[1] = self.heights[col]
        self.heights[col] -= node.height + NODE_PADDING
        self.frames[col].height += node.height + NODE_PADDING
        node.parent = self.frames[col]

        return True

    def place_node_right(self, node, col, srow):
        heights_backup = self.heights[col]
        if self.place_node(node, col):
            node.location[0] += self.col_roffs[col][srow]
            if srow == 1:
                node.location[1] -= 175
            self.col_roffs[col][srow] += 200
        self.heights[col] = heights_backup

    def row_break(self, col):
        self.heights[col] -= 350
        self.col_roffs[col][0] = 0.0
        self.col_roffs[col][1] = 0.0
