import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'
ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'

sm = unreal.load_asset(MESH)
ref = unreal.new_object(unreal.RigHierarchy)
ref.get_controller(True).import_bones_from_skeletal_mesh(sm, 'None', True, True, False, False, False)

for n in ('meibu_L', 'meibu_R', 'head', 'etou_L', 'etou_R', 'yankuang_L'):
    k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)
    if ref.contains(k):
        loc = ref.get_local_transform(k)
        glob = ref.get_global_transform(k)
        parent = ref.get_first_parent(k)
        print("%s parent=%s local_t=%s local_r=%s local_s=%s" % (
            n, parent.name if parent else None, loc.translation, loc.rotation, loc.scale3d))
        print("   global_t=%s" % (glob.translation,))

rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
for n in ('CTRL_1_happy', 'CTRL_1_jing', 'RAIL_1_happy', 'MorphBoard'):
    k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL if n.startswith(('CTRL', 'RAIL')) else unreal.RigElementType.NULL, name=n)
    if h.contains(k):
        off = h.get_control_offset_transform(k)
        print("offset %s: t=%s s=%s" % (n, off.translation, off.scale3d))

