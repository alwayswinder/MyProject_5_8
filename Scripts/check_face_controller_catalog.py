"""在 UE 中只读核对名称表覆盖；不构建或修改 Rig。"""
import datetime,json
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
catalog=json.loads((ROOT/'Docs/Characters/Seven.face-controls.json').read_text(encoding='utf-8'))
bp=unreal.load_asset(catalog['target']['control_rig']);assert bp
h=bp.get_hierarchy();wanted={r['ue_control'] for r in catalog['bindings'] if r['enabled']}
animation={str(k.name) for k in h.get_controls() if h.get_control_settings(k).animation_type!=unreal.RigControlAnimationType.VISUAL_CUE}
cue=[str(k.name) for k in h.get_controls() if h.get_control_settings(k).animation_type==unreal.RigControlAnimationType.VISUAL_CUE]
report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),kind='name_coverage_only',asset=bp.get_path_name(),catalog_count=len(wanted),animation_controls=len(animation),visual_cues=cue,missing=sorted(wanted-animation),uncatalogued=sorted(animation-wanted),bones=len(h.get_bones()),nodes=len(bp.get_default_model().get_nodes()),curves=len(h.get_curves()))
report['passed']=not report['missing'] and not report['uncatalogued']
(ROOT/'Docs/Evidence/ControllerCatalog/ue_name_coverage.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
assert report['passed'],report
print(json.dumps(report,ensure_ascii=False))
