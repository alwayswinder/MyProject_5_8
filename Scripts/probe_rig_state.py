"""检查 rig 当前骨骼与预览网格引用状态。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {'status': str(rig.get_editor_property('status'))}
    for b in ('Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038'):
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=b)
        try:
            t = h.get_global_transform(k)
            out[b] = [round(t.translation.x, 3), round(t.translation.y, 3), round(t.translation.z, 3)]
        except Exception as exc:
            out[b] = 'ERR %r' % exc
    try:
        out['bone_count'] = len([k for k in h.get_all_keys() if k.type == unreal.RigElementType.BONE])
    except Exception as exc:
        out['bone_count'] = 'ERR %r' % exc
    try:
        pm = rig.get_preview_mesh()
        out['preview_mesh'] = str(pm)
    except Exception as exc:
        out['preview_mesh'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
