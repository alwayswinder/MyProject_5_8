"""探测 UE 网格的蒙皮权重上限 / LOD 设置。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    mesh = unreal.load_asset(MESH)
    out = {'class': str(mesh.get_class().get_name()), 'props': []}
    for p in dir(mesh):
        low = p.lower()
        if 'bone' in low or 'skin' in low or 'weight' in low or 'lod' in low:
            out['props'].append(p)
    for name in ('max_bones_per_vertex', 'skinning_data', 'lod_info', 'skeleton', 'import_settings'):
        try:
            out[name] = str(mesh.get_editor_property(name))
        except Exception as exc:
            out[name] = 'ERR %r' % exc
    try:
        lods = mesh.get_editor_property('lod_info')
        out['lod_count'] = len(lods)
        if lods:
            l0 = lods[0]
            out['lod0_props'] = [p for p in dir(l0) if 'bone' in p.lower() or 'weight' in p.lower()]
    except Exception as exc:
        out['lod_info'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
