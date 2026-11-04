import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'
sm = unreal.load_asset(MESH)
skel = sm.get_editor_property('skeleton')

names = [str(n) for n in skel.get_curve_meta_data_names()]
print("existing curve names (count=%d): %s" % (len(names), names[:40]))

for name in ('browInnerUp', 'browDownLeft', 'browDownRight'):
    n = unreal.Name(name)
    if name not in names:
        try:
            skel.add_curve_meta_data(n, True)
            print("added", name)
        except Exception as exc:
            print("add failed", name, repr(exc))
    try:
        skel.set_curve_meta_data_morph_target(n, True)
        print("%s morph_target=%s" % (name, skel.get_curve_meta_data_morph_target(n)))
    except Exception as exc:
        print("set failed", name, repr(exc))

unreal.EditorAssetLibrary.save_loaded_asset(skel, False)
print("saved skeleton")
