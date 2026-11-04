import json
import unreal

M = unreal.MathLibrary


def axes(rot):
    q = rot.quaternion()
    f = M.get_forward_vector(rot)
    r = M.get_right_vector(rot)
    u = M.get_up_vector(rot)

    def s(v):
        return [round(v.x, 3), round(v.y, 3), round(v.z, 3)]
    return {'quat': [round(q.x, 3), round(q.y, 3), round(q.z, 3), round(q.w, 3)],
            'X': s(f), 'Y': s(r), 'Z': s(u)}


cases = {
    'identity': unreal.Rotator(0, 0, 0),
    'roll-90(current)': unreal.Rotator(0, 0, -90),
    'roll+90': unreal.Rotator(0, 0, 90),
    'yaw-90': unreal.Rotator(0, -90, 0),
    'yaw+90': unreal.Rotator(0, 90, 0),
    'pitch-90': unreal.Rotator(-90, 0, 0),
}
out = {}
for k, r in cases.items():
    out[k] = axes(r)

out['quat(0.5,0.5,0.5,0.5)'] = axes(unreal.Quat(0.5, 0.5, 0.5, 0.5).rotator())
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
