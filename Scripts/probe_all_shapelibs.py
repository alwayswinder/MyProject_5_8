"""枚举所有 ControlRigShapeLibrary 资产与其中的 shape 名。"""
import json

import unreal


def main():
    out = {'libs': [], 'thick': []}
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    classes = []
    for cls_name in ('/Script/ControlRig.ControlRigShapeLibrary',
                     '/Script/ControlRigEditor.ControlRigShapeLibrary'):
        try:
            classes.append(unreal.TopLevelAssetPath(cls_name))
        except Exception:
            pass
    for cls in classes:
        try:
            assets = ar.get_assets_by_class(cls, True)
        except Exception as exc:
            out['libs'].append({'class': str(cls), 'err': repr(exc)})
            continue
        for a in assets:
            path = str(a.package_name)
            item = {'class': str(cls), 'path': path}
            try:
                lib = unreal.load_asset(path)
                if lib is not None:
                    names = []
                    for s in lib.get_editor_property('shapes'):
                        try:
                            n = str(s.get_editor_property('shape_name'))
                        except Exception:
                            n = str(s)
                        names.append(n)
                        if 'thick' in n.lower():
                            out['thick'].append(path + '::' + n)
                    item['shapes'] = names
                else:
                    item['shapes'] = 'load None'
            except Exception as exc:
                item['shapes'] = 'ERR %r' % exc
            out['libs'].append(item)
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
