import json
import unreal

out = {}
for p in ('/Game/_MyTest/Mouth/Rig/CR_Seven_MetaHuman',
          '/Game/_MyTest/Mouth/Rig/CR_Seven_FaceMorphs',
          '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'):
    rig = unreal.load_asset(p)
    h = rig.get_hierarchy()
    ctrls, nulls, bones = [], [], 0
    for k in h.get_all_keys():
        if k.type == unreal.RigElementType.CONTROL:
            ctrls.append(str(k.name))
        elif k.type == unreal.RigElementType.NULL:
            nulls.append(str(k.name))
        elif k.type == unreal.RigElementType.BONE:
            bones += 1
    out[p] = {'controls': ctrls, 'nulls': nulls, 'bone_count': bones}

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
