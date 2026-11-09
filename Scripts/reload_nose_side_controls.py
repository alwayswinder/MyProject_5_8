"""保存重载鼻侧控制、再次复核，并恢复用户预览输入。"""
import datetime,json,types
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));E=ROOT/'Docs/Evidence/NoseSides'
v=types.ModuleType('verify_nose_sides');exec(compile((ROOT/'Scripts/verify_nose_side_controls.py').read_text(encoding='utf-8'),'verify_nose_sides','exec'),v.__dict__)
before=v.main();rig=unreal.load_asset(v.b.ASSET);build=json.loads((E/'build_report.json').read_text(encoding='utf-8'))
links=v.b.b.base.base.helper.exec_links(rig.get_default_model());assert sorted(links)==sorted(tuple(x) for x in build['old_execution_links'])
assert unreal.EditorAssetLibrary.save_loaded_asset(rig,False)
sub=unreal.get_editor_subsystem(unreal.AssetEditorSubsystem);sub.close_all_editors_for_asset(rig)
assert unreal.EditorLoadingAndSavingUtils.reload_packages([rig.get_outer()],unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE)[0]
rig=unreal.load_asset(v.b.ASSET);after=v.main();assert (before['nodes'],before['controls'])==(after['nodes'],after['controls'])
sub.open_editor_for_assets([rig]);live=rig.get_debugged_control_rig();assert live;h=live.get_hierarchy()
for n,values in build['preview_values'].items():h.set_control_value(v.b.key(n,unreal.RigElementType.CONTROL),h.make_control_value_from_vector(unreal.Vector(*values)))
for n,values in build['preview_values'].items():assert (h.get_vector_from_control_value(h.get_control_value(v.b.key(n,unreal.RigElementType.CONTROL)))-unreal.Vector(*values)).length()<1e-5
live.clear_control_selection();live.select_control('CTRL_NOSE_SIDE_R',True)
build['saved']=True;(E/'build_report.json').write_text(json.dumps(build,ensure_ascii=False,indent=2),encoding='utf-8')
report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),passed=True,saved=True,reloaded=True,preview_restored=True,selected_control='CTRL_NOSE_SIDE_R',execution_chain_unchanged=True,nodes=after['nodes'],controls=after['controls'])
(E/'reload_report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps(report,ensure_ascii=False))
