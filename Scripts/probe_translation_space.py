"""实测 set_global_transform / set_local_transform 对眉毛骨骼世界位移的影响。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def key(name):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=name)


def make_instance():
    rig = unreal.load_asset(ASSET)
    inst = rig.create_control_rig()
    inst.request_init()
    return inst


def probe(mode, comp, amount, bone):
    inst = make_instance()
    rh = inst.get_hierarchy()
    k = key(bone)
    before = rh.get_global_transform(k).translation
    if mode == 'global':
        g = rh.get_global_transform(k)
        t = g.translation
        if comp == 'X':
            t.x += amount
        elif comp == 'Y':
            t.y += amount
        else:
            t.z += amount
        g.translation = t
        rh.set_global_transform(k, g, False, True)
    else:
        lt = rh.get_local_transform(k)
        t = lt.translation
        if comp == 'X':
            t.x += amount
        elif comp == 'Y':
            t.y += amount
        else:
            t.z += amount
        lt.translation = t
        rh.set_local_transform(k, lt, False, True)
    inst.execute('Forwards Solve')
    after = rh.get_global_transform(k).translation
    return {
        'mode': mode, 'bone': bone, 'comp': comp, 'amount': amount,
        'delta_world': [round(after.x - before.x, 3), round(after.y - before.y, 3), round(after.z - before.z, 3)],
    }


def main():
    out = {'results': []}
    for bone in ('meibu_L', 'meibu_R'):
        for mode in ('global', 'local'):
            for comp in ('X', 'Y', 'Z'):
                try:
                    out['results'].append(probe(mode, comp, 5.0, bone))
                except Exception as exc:
                    out['results'].append({'mode': mode, 'bone': bone, 'comp': comp, 'err': repr(exc)})
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
