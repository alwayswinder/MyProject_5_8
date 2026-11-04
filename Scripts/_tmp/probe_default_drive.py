import json
import time
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
out = {}

def live():
    for a in unreal.EditorLevelLibrary.get_all_level_actors():
        if a.get_actor_label() != 'CR_Seven_MorphSliders':
            continue
        for c in a.get_components_by_class(unreal.SkeletalMeshComponent):
            res = {}
            for b in ('head', 'meibu_L', 'meibu_R'):
                t = c.get_bone_transform(b, unreal.RelativeTransformSpace.RTS_WORLD)
                res[b] = round(t.translation.z, 2)
            return res
    return None

out['before'] = live()
h.set_control_value(k, h.make_control_value_from_vector(unreal.Vector(0.0, 0.0, 1.25)))
time.sleep(1.0)
out['after_Z125'] = live()
h.set_control_value(k, h.make_control_value_from_vector(unreal.Vector(1.25, 0.0, 0.0)))
time.sleep(1.0)
out['after_X125'] = live()
h.set_control_value(k, h.make_control_value_from_vector(unreal.Vector(0.0, 0.0, 0.0)))
time.sleep(0.5)
out['reset'] = live()
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
