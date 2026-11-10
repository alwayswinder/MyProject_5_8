"""在新资产的正常预览动画帧检查 28 条 Morph，并帧后检查输入恢复。"""

# Shared package paths: reference inputs are read-only; reports go to the target project.
from face_paths import (tool_path as _fm_tool, legacy_saved_path as _fm_saved,
    project_root as _fm_project, read_path as _fm_read, output_path as _fm_output,
    asset_path as _fm_asset, asset_disk_path as _fm_disk,
    load_asset as _fm_load_asset, save_asset as _fm_save_asset,
    node_name as _fm_node, break_link as _fm_break_link)
import sys as _fm_sys
_fm_sys.dont_write_bytecode = True
import datetime
import json
from pathlib import Path
import traceback
import unreal

class RuntimeFacePreviewProbe:
    def __init__(self):
        self.root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
        self.out=_fm_tool('Docs/Evidence/SevenRuntime/preview_morph_check.json')
        self.target=unreal.load_asset(_fm_asset('/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset'))
        candidates=[]
        for comp in unreal.ObjectIterator(unreal.SkeletalMeshComponent):
            if comp.get_skinned_asset()!=self.target.get_editor_property('preview_skeletal_mesh') or 'AnimationEditorPreviewActor' not in comp.get_path_name():continue
            anim=comp.get_anim_instance()
            if not anim:continue
            for host in unreal.RigVMHost.find_rig_vm_hosts(anim,unreal.ControlRig):
                if host.get_vm().get_outer()==self.target and len(host.get_hierarchy().get_controls())==73:
                    candidates.append((comp,anim,host))
        if len(candidates)>1 and self.out.exists():
            previous=json.loads(_fm_read(self.out).read_text(encoding='utf-8'))
            candidates=[row for row in candidates if row[0].get_path_name()==previous.get('preview_component')]
        assert len(candidates)==1,str(candidates)
        self.comp,self.anim,self.live=candidates[0]
        self.h=self.live.get_hierarchy()
        self.saved={str(k.name):list(self.h.get_vector_from_control_value(self.h.get_control_value(k)).to_tuple()) for k in self.h.get_controls()}
        self.old_update=self.comp.get_editor_property('update_animation_in_editor')
        self.rows=[]
        for group in ('ExpressionSliders','PhonemeSliders','EyeExpressions'):
            self.rows.extend(json.loads((_fm_read(self.root/f'Docs/Evidence/{group}/parameters.json')).read_text(encoding='utf-8'))['rows'])
        self.expected={m['name']:.5*m.get('gain',1.0) for row in self.rows for m in row['morphs']}
        assert len(self.expected)==28
        self.phase='half';self.frames=0;self.result=dict(target=self.target.get_path_name(),pending=True,started_at=datetime.datetime.now().astimezone().isoformat(),preview_component=self.comp.get_path_name(),preview_host=self.live.get_path_name())

    def key(self,name):
        return unreal.RigElementKey(type=unreal.RigElementType.CONTROL,name=name)

    def write(self):
        _fm_output(self.out).write_text(json.dumps(self.result,ensure_ascii=False,indent=2),encoding='utf-8')

    def restore(self):
        for name,value in self.saved.items():
            self.h.set_control_value(self.key(name),self.h.make_control_value_from_vector(unreal.Vector(*value)))
        self.live.request_run_once_event('Forwards Solve')

    def diagnose_stall(self):
        """Preview stall has two layers: no evaluation, and no curve transfer to the mesh.

        Force one solve on the live host and record Rig-hierarchy curves vs anim curves so the
        report separates "solver did not run" from "mesh curves were not updated".
        """
        try:
            rig_after={name:self.h.get_curve_value(unreal.RigElementKey(type=unreal.RigElementType.CURVE,name=name)) for name in self.expected}
            self.live.execute('Forwards Solve')
            rig_solved={name:self.h.get_curve_value(unreal.RigElementKey(type=unreal.RigElementType.CURVE,name=name)) for name in self.expected}
            anim_after={name:self.anim.get_curve_value(name) for name in self.expected}
            return {'host_time':self.live.get_absolute_time(),
                    'rig_curves_before_execute_max':max(rig_after.values()),
                    'rig_curves_after_execute_max':max(rig_solved.values()),
                    'anim_curves_after_execute_max':max(anim_after.values()),
                    'sample':{name:rig_solved[name] for name in list(rig_solved)[:2]},
                    'verdict':('solver_runs_but_mesh_curves_stale'
                               if max(rig_solved.values())>1e-5 and max(anim_after.values())<1e-5
                               else 'solver_did_not_run' if max(rig_solved.values())<1e-5 else 'curves_transfer_partial')}
        except Exception as exc:
            return {'error':traceback.format_exc()[-400:]}

    def start(self):
        self.write()
        self.comp.set_update_animation_in_editor(True)
        for row in self.rows:
            self.h.set_control_value(self.key(row['control']),self.h.make_control_value_from_vector(unreal.Vector(4,0,0)))
        self.live.request_run_once_event('Forwards Solve')
        self.handle=unreal.register_slate_post_tick_callback(self.tick)

    def tick(self,delta):
        self.frames+=1
        if self.frames<25:return
        try:
            actual={name:self.anim.get_curve_value(name) for name in self.expected}
            if self.phase=='half':
                active={str(n) for n in self.anim.get_active_curve_names(unreal.AnimCurveType.MORPH_TARGET_CURVE)}
                self.result.update(expected_half=self.expected,actual_half=actual,
                                   half_error=max(abs(actual[n]-v) for n,v in self.expected.items()),
                                   missing_morph_type=sorted(set(self.expected)-active),
                                   rig_half={name:self.h.get_curve_value(unreal.RigElementKey(type=unreal.RigElementType.CURVE,name=name)) for name in self.expected},
                                   half_inputs={row['control']:list(self.h.get_vector_from_control_value(self.h.get_control_value(self.key(row['control']))).to_tuple()) for row in self.rows},
                                   rig_absolute_time=self.live.get_absolute_time())
                if self.result['half_error']>1e-5:
                    self.result['stall_diagnosis']=self.diagnose_stall()
                self.restore();self.phase='restored';self.frames=0;self.write();return
            restored={m['name']:max(0,min(8,self.saved[row['control']][0]))/8*m.get('gain',1.0) for row in self.rows for m in row['morphs']}
            err=max((self.h.get_vector_from_control_value(self.h.get_control_value(self.key(n)))-unreal.Vector(*v)).length() for n,v in self.saved.items())
            self.result.update(actual_restored=actual,expected_restored=restored,
                               restored_curve_error=max(abs(actual[n]-v) for n,v in restored.items()),
                               restored_input_error=err,saved_inputs=self.saved,pending=False,
                               captured_at=datetime.datetime.now().astimezone().isoformat())
            self.result['passed']=self.result['half_error']<1e-5 and not self.result['missing_morph_type'] and self.result['restored_curve_error']<1e-5 and err<1e-6
            unreal.unregister_slate_post_tick_callback(self.handle)
            self.comp.set_update_animation_in_editor(self.old_update)
            self.write()
        except Exception:
            self.restore()
            unreal.unregister_slate_post_tick_callback(self.handle)
            self.comp.set_update_animation_in_editor(self.old_update)
            self.result.update(pending=False,passed=False,error=traceback.format_exc())
            self.write()

_runtime_face_preview_probe=RuntimeFacePreviewProbe()
_runtime_face_preview_probe.start()
print('Scheduled normal-frame Morph preview and restoration checks')
