"""只读确认预览中实际创建了参考图 ShapeActor，未使用截图验收。"""
import json,unreal
from pathlib import Path
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
E=ROOT/'Docs/Evidence/PhonemeReference'
folder='/Game/_MyTest/Mouth/LidDiagnostic/PhonemeReference'
parameters=json.loads((E/'parameters.json').read_text(encoding='utf-8'))
matches=[]
for obj in unreal.ObjectIterator(unreal.ControlRigShapeActor):
    comp=obj.get_component_by_class(unreal.StaticMeshComponent)
    if comp and comp.static_mesh and comp.static_mesh.get_path_name()==folder+'/SM_PhonemeReference.SM_PhonemeReference':
        tf=obj.get_actor_transform();mat=comp.get_material(0).get_base_material()
        assert not obj.is_hidden_ed() and comp.get_editor_property('visible')
        assert (tf.translation-unreal.Vector(*parameters['origin'])).length()<1e-5
        assert mat.get_path_name()==folder+'/M_PhonemeReference.M_PhonemeReference'
        matches.append(dict(actor=obj.get_path_name(),position=list(tf.translation.to_tuple()),material=mat.get_path_name(),visible=True))
assert matches,'Reference shape actor missing from preview'
report=dict(passed=True,actors=matches,visual_approval='pending_user')
(E/'preview_binding.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
