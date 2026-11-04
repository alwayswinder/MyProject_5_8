import json
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
CK = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
BK = {n: unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n) for n in ('meibu_L', 'meibu_R')}
out = {}


def probe(x, y):
    i = rig.create_control_rig()
    i.request_init()
    rh = i.get_hierarchy()
    rh.set_control_value(CK, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(x, y)))
    i.execute('Forwards Solve')
    res = {}
    for n, k in BK.items():
        t = rh.get_global_transform(k).translation
        res[n] = [round(t.x, 3), round(t.y, 3), round(t.z, 3)]
    return res


base = probe(0, 0)
out['base'] = base
for tag, (x, y) in (('X+10', (10, 0)), ('Y+10', (0, 10))):
    cur = probe(x, y)
    out[tag] = {n: [round(cur[n][i] - base[n][i], 3) for i in range(3)] for n in BK}
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
