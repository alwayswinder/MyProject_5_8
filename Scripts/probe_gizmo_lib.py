"""读取 DefaultGizmoLibrary 的 shape 名称清单。"""
import json

import unreal


def main():
    out = {}
    for path in ('/Engine/Plugins/Animation/ControlRig/Content/Controls/DefaultGizmoLibrary',
                 '/Engine/Plugins/Animation/ControlRig/Content/Controls/DefaultGizmoLibraryNormalized'):
        try:
            lib = unreal.load_asset(path)
            out[path] = str(lib)
            if lib is not None:
                names = []
                for s in lib.get_editor_property('shapes'):
                    try:
                        names.append(str(s.get_editor_property('shape_name')))
                    except Exception:
                        names.append(str(s))
                out[path + '::names'] = names
                try:
                    d = lib.get_editor_property('default_shape')
                    out[path + '::default'] = str(d.get_editor_property('shape_name'))
                except Exception as exc:
                    out[path + '::default'] = 'ERR %r' % exc
        except Exception as exc:
            out[path] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
