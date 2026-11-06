"""读 Seven 网格与骨架的导入源信息（FBX 路径、时间），判断 UE 网格是否与 Max 场景同源。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'

out = {}
for path in (MESH, '/Game/_MyTest/Mouth/Mesh/Seven_Skeleton'):
    a = unreal.load_asset(path)
    rec = {'class': a.get_class().get_name()}
    try:
        d = a.get_editor_property('asset_import_data')
        rec['import_data'] = str(d)
        if d is not None:
            try:
                rec['source_file'] = str(d.get_first_filename())
            except Exception as exc:
                rec['source_file'] = 'ERR %r' % exc
            for attr in ('source_file', 'import_source', 'source_file_timestamp'):
                try:
                    rec[attr] = str(d.get_editor_property(attr))
                except Exception:
                    pass
    except Exception as exc:
        rec['import_data'] = 'ERR %r' % exc
    try:
        rec['modified'] = str(a.get_outermost().get_editor_property('saved_hash'))
    except Exception:
        pass
    out[path] = rec

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
