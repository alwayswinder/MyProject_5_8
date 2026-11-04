import json
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
CK = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
BK = {n: unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n) for n in ('meibu_L', 'meibu_R')}
out = {}


def probe(v):
    i = rig.create_control_rig()
    i.request_init()
    rh = i.get_hierarchy()
    rh.set_control_value(CK, rh.make_control_value_from_vector(unreal.Vector(*v)))
    i.execute('Forwards Solve')
    res = {}
    for n, k in BK.items():
        t = rh.get_global_transform(k).translation
        res[n] = [round(t.x, 3), round(t.y, 3), round(t.z, 3)]
    ctl = rh.get_global_transform(CK).translation
    res['ctrl'] = [round(ctl.x, 3), round(ctl.y, 3), round(ctl.z, 3)]
    return res


base = probe((0, 0, 0))
out['base'] = base
for tag, v in (('value_X_+1', (1, 0, 0)), ('value_Y_+1', (0, 1, 0)), ('value_Z_+1', (0, 0, 1))):
    cur = probe(v)
    out[tag] = {
        'ctrl_delta': [round(cur['ctrl'][i] - base['ctrl'][i], 3) for i in range(3)],
        'L_delta': [round(cur['meibu_L'][i] - base['meibu_L'][i], 3) for i in range(3)],
        'R_delta': [round(cur['meibu_R'][i] - base['meibu_R'][i], 3) for i in range(3)],
    }
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
