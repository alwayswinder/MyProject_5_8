"""重新打开资产刷新预览求值，恢复原输入并进行帧后检查；不修改全局设置。"""
import json,unreal
from pathlib import Path
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
bp=unreal.load_asset('/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation')
sub=unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
live=bp.get_debugged_control_rig();h=live.get_hierarchy();values={str(k.name):h.get_vector_from_control_value(h.get_control_value(k)).to_tuple() for k in h.get_all_keys() if k.type==unreal.RigElementType.CONTROL}
sub.close_all_editors_for_asset(bp);sub.open_editor_for_assets([bp])
live=bp.get_debugged_control_rig();h=live.get_hierarchy()
for n,v in values.items():h.set_control_value(unreal.RigElementKey(type=unreal.RigElementType.CONTROL,name=n),h.make_control_value_from_vector(unreal.Vector(*v)))
live.clear_control_selection();live.select_control('CTRL_EXPR_HUANGZHANG',True)
exec(compile((root/'Scripts/probe_expression_preview.py').read_text(encoding='utf-8'),'preview_probe','exec'))
