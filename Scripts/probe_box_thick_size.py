"""读取 Box_Thick / Box_Thin / Box_Solid 的实际尺寸（shape proxy 包围盒）。"""
import json

import unreal

LIB = '/ControlRig/Controls/DefaultGizmoLibraryNormalized'


def main():
    out = {}
    lib = unreal.load_asset(LIB)
    defs = lib.get_editor_property('shapes')
    for d in defs:
        try:
            name = str(d.get_editor_property('shape_name'))
        except Exception:
            continue
        if name not in ('Box_Thick', 'Box_Thin', 'Box_Solid'):
            continue
        item = {'name': name}
        try:
            proxy = d.get_editor_property('shape_proxy')
            item['proxy'] = str(proxy)
            try:
                path = proxy.to_soft_object_path().export_text() if hasattr(proxy, 'to_soft_object_path') else str(proxy)
            except Exception:
                path = None
            item['path'] = path
            if path:
                mesh = unreal.load_asset(path.split('.')[0])
                if mesh:
                    b = mesh.get_bounding_box()
                    item['size'] = [round(b.max.x - b.min.x, 4), round(b.max.y - b.min.y, 4), round(b.max.z - b.min.z, 4)]
                    item['min'] = [round(b.min.x, 4), round(b.min.y, 4), round(b.min.z, 4)]
                    item['max'] = [round(b.max.x, 4), round(b.max.y, 4), round(b.max.z, 4)]
        except Exception as exc:
            item['err'] = repr(exc)
        out[name] = item
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
