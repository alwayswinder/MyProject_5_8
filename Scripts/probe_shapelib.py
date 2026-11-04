"""探测 ControlRig 形状库里是否有 Box_thick 及其尺寸。"""
import json

import unreal


def main():
    out = {'libraries': {}, 'thick': []}
    for path in ('/Engine/Plugins/Animation/ControlRig/Content/Controls/ControlRigShapes',
                 '/Engine/Plugins/Animation/ControlRig/Content/Controls/ControlRigShapes.ControlRigShapes',
                 '/Engine/ControlRig/Controls/ControlRigShapes'):
        try:
            a = unreal.load_asset(path)
            if a is None:
                out['libraries'][path] = 'None'
                continue
            out['libraries'][path] = str(a.get_class().get_name())
            shapes = a.get_editor_property('shapes')
            names = []
            for s in shapes:
                try:
                    n = str(s.get_editor_property('shape_name'))
                except Exception:
                    n = str(s)
                names.append(n)
                if 'thick' in n.lower():
                    out['thick'].append(n)
            out['libraries'][path + '::names'] = names
        except Exception as exc:
            out['libraries'][path] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
