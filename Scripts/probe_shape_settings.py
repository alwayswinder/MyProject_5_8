"""查 RigControlSettings 里与手柄显示相关的属性（shape transform 等）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_L')
    st = h.get_control_settings(k)

    out = {'props': []}
    for p in dir(st):
        if p.startswith('_'):
            continue
        low = p.lower()
        if 'shape' in low or 'color' in low or 'transform' in low or 'scale' in low:
            try:
                v = getattr(st, p)
                out['props'].append({'name': p, 'value': str(v)[:120]})
            except Exception as exc:
                out['props'].append({'name': p, 'error': repr(exc)[:80]})

    out['offset_scale'] = [round(x, 3) for x in (
        h.get_global_control_offset_transform(k).scale3d.x,
        h.get_global_control_offset_transform(k).scale3d.y,
        h.get_global_control_offset_transform(k).scale3d.z)]

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
