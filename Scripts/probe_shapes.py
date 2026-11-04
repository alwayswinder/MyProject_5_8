"""列出现有 Control Rig 形状库里的形状名，方便选一个贴合眉毛的。"""
import json

import unreal

CANDIDATES = [
    '/Engine/ControlRig/Controls/ControlRigShapes',
    '/Engine/ControlRig/Controls/ControlRigShapes.ControlRigShapes',
    '/Engine/EngineResources/ControlRigShapes',
]

PATHS = [
    '/Engine/ControlRig',
    '/Engine/EditorResources',
    '/Engine/Plugins/Animation/ControlRig/Content',
]


def main():
    out = {'libraries': {}, 'found_assets': []}
    for p in CANDIDATES:
        try:
            a = unreal.load_asset(p)
            out['libraries'][p] = 'OK' if a else 'None'
            if a:
                shapes = a.get_editor_property('shapes')
                names = []
                for s in shapes:
                    try:
                        names.append(str(s.get_editor_property('shape_name')))
                    except Exception:
                        names.append(str(s))
                out['libraries'][p + '::names'] = names
        except Exception as exc:
            out['libraries'][p] = 'ERR %r' % exc

    for p in PATHS:
        try:
            assets = unreal.EditorAssetLibrary.list_assets(p, True, False)
            out['found_assets'].extend([str(a) for a in assets if 'hape' in str(a)][:40])
        except Exception as exc:
            out['found_assets'].append('%s ERR %r' % (p, exc))

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
