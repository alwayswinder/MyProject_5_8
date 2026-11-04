"""打印 UE 里眉毛骨骼世界坐标与父级，用于确定控制摆放位置。"""
import json
import traceback

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    names = ['meibu_L', 'Bone007', 'Bone008', 'Bone009',
             'meibu_R', 'Bone010', 'Bone011', 'Bone012', 'head']
    out = {}
    for n in names:
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)
        try:
            t = h.get_global_transform(k).translation
            local = h.get_local_transform(k).translation
            par = str(h.get_first_parent(k).name)
            out[n] = {
                'world': [round(t.x, 3), round(t.y, 3), round(t.z, 3)],
                'local': [round(local.x, 3), round(local.y, 3), round(local.z, 3)],
                'parent': par,
            }
        except Exception as exc:
            out[n] = {'error': repr(exc)}
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')
    return out


if __name__ == '__main__':
    try:
        main()
    except Exception:
        unreal.log_error('[ProbeBrowBones] FAILED\n' + traceback.format_exc())
        raise
