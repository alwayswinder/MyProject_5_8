import unreal

hc = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders').get_hierarchy_controller()

for fn in ('add_bone', 'add_control', 'import_bones', 'set_control_settings', 'add_curve'):
    f = getattr(hc, fn, None)
    print("=== %s" % fn)
    print(getattr(f, '__doc__', None))

print("=== RigHierarchy methods")
h = hc.get_hierarchy()
print([m for m in dir(h) if 'bone' in m.lower() or 'transform' in m.lower()])
