"""确认两个控制手柄的世界位置/大小，并查 ControlRig 编辑器相关 Python 入口。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {'controls': {}}
    for name in ('CTRL_BROW_L', 'CTRL_BROW_R'):
        k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=name)
        g = h.get_global_control_offset_transform(k)
        out['controls'][name] = {
            'world_t': [round(g.translation.x, 2), round(g.translation.y, 2), round(g.translation.z, 2)],
            'scale': [round(g.scale3d.x, 2), round(g.scale3d.y, 2), round(g.scale3d.z, 2)],
        }
    out['editor_classes'] = [n for n in dir(unreal) if 'ControlRigEditor' in n or 'PreviewMode' in n]
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
