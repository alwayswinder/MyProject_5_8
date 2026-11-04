import json
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
CK = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
BK = {n: unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n) for n in ('meibu_L', 'meibu_R', 'head')}
out = {}


def probe(y):
    i = rig.create_control_rig()
    i.request_init()
    rh = i.get_hierarchy()
    rh.set_control_value(CK, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(0.0, y)))
    i.execute('Forwards Solve')
    res = {}
    for n, k in BK.items():
        g = rh.get_global_transform(k).translation
        lo = rh.get_local_transform(k).translation
        res[n] = {'global': [round(g.x, 3), round(g.y, 3), round(g.z, 3)],
                  'local': [round(lo.x, 3), round(lo.y, 3), round(lo.z, 3)]}
    return res


a = probe(0.0)
b = probe(10.0)
out['head_global_rot'] = json.loads(str(rig.get_hierarchy().get_global_transform(BK['head']).rotation).replace('(', '[').replace(')', ']')) if False else None
for n in ('meibu_L', 'meibu_R'):
    out[n] = {
        'd_global': [round(b[n]['global'][i] - a[n]['global'][i], 3) for i in range(3)],
        'd_local': [round(b[n]['local'][i] - a[n]['local'][i], 3) for i in range(3)],
    }
out['head_global_t'] = a['head']['global']
qr = rig.get_hierarchy().get_global_transform(BK['head']).rotation
out['head_quat'] = [round(qr.x, 4), round(qr.y, 4), round(qr.z, 4), round(qr.w, 4)]
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
