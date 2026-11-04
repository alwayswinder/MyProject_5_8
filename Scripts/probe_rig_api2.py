import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
print('HIER:', [m for m in dir(h) if ('bone' in m.lower() or 'add' in m.lower() or 'element' in m.lower())][:60])
ctrl = rig.get_controller(rig.get_default_model())
print('CTRL:', [m for m in dir(ctrl) if ('link' in m.lower() or 'pin' in m.lower())][:60])
print('HIER_KEYS:', [m for m in dir(h) if 'key' in m.lower()][:40])
