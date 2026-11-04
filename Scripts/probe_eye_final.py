"""最终核对：控制数、各前缀节点数、编译状态。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    names = [str(n.get_name()) for n in rig.get_default_model().get_nodes()]
    controls = [str(k.name) for k in h.get_all_keys() if k.type == unreal.RigElementType.CONTROL]
    out = {
        'status': str(rig.get_editor_property('status')),
        'control_count': len(controls),
        'controls': sorted(controls),
        'node_total': len(names),
        'prefixes': {
            'BROW_': len([n for n in names if n.startswith('BROW_')]),
            'BPT_': len([n for n in names if n.startswith('BPT_')]),
            'BB_': len([n for n in names if n.startswith('BB_')]),
            'BBL_': len([n for n in names if n.startswith('BBL_')]),
            'EYE_': len([n for n in names if n.startswith('EYE_')]),
        },
        'dup_eye': len([n for n in names if n.startswith('EYE_')]) != len(set(n for n in names if n.startswith('EYE_'))),
    }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
