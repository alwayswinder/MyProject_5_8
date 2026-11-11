"""把 Seven 完整面部图迁入用户指定的新式资产，保留目标路径和类型。

只允许空目标；已有输出不会被重复覆盖。源 Blueprint 仅读取。
"""

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
import shutil
import unreal
from slider_handle_layout import apply_slider_layout, RAIL_CONTROL

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = _fm_asset('/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation')
TARGET = _fm_asset('/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset')
E = _fm_tool('Docs/Evidence/SevenRuntime')

def digest(path):
    return hashlib.sha256(_fm_read(path).read_bytes()).hexdigest()

def main():
    assert SOURCE != TARGET, '源和目标不能相同；检查任务资产映射'
    source, target = unreal.load_asset(SOURCE), unreal.load_asset(TARGET)
    assert source and target
    assert target.get_class().get_name() == 'ControlRigRuntimeAsset'
    assert target.get_editor_property('preview_skeletal_mesh') == source.get_preview_mesh()
    editor = target.get_editor_asset()
    sh, th = source.get_hierarchy(), target.hierarchy
    sm, tm = source.get_default_model(), editor.get_default_model()
    assert len(th.get_controls()) == 0 and len(tm.get_nodes()) <= 1, '目标已有内容，停止覆盖'
    assert len(sh.get_bones()) == 149 and len(sh.get_curves()) == 28
    assert len({str(k.name) for k in sh.get_controls()} - {RAIL_CONTROL}) == 73
    assert not source.get_local_function_library().get_nodes(), '存在本地函数，需额外迁移'
    _fm_output(E).mkdir(parents=True, exist_ok=True)
    backup = _fm_saved('Backup') / ('SevenRuntime_' + datetime.datetime.now().strftime('%Y%m%d_%H%M%S'))
    _fm_output(backup).mkdir(parents=True)
    disk = _fm_disk('Content/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset.uasset')
    source_disk = _fm_disk('Content/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation.uasset')
    shutil.copy2(disk, backup / 'disk_before.uasset')
    assert _fm_save_asset(target, False)
    shutil.copy2(disk, backup / 'memory_before.uasset')
    source_hash = digest(source_disk)
    text = source.get_controller(sm).export_nodes_to_text([n.get_fname() for n in sm.get_nodes()])
    (_fm_output(E / 'graph_export.txt')).write_text(text, encoding='utf-8')
    report = dict(captured_at=datetime.datetime.now().astimezone().isoformat(), source=SOURCE, target=TARGET,
                  engine=unreal.SystemLibrary.get_engine_version(), backup=str(backup),
                  source_disk_sha256=source_hash, graph_export_sha256=hashlib.sha256(text.encode()).hexdigest(),
                  source_nodes=len(sm.get_nodes()), source_links=len(sm.get_links()), passed=False)
    (_fm_output(E / 'build_report.json')).write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    auto = editor.get_auto_vm_recompile()
    try:
        editor.set_auto_vm_recompile(False)
        with unreal.ScopedEditorTransaction('Populate Seven facial controls in runtime asset'):
            target.modify()
            editor.modify()
            th.copy_hierarchy(sh)
            for prop in ('hierarchy_settings', 'shape_libraries', 'source_hierarchy_import', 'source_curve_import'):
                target.set_editor_property(prop, source.get_editor_property(prop))
            editor.set_preview_mesh(source.get_preview_mesh(), True)
            controller = editor.get_controller(tm)
            for node in list(tm.get_nodes()):
                assert controller.remove_node(node, False)
            imported = controller.import_nodes_from_text(text, False, False)
            assert len(imported) == len(sm.get_nodes()), (len(imported), len(sm.get_nodes()))
            # 新资产以作者设定的 Initial 值启动，不继承参考资产的当前预览姿势。
            th.reset_pose_to_initial(unreal.RigElementType.CONTROL)
            # 示例轨道可能穿过手柄；目标统一应用拾取间距，不改写示例资产。
            apply_slider_layout(target)
            editor.recompile_vm()
            assert len(tm.get_nodes()) == len(sm.get_nodes())
            def links(model):
                return sorted((str(l.get_source_pin().get_pin_path()), str(l.get_target_pin().get_pin_path())) for l in model.get_links())
            assert links(tm) == links(sm), '迁移后的连接与源不一致'
    finally:
        editor.set_auto_vm_recompile(auto)
    assert digest(source_disk) == source_hash
    report.update(passed=True, target_nodes=len(tm.get_nodes()), target_links=len(tm.get_links()),
                  bones=len(th.get_bones()), controls=len(th.get_controls()), curves=len(th.get_curves()),
                  source_disk_unchanged=True, saved=False)
    (_fm_output(E / 'build_report.json')).write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False))

if __name__ == '__main__':
    main()
