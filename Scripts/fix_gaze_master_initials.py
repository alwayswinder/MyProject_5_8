"""修复总控改父级时引入的单眼非零初始值，并验证持久化。"""
import datetime,json,shutil,types
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));E=ROOT/'Docs/Evidence/GazeMaster'
def main():
    m=types.ModuleType('eye');exec(compile((ROOT/'Scripts/build_eyeball_controls.py').read_text(encoding='utf-8'),'eyes','exec'),m.__dict__)
    rig=unreal.load_asset(m.ASSET);h=rig.get_hierarchy();preview=m.base.preview_values(rig)
    old=json.loads((ROOT/'Docs/Evidence/Eyeballs/parameters.json').read_text(encoding='utf-8'))
    backup=ROOT/'Saved/Codex/Backup'/('GazeInitials_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S'));backup.mkdir(parents=True)
    disk=ROOT/'Content/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation.uasset';shutil.copy2(disk,backup/'disk_before.uasset')
    assert unreal.EditorAssetLibrary.save_loaded_asset(rig,False);shutil.copy2(disk,backup/'memory_before.uasset')
    before={};rig.modify()
    with unreal.ScopedEditorTransaction('Fix binocular child control initial values'):
        for s in old['sides']:
            name=s['control'];ck=m.key(name,unreal.RigElementType.CONTROL)
            iv=h.get_vector_from_control_value(h.get_control_value(ck,unreal.RigControlValueType.INITIAL))
            before[name]=dict(initial_value=iv.to_tuple(),initial_position=h.get_global_transform(ck,True).translation.to_tuple())
            # A reset-to-bad-default pose is corrected to zero; intentional
            # user input that differs from the bad default is retained.
            current=preview.get(name,[0,0,0])
            if iv.length()>100 and (unreal.Vector(*current)-iv).length()<1e-4:preview[name]=[0,0,0]
            h.set_control_value(ck,h.make_control_value_from_vector(unreal.Vector()),unreal.RigControlValueType.INITIAL)
            h.set_control_value(ck,h.make_control_value_from_vector(unreal.Vector(*preview.get(name,[0,0,0]))),unreal.RigControlValueType.CURRENT)
        rig.recompile_vm();assert rig.get_editor_property('status')!=unreal.BlueprintStatus.BS_ERROR
    reload=types.ModuleType('reload_master');exec(compile((ROOT/'Scripts/reload_gaze_master.py').read_text(encoding='utf-8'),'reload','exec'),reload.__dict__);reload.main(preview)
    rig=unreal.load_asset(m.ASSET);live=rig.get_debugged_control_rig();assert live
    after={}
    for label,hh in (('blueprint',rig.get_hierarchy()),('preview',live.get_hierarchy())):
        after[label]={}
        for s in old['sides']:
            ck=m.key(s['control'],unreal.RigElementType.CONTROL);iv=hh.get_vector_from_control_value(hh.get_control_value(ck,unreal.RigControlValueType.INITIAL));position=hh.get_global_transform(ck,True).translation
            assert iv.length()<1e-6 and (position-unreal.Vector(*s['control_rest'])).length()<1e-5
            after[label][s['control']]=dict(initial_value=iv.to_tuple(),initial_position=position.to_tuple(),current_position=hh.get_global_transform(ck).translation.to_tuple())
    report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),asset=m.ASSET,backup=str(backup),cause='Maintain-global reparent changed child initial local values; explicit relative offsets retained that compensation twice',before=before,after=after,restored_preview=preview,saved_and_reloaded=True,passed=True)
    (E/'initial_position_fix.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps(report,ensure_ascii=False))
if __name__=='__main__':main()
