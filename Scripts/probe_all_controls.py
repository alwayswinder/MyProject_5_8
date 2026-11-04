"""最终核对：所有控制形状 + 编译状态 + 节点数。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {'controls': [], 'status': str(rig.get_editor_property('status'))}
    for k in h.get_all_keys():
        if k.type != unreal.RigElementType.CONTROL:
            continue
        st = h.get_control_settings(k)
        off = h.get_global_control_offset_transform(k).translation
        out['controls'].append({
            'name': str(k.name),
            'display': str(st.display_name),
            'shape': str(st.shape_name),
            't': [round(off.x, 2), round(off.y, 2), round(off.z, 2)],
        })
    names = [str(n.get_name()) for n in rig.get_default_model().get_nodes()]
    out['node_count'] = len(names)
    out['bb'] = len([n for n in names if n.startswith('BB_')])
    out['bbl'] = len([n for n in names if n.startswith('BBL_')])
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
