"""验证八个眼神滑块，保存重载、复验并恢复预览输入。"""
import datetime,json,types
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));E=ROOT/'Docs/Evidence/EyeExpressions'

def main(preview_values=None):
    v=types.ModuleType('verify');exec(compile((ROOT/'Scripts/verify_eye_expression_sliders.py').read_text(encoding='utf-8'),'verify','exec'),v.__dict__)
    before=v.main();rig=unreal.load_asset(v.b.ASSET);build=json.loads((E/'build_report.json').read_text(encoding='utf-8'));preview=(v.b.b.base.preview_values(rig) or build['preview_values']) if preview_values is None else preview_values;helper=v.b.b.base.base.helper
    links=helper.exec_links(rig.get_default_model());assert sorted(links)==sorted(tuple(x) for x in build['expected_execution_links'])
    adj={a.split('.')[0]:b.split('.')[0] for a,b in links};assert len(adj)==len(links);starts=set(adj)-set(adj.values());assert len(starts)==1
    cursor=starts.pop();assert cursor in ('Begin','BeginExecution');visited=[]
    while cursor in adj:
        assert cursor not in visited;visited.append(cursor);cursor=adj[cursor]
    visited.append(cursor);assert len(visited)==len(links)+1
    assert unreal.EditorAssetLibrary.save_loaded_asset(rig,False)
    sub=unreal.get_editor_subsystem(unreal.AssetEditorSubsystem);sub.close_all_editors_for_asset(rig)
    result=unreal.EditorLoadingAndSavingUtils.reload_packages([rig.get_outer()],unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE);assert result[0]
    rig=unreal.load_asset(v.b.ASSET);after=v.main();assert (before['nodes'],before['controls'])==(after['nodes'],after['controls']);assert sorted(helper.exec_links(rig.get_default_model()))==sorted(links)
    build['saved']=True;(E/'build_report.json').write_text(json.dumps(build,ensure_ascii=False,indent=2),encoding='utf-8')
    sub.open_editor_for_assets([rig]);live=rig.get_debugged_control_rig();assert live;lh=live.get_hierarchy()
    # 让预览动画节点完成正常求值；不在其外侧提前 Execute 曲线。
    for n,value in preview.items():
        ck=v.b.b.key(n,unreal.RigElementType.CONTROL);lh.set_control_value(ck,lh.make_control_value_from_vector(unreal.Vector(*value)))
    err=0
    for n,value in preview.items():
        ck=v.b.b.key(n,unreal.RigElementType.CONTROL);got=lh.get_vector_from_control_value(lh.get_control_value(ck));err=max(err,(got-unreal.Vector(*value)).length())
    assert err<1e-5;live.clear_control_selection();live.select_control('CTRL_EYE_EXPR_QI_R',True)
    report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),asset=v.b.ASSET,nodes=after['nodes'],controls=after['controls'],reload_result=str(result),execution_chain=visited,validation_after_reload_passed=True,preview_restored=True,preview_input_error=err,selected_control='CTRL_EYE_EXPR_QI_R',status=str(rig.get_editor_property('status')))
    (E/'reload_report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps({k:val for k,val in report.items() if k!='execution_chain'},ensure_ascii=False))

if __name__=='__main__':main()
