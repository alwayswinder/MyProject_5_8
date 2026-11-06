"""保存后关闭指定 Rig 编辑器，重载包，再打开；不涉及其他资产编辑器。"""
import json
from pathlib import Path
import unreal

asset = '/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation'
rig = unreal.load_asset(asset)
subsystem = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
subsystem.close_all_editors_for_asset(rig)
package = rig.get_outer()
result = unreal.EditorLoadingAndSavingUtils.reload_packages([package], unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE)
assert result[0], str(result)
rig = unreal.load_asset(asset)
assert rig.get_editor_property('status') == unreal.BlueprintStatus.BS_UP_TO_DATE
assert len(rig.get_default_model().get_nodes()) == 394
subsystem.open_editor_for_assets([rig])
root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
(root / 'Docs/Evidence/XYP/reload.json').write_text(json.dumps({'asset': asset, 'reload_result': str(result), 'nodes': 394, 'opened': True}), encoding='utf-8')
print('Reloaded and opened', asset)
