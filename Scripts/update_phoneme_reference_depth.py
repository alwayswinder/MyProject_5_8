"""将参考图后移 6 cm，保持滑块及图形的 X/Z 布局，保存并恢复预览。"""
import datetime,json,shutil,types
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
m=types.ModuleType('reference');exec(compile((ROOT/'Scripts/build_phoneme_reference.py').read_text(encoding='utf-8'),'reference','exec'),m.__dict__)
E=m.E;p=json.loads((E/'parameters.json').read_text(encoding='utf-8'))
rig=unreal.load_asset(m.b.ASSET);h=rig.get_hierarchy();ck=m.b.b.key(m.CONTROL,unreal.RigElementType.CONTROL)
assert h.contains(ck)
preview=m.b.b.base.preview_values(rig)
before=m.signature(rig);old=h.get_global_control_offset_transform(ck,True).translation
assert abs(old.x-47)<1e-5 and abs(old.z-164)<1e-5
assert abs(old.y-18)<1e-5,'Depth already updated or changed manually'
old_offsets={str(k.name):h.get_global_control_offset_transform(k,True).to_tuple() for k in h.get_controls() if str(k.name)!=m.CONTROL}
backup=ROOT/'Saved/Codex/Backup'/('PhonemeReferenceDepth_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S'));backup.mkdir(parents=True)
disk=ROOT/'Content/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation.uasset'
shutil.copy2(disk,backup/'disk_before.uasset');assert unreal.EditorAssetLibrary.save_loaded_asset(rig,False);shutil.copy2(disk,backup/'memory_before.uasset')
with unreal.ScopedEditorTransaction('Move phoneme reference behind sliders'):
    rig.modify()
    for initial in (True,False):
        tf=h.get_global_control_offset_transform(ck,initial);tf.translation=unreal.Vector(47,12,164)
        h.set_control_offset_transform(ck,tf,initial,True)
    rig.recompile_vm();assert rig.get_editor_property('status')!=unreal.BlueprintStatus.BS_ERROR
assert m.signature(rig)==before
assert {str(k.name):h.get_global_control_offset_transform(k,True).to_tuple() for k in h.get_controls() if str(k.name)!=m.CONTROL}==old_offsets
assert unreal.EditorAssetLibrary.save_loaded_asset(rig,False)
sub=unreal.get_editor_subsystem(unreal.AssetEditorSubsystem);sub.close_all_editors_for_asset(rig)
assert unreal.EditorLoadingAndSavingUtils.reload_packages([rig.get_outer()],unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE)[0]
rig=unreal.load_asset(m.b.ASSET);h=rig.get_hierarchy()
assert (h.get_global_control_offset_transform(ck,True).translation-unreal.Vector(47,12,164)).length()<1e-5
assert m.signature(rig)==before
assert {str(k.name):h.get_global_control_offset_transform(k,True).to_tuple() for k in h.get_controls() if str(k.name)!=m.CONTROL}==old_offsets
sub.open_editor_for_assets([rig]);live=rig.get_debugged_control_rig();assert live;lh=live.get_hierarchy()
for n,v in preview.items():lh.set_control_value(m.b.b.key(n,unreal.RigElementType.CONTROL),lh.make_control_value_from_vector(unreal.Vector(*v)))
for n,v in preview.items():assert (lh.get_vector_from_control_value(lh.get_control_value(m.b.b.key(n,unreal.RigElementType.CONTROL)))-unreal.Vector(*v)).length()<1e-5
p['origin']=[47,12,164];p['depth_behind_sliders_cm']=6
(E/'parameters.json').write_text(json.dumps(p,ensure_ascii=False,indent=2),encoding='utf-8')
report=dict(passed=True,saved=True,reloaded=True,backup=str(backup),before=[old.x,old.y,old.z],after=p['origin'],slider_y=18,depth_behind_sliders_cm=6,other_offsets_unchanged=True,graph_unchanged=True,preview_restored=True,captured_at=datetime.datetime.now().astimezone().isoformat(),visual_approval='pending_user')
(E/'depth_update.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
