"""最终状态核对：控制列表 + BPT_ 节点数（应无重复）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {'controls': []}
    for k in h.get_all_keys():
        if k.type != unreal.RigElementType.CONTROL:
            continue
        st = h.get_control_settings(k)
        off = h.get_global_control_offset_transform(k).translation
        out['controls'].append({
            'name': str(k.name),
            'display': str(st.display_name),
            'type': str(st.control_type).split('.')[-1],
            'shape': str(st.shape_name),
            't': [round(off.x, 3), round(off.y, 3), round(off.z, 3)],
            'parent': str(h.get_first_parent(k).name),
        })
    nodes = [str(n.get_name()) for n in rig.get_default_model().get_nodes()]
    out['node_count'] = len(nodes)
    out['bpt_nodes'] = [n for n in nodes if n.startswith('BPT_')]
    out['dup_check'] = len(out['bpt_nodes']) == len(set(out['bpt_nodes']))
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
