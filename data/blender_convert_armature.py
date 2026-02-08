import bpy
import csv
from mathutils import Vector

# -------- SETTINGS --------
CSV_PATH = "C:\code\projects\hacking\smc64\data\mario_skeleton.csv"
BONE_LENGTH = 10 / 3
HAS_HEADER = False
ARMATURE_NAME = "CSV_Armature"
# --------------------------

# Create armature object
arm_data = bpy.data.armatures.new(ARMATURE_NAME)
arm_obj = bpy.data.objects.new(ARMATURE_NAME, arm_data)
bpy.context.collection.objects.link(arm_obj)

bpy.context.view_layer.objects.active = arm_obj
bpy.ops.object.mode_set(mode='EDIT')

with open(CSV_PATH, newline='') as csvfile:
    reader = csv.reader(csvfile)

    if HAS_HEADER:
        next(reader)

    for i, row in enumerate(reader):
        print(row)
        px, pz, py = map(float, row[0:3])
        fx, fz, fy = map(float, row[3:6])
        ux, uz, uy = map(float, row[6:9])

        pos = Vector((px, -py, pz))
        fwd = Vector((fx, -fy, fz)).normalized()
        up = Vector((ux, -uy, uz)).normalized()
        right = -fwd.cross(up).normalized()


        # Blender bones point down +Y
        head = pos / 3
        tail = pos / 3 + (right * BONE_LENGTH)

        bone = arm_data.edit_bones.new(f"bone_{i}")

        # Build orientation matrix
        corrected_up = right.cross(fwd).normalized()

        bone.head = head
        bone.tail = tail
        bone.align_roll(up)

bpy.ops.object.mode_set(mode='OBJECT')

print("Armature created successfully.")
