"""查 Control Rig 预览模式相关 API。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    out = {'dir_previewmode': [], 'rig_attrs': []}
    for n in dir(unreal):
        if 'PreviewMode' in n or 'Previewing' in n:
            out['dir_previewmode'].append(n)
    for p in dir(rig):
        if 'preview' in p.lower() or 'mode' in p.lower():
            out['rig_attrs'].append(p)

    for prop in ('preview_mode', 'PreviewMode'):
        try:
            out[prop] = str(rig.get_editor_property(prop))
        except Exception as exc:
            out[prop] = 'ERR %r' % exc

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
