"""只读检查新版资产暴露的 Python 接口。"""
import json
import unreal
target=unreal.load_asset('/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset')
source=unreal.load_asset('/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation')
editor=target.get_editor_asset()
print('EDITOR',editor)
print('EDITOR_METHODS', [n for n in dir(editor) if not n.startswith('_')])
for obj,names in ((editor,('get_default_model','get_controller','create_rig_vm_host','recompile_vm','get_hierarchy','get_editor_property')), (source.get_controller(),('export_nodes_to_text','import_nodes_from_text')), (unreal.ControlRigBlueprintLibrary,('create_control_rig_asset_from_blueprint',)), (unreal.ControlRig,('set_rig_vm_asset','initialize','request_init'))):
    for name in names:
        print(name, getattr(getattr(obj,name,None),'__doc__',None))
for name in ('hierarchy','preview_skeletal_mesh','shape_libraries','vm','editor_only_data'):
    try: print('TARGET_PROP',name,str(target.get_editor_property(name)))
    except Exception as exc: print('MISSING_PROP',name,str(exc))
if hasattr(editor,'get_default_model'):
    print('TARGET_NODES',len(editor.get_default_model().get_nodes()))
print('TARGET_COUNTS',len(target.hierarchy.get_bones()),len(target.hierarchy.get_controls()))
print('HOST_METHODS',[n for n in dir(unreal.ControlRig) if any(s in n for s in ('asset','init','vm'))])
