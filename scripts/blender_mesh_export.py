print("Hi!")

import random
import struct

import bpy
from mathutils import Vector

obj = bpy.context.object
mesh = obj.data

mesh.calc_normals_split()

num_vertices = len(mesh.vertices)
positions = []
normals = []
colors = []
colorData = mesh.color_attributes["Color"].data
for vertIx in range(num_vertices):
    v = mesh.vertices[vertIx]
    pos = v.co
    norm = v.normal
    col = Vector(colorData[vertIx].color)
    positions.extend(pos)
    normals.extend(norm)
    colors.extend(col[:3])
    # colors.extend([random.random(), random.random(), random.random()])
    if vertIx < 5:
        print(f"vert[{vertIx}] {pos}, {norm}, {col}")

# mesh.polygons are for actual faces, loop_triangles are triangulated faces

indices = []
for triIx, tri in enumerate(mesh.loop_triangles):
    indices.extend(tri.vertices)
    if triIx < 5:
        print(triIx, tri.vertices[0], tri.vertices[1], tri.vertices[2])
num_indices = len(indices)

print(f"num vertices {num_vertices}, num positions {len(positions)}, num normals {len(normals)}, num colors {len(colors)}, num indices {len(indices)}")

with open("c:/Users/veliu/Documents/spiky.mesh", "wb") as f:
    f.write(struct.pack('I', num_vertices))
    f.write(struct.pack('I', num_indices))
    for x in positions:
        f.write(struct.pack('f', x))
    for x in normals:
        f.write(struct.pack('f', x))
    for x in colors:
        f.write(struct.pack('f', x))
    for ix in indices:
        f.write(struct.pack('I', ix))
    

print("Bye!")