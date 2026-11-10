"""仅对指定新资产保存、包重载，并重复完整验证。"""

# Shared package paths: reference inputs are read-only; reports go to the target project.
from face_paths import (tool_path as _fm_tool, legacy_saved_path as _fm_saved,
    project_root as _fm_project, read_path as _fm_read, output_path as _fm_output,
    asset_path as _fm_asset, asset_disk_path as _fm_disk,
    load_asset as _fm_load_asset, save_asset as _fm_save_asset,
    node_name as _fm_node, break_link as _fm_break_link)
import sys as _fm_sys
_fm_sys.dont_write_bytecode = True
import datetime
import hashlib
import json
from pathlib import Path
import types
import unreal

ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
E=_fm_tool('Docs/Evidence/SevenRuntime')
TARGET=_fm_asset('/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset')
before=json.loads((_fm_read(E/'before_reload/summary.json')).read_text(encoding='utf-8'))
assert before['passed'], '保存前验证尚未通过'
target=unreal.load_asset(TARGET)
hierarchy=target.hierarchy
preview={str(k.name):list(hierarchy.get_vector_from_control_value(hierarchy.get_control_value(k)).to_tuple()) for k in hierarchy.get_controls()}
assert _fm_save_asset(target,False)
sub=unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
sub.close_all_editors_for_asset(target)
reload_result=unreal.EditorLoadingAndSavingUtils.reload_packages([target.get_outer()],unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE)
assert reload_result[0],str(reload_result)
target=unreal.load_asset(TARGET)
verify=types.ModuleType('runtime_verify')
exec(compile((_fm_read(_fm_tool('Scripts/verify_runtime_face_controls.py'))).read_text(encoding='utf-8'),'runtime_verify','exec'),verify.__dict__)
after=verify.main('after_reload')
assert after['passed'],after
build=json.loads((_fm_read(E/'build_report.json')).read_text(encoding='utf-8'))
source_disk=_fm_disk('Content/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation.uasset')
assert hashlib.sha256(_fm_read(source_disk).read_bytes()).hexdigest()==build['source_disk_sha256']
sub.open_editor_for_assets([target])
build['saved']=True
(_fm_output(E/'build_report.json')).write_text(json.dumps(build,ensure_ascii=False,indent=2),encoding='utf-8')
report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),target=TARGET,passed=True,
            saved=True,reloaded=True,reload_result=str(reload_result),validation_after_reload=after,
            source_disk_unchanged=True,target_editor_open=True,authored_inputs_before_reload=preview)
(_fm_output(E/'reload_report.json')).write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in report.items() if k!='authored_inputs_before_reload'},ensure_ascii=False))
