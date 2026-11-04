"""右眉弯曲控制：最终状态与叠加自检。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
BEND = 'CTRL_BROW_R_BEND'


def key(name, kind=unreal.RigElementType.CONTROL):
    return unreal.RigElementKey(type=kind, name=name)


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {'controls': [], 'stack': {}}
    for k in h.get_all_keys():
        if k.type != unreal.RigElementType.CONTROL:
            continue
        st = h.get_control_settings(k)
        off = h.get_global_control_offset_transform(k).translation
        out['controls'].append({
            'name': str(k.name),
            'display': str(st.display_name),
            't': [round(off.x, 2), round(off.y, 2), round(off.z, 2)],
            'parent': str(h.get_first_parent(k).name),
        })

    def probe(values):
        inst = rig.create_control_rig()
        inst.request_init()
        rh = inst.get_hierarchy()
        for c, v in values.items():
            rh.set_control_value(key(c), rh.make_control_value_from_vector(unreal.Vector(*v)))
        inst.execute('Forwards Solve')
        res = {}
        for b in ('Bone010', 'Bone011', 'Bone012'):
            t = rh.get_global_transform(unreal.RigElementKey(type=unreal.RigElementType.BONE, name=b))
            res[b] = [round(t.translation.x, 4), round(t.translation.y, 4), round(t.translation.z, 4)]
        return res

    base = probe({})
    out['base'] = base
    # 整体 + 弯曲叠加
    cur = probe({'CTRL_BROW_R': (10.0, 0.0, 0.0), BEND: (0.0, 0.0, 2.0)})
    out['stack_BROW_R_x10_BEND_z2'] = {b: [round(cur[b][i] - base[b][i], 4) for i in range(3)] for b in base}

    # 点控 + 弯曲叠加
    cur2 = probe({'CTRL_BROW_R_MID': (0.0, 0.0, 5.0), BEND: (0.0, 0.0, -2.0)})
    out['stack_MID_z5_BEND_z-2'] = {b: [round(cur2[b][i] - base[b][i], 4) for i in range(3)] for b in base}

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
