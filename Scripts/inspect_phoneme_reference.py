import json,unreal
tex=unreal.load_asset('/Game/_MyTest/Mouth/LidDiagnostic/口型单列竖排12个_去背景_1')
assert tex
print('TEXTURE',tex.get_path_name(),tex.blueprint_get_size_x(),tex.blueprint_get_size_y())
try:print('SOURCES',tex.get_editor_property('asset_import_data').extract_filenames())
except Exception as exc:print(str(exc))
lib=unreal.load_asset('/ControlRig/Controls/DefaultGizmoLibraryNormalized')
print('LIB',lib.get_class().get_name(),[n for n in dir(unreal) if 'ShapeLibrary' in n])
print('PLANE',unreal.load_asset('/Engine/BasicShapes/Plane').get_bounding_box())
print('LIB_MATERIAL',lib.get_editor_property('default_material').get_path_name())
