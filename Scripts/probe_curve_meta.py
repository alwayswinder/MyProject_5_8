import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'
sm = unreal.load_asset(MESH)
skel = sm.get_editor_property('skeleton')
print("skeleton:", skel)

for name in ('browInnerUp', 'browDownLeft', 'browDownRight'):
    n = unreal.Name(name)
    try:
        has = skel.get_curve_meta_data(n)
        mt = skel.get_curve_meta_data_morph_target(n)
        print("%s exists=%s morph_target=%s" % (name, has, mt))
    except Exception as exc:
        print("%s err %r" % (name, exc))

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
print("preview mesh:", rig.get_preview_mesh())
