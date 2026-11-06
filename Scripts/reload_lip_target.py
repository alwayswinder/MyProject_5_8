"""校验、保存并重载嘴唇诊断 Rig，再做相同数值对照并打开编辑器。"""
import datetime
import json
from pathlib import Path
import types
import unreal

ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
EVIDENCE=ROOT/'Docs/Evidence/LipControls'


def main():
    module=types.ModuleType('lip_verify')
    exec(compile((ROOT/'Scripts/verify_lip_controls.py').read_text(encoding='utf-8'),'verify_lip_controls.py','exec'),module.__dict__)
    before=module.main()
    asset=module.helpers.ASSET
    rig=unreal.load_asset(asset)
    model=rig.get_default_model()
    links=module.helpers.helper.exec_links(model)
    # Check that all execution nodes form one reachable chain from BeginExecution.
    adjacent={a.split('.')[0]: b.split('.')[0] for a,b in links}
    assert len(adjacent)==len(links)
    starts=set(adjacent)-set(adjacent.values())
    assert len(starts)==1,starts
    cursor=starts.pop()
    assert cursor in ('Begin','BeginExecution'),cursor
    visited=[]
    while cursor in adjacent:
        assert cursor not in visited
        visited.append(cursor)
        cursor=adjacent[cursor]
    visited.append(cursor)
    assert len(visited)==len(links)+1
    assert all('LIP_SET_'+b in visited for b in json.loads((EVIDENCE/'parameters.json').read_text(encoding='utf-8'))['bones'])
    assert unreal.EditorAssetLibrary.save_loaded_asset(rig,False)
    subsystem=unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
    subsystem.close_all_editors_for_asset(rig)
    result=unreal.EditorLoadingAndSavingUtils.reload_packages([rig.get_outer()],unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE)
    assert result[0],str(result)
    rig=unreal.load_asset(asset)
    after=module.main()
    assert after['nodes']==before['nodes']
    assert after['controls']==before['controls']
    assert sorted(module.helpers.helper.exec_links(rig.get_default_model()))==sorted(links)
    report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),asset=asset,
                reload_result=str(result),nodes=after['nodes'],controls=after['controls'],
                execution_nodes=len(visited),execution_chain=visited,
                status=after['status'],validation_after_reload_passed=after['passed'],
                max_position_error_cm=after['max_position_error_cm'],
                max_rotation_error_deg=after['max_rotation_error_deg'])
    (EVIDENCE/'reload_report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    subsystem.open_editor_for_assets([rig])
    print(json.dumps(report,ensure_ascii=False))


if __name__=='__main__':
    main()
