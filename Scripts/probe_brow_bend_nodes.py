"""检查 BB_ 节点无重复、编译状态。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    names = [str(n.get_name()) for n in rig.get_default_model().get_nodes()]
    bb = [n for n in names if n.startswith('BB_')]
    out = {
        'status': str(rig.get_editor_property('status')),
        'node_total': len(names),
        'bb_count': len(bb),
        'bb_dup': len(bb) != len(set(bb)),
        'bb_names': bb,
    }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
