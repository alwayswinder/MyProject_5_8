"""新式 Seven Rig 的结构、全控制迁移对照和历史源样本验证。

历史验证器仅在内存中适配：目标用新资产，输出用独立目录；
原始验证脚本、历史采样及历史报告均不改写。
"""
import ast
import contextlib
import datetime
import io
import json
import math
from pathlib import Path
import traceback
import types
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = '/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation'
TARGET = '/Game/_MyTest/Mouth/TMp/NewControlRigRuntimeAsset'
E = ROOT / 'Docs/Evidence/SevenRuntime'

class RuntimeRig:
    def __init__(self, asset):
        self.asset = asset
        self.editor = asset.get_editor_asset()

    def get_hierarchy(self):
        return self.asset.hierarchy

    def create_control_rig(self):
        host = self.editor.create_rig_vm_host()
        assert isinstance(host, unreal.ControlRig), str(host)
        return host

    def get_editor_property(self, name):
        if name == 'status':
            raise RuntimeError('新版资产无 BlueprintStatus，应通过真实实例执行检查编译')
        return self.asset.get_editor_property(name)

    def __getattr__(self, name):
        return getattr(self.editor, name)

def instance(asset):
    inst = asset.create_control_rig()
    inst.request_init()
    h = inst.get_hierarchy()
    if not h.get_all_keys():
        h.copy_hierarchy(asset.get_hierarchy())
        inst.request_init()
    assert len(h.get_bones()) == 149 and len(h.get_controls()) == 73 and len(h.get_curves()) == 28
    return inst, h

def key(name, kind=unreal.RigElementType.BONE):
    return unreal.RigElementKey(type=kind, name=name)

def solve(inst, h, values):
    h.reset_pose_to_initial(unreal.RigElementType.CONTROL)
    for name, value in values.items():
        ck = key(name, unreal.RigElementType.CONTROL)
        assert h.contains(ck), name
        h.set_control_value(ck, h.make_control_value_from_vector(unreal.Vector(*value)))
    assert inst.execute('Forwards Solve')

def qerror(a, b):
    aa, bb = (a.x,a.y,a.z,a.w), (b.x,b.y,b.z,b.w)
    dot = abs(sum(x*y for x,y in zip(aa,bb))) / math.sqrt(sum(x*x for x in aa)*sum(x*x for x in bb))
    return math.degrees(2*math.acos(min(1,max(0,dot))))

def structural_and_transfer(phase):
    source = unreal.load_asset(SOURCE)
    target = RuntimeRig(unreal.load_asset(TARGET))
    sh, th = source.get_hierarchy(), target.get_hierarchy()
    sm, tm = source.get_default_model(), target.get_default_model()
    catalog = json.loads((ROOT/'Docs/Characters/Seven.face-controls.json').read_text(encoding='utf-8'))
    wanted = {r['ue_control'] for r in catalog['bindings'] if r['enabled']}
    actual = {str(k.name) for k in th.get_controls() if th.get_control_settings(k).animation_type != unreal.RigControlAnimationType.VISUAL_CUE}
    assert wanted == actual and len(actual) == 72
    links = lambda m: sorted((str(l.get_source_pin().get_pin_path()),str(l.get_target_pin().get_pin_path())) for l in m.get_links())
    assert links(sm) == links(tm)
    edges = [(str(l.get_source_node().get_name()),str(l.get_target_node().get_name())) for l in tm.get_links() if 'Execute' in str(l.get_target_pin().get_name())]
    adjacent = dict(edges)
    assert len(adjacent) == len(edges)
    starts=set(adjacent)-set(adjacent.values())
    assert len(starts)==1
    cursor=starts.pop();chain=[]
    while cursor in adjacent:
        assert cursor not in chain
        chain.append(cursor);cursor=adjacent[cursor]
    chain.append(cursor)
    assert len(chain)==len(edges)+1
    driven=[]
    for node in tm.get_nodes():
        if str(node.get_name()).startswith(('BROW_SET_','EYE_SET_','LIP_SET_','CHEEK_SET_','NOSE_SET_','GAZE_SET_','CHIN_SET_')):
            assert str(node.get_name()) in chain
            driven.append(node.find_pin('Item').get_default_value().split('Name="')[1].split('"')[0])
    assert len(driven)==len(set(driven))==57
    cases=[{}]
    coverage={}
    for name in sorted(actual):
        ck=key(name,unreal.RigElementType.CONTROL)
        st=th.get_control_settings(ck)
        assert st.control_type == unreal.RigControlType.POSITION
        assert th.get_vector_from_control_value(th.get_control_value(ck,unreal.RigControlValueType.INITIAL)).length()<1e-6
        assert (th.get_global_control_offset_transform(ck,True).scale3d-unreal.Vector(1,1,1)).length()<1e-6
        lo=th.get_vector_from_control_value(st.minimum_value).to_tuple()
        hi=th.get_vector_from_control_value(st.maximum_value).to_tuple()
        coverage[name]=0
        for axis in range(3):
            limit=st.limit_enabled[axis]
            low=lo[axis] if limit.minimum else -8
            high=hi[axis] if limit.maximum else 8
            for value in sorted(set((low,low*.5,0,high*.5,high,low-2,high+2))):
                v=[0,0,0];v[axis]=value;cases.append({name:v});coverage[name]+=1
    import random
    rng=random.Random(20261109)
    for i in range(32):
        values={}
        for name in sorted(actual):
            st=th.get_control_settings(key(name,unreal.RigElementType.CONTROL))
            lo=th.get_vector_from_control_value(st.minimum_value).to_tuple();hi=th.get_vector_from_control_value(st.maximum_value).to_tuple()
            values[name]=[rng.uniform(lo[a] if st.limit_enabled[a].minimum else -3,hi[a] if st.limit_enabled[a].maximum else 3) for a in range(3)]
        cases.append(values)
    si,srh=instance(source);ti,trh=instance(target)
    worst=dict(position_cm=0,rotation_deg=0,scale=0,curve=0,control_position_cm=0)
    bone_keys=th.get_bones();curve_keys=th.get_curves();control_keys=th.get_controls()
    for i,values in enumerate(cases):
        solve(si,srh,values);solve(ti,trh,values)
        for bk in bone_keys:
            a,b=srh.get_global_transform(bk),trh.get_global_transform(bk)
            worst['position_cm']=max(worst['position_cm'],(a.translation-b.translation).length())
            worst['rotation_deg']=max(worst['rotation_deg'],qerror(a.rotation,b.rotation))
            worst['scale']=max(worst['scale'],(a.scale3d-b.scale3d).length())
        for ck in curve_keys:worst['curve']=max(worst['curve'],abs(srh.get_curve_value(ck)-trh.get_curve_value(ck)))
        for ck in control_keys:worst['control_position_cm']=max(worst['control_position_cm'],(srh.get_global_transform(ck).translation-trh.get_global_transform(ck).translation).length())
    assert worst['position_cm']<1e-6 and worst['rotation_deg']<1e-4 and worst['scale']<1e-6 and worst['curve']<1e-6 and worst['control_position_cm']<1e-5,worst
    solve(ti,trh,{})
    curve_names={str(k.name) for k in curve_keys}
    mesh=target.asset.get_editor_property('preview_skeletal_mesh')
    morphs={str(m) for m in mesh.get_all_morph_target_names()}
    assert curve_names<=morphs,curve_names-morphs
    report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),target=TARGET,phase=phase,passed=True,
                animation_controls=72,visual_cues=1,bones=149,curves=28,nodes=len(tm.get_nodes()),links=len(tm.get_links()),
                execution_edges=len(edges),bone_writes=len(driven),cases=len(cases),coverage=coverage,worst=worst,
                curve_targets_exist=True,comparison='live legacy Seven reference; migration equivalence, not new Max capture',
                limits='compared with reference control values; role-specific verifiers assert functional limits')
    out=E/phase/'transfer_validation.json';out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    return report

class AdaptVerifier(ast.NodeTransformer):
    def visit_Call(self,node):
        self.generic_visit(node)
        if isinstance(node.func,ast.Attribute):
            if node.func.attr=='load_asset' and isinstance(node.func.value,ast.Name) and node.func.value.id=='unreal':
                node.func=ast.Name(id='_load_asset',ctx=ast.Load())
            elif node.func.attr=='write_text':
                node.func.value=ast.Call(func=ast.Name(id='_report_path',ctx=ast.Load()),args=[node.func.value],keywords=[])
            elif node.func.attr=='get_editor_property' and len(node.args)==1 and isinstance(node.args[0],ast.Constant) and node.args[0].value=='status':
                return ast.Constant(value='RuntimeAsset: fresh instance execution validated; no BlueprintStatus')
        return node

def legacy_verifiers(phase):
    files=['verify_lower_lids.py','verify_lip_controls.py','verify_cheek_controls.py','verify_nose_controls.py',
           'verify_mouth_controls.py','verify_eyeball_controls.py','verify_gaze_master.py','verify_chin_control.py',
           'verify_expression_sliders.py','verify_phoneme_sliders.py','verify_eye_expression_sliders.py','verify_nose_side_controls.py']
    results=[]
    for filename in files:
        outdir=E/phase/Path(filename).stem;outdir.mkdir(parents=True,exist_ok=True)
        def report_path(path):
            return outdir/path.name
        facade=RuntimeRig(unreal.load_asset(TARGET))
        def load_asset(path,*args,**kwargs):
            return facade if path in (SOURCE,TARGET) else unreal.load_asset(path,*args,**kwargs)
        module=types.ModuleType('runtime_test_'+Path(filename).stem)
        module.__dict__.update(_load_asset=load_asset,_report_path=report_path)
        tree=ast.fix_missing_locations(AdaptVerifier().visit(ast.parse((ROOT/'Scripts'/filename).read_text(encoding='utf-8-sig'))))
        started=datetime.datetime.now().astimezone().isoformat()
        output=io.StringIO()
        try:
            with contextlib.redirect_stdout(output):
                exec(compile(tree,filename,'exec'),module.__dict__)
                seen=set()
                def retarget(mod):
                    if id(mod) in seen or mod is unreal:return
                    seen.add(id(mod))
                    if getattr(mod,'ASSET',None)==SOURCE:mod.ASSET=TARGET
                    for value in list(vars(mod).values()):
                        if isinstance(value,types.ModuleType) and value.__name__ not in ('unreal','sys','os','json','math','types','datetime','importlib','shutil','pathlib'):
                            if any(k in vars(value) for k in ('ASSET','helpers','base','helper','b','v')):retarget(value)
                retarget(module)
                module.main()
            result=dict(script=filename,passed=True,started_at=started)
        except Exception:
            result=dict(script=filename,passed=False,started_at=started,error=traceback.format_exc())
        (outdir/'console.txt').write_text(output.getvalue(),encoding='utf-8')
        (outdir/'result.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
        results.append(result)
        print(json.dumps(result,ensure_ascii=False))
    (E/phase/'source_verifiers.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf-8')
    return results

def main(phase='before_reload'):
    E.mkdir(parents=True,exist_ok=True)
    transfer=structural_and_transfer(phase)
    source=legacy_verifiers(phase)
    report=dict(captured_at=datetime.datetime.now().astimezone().isoformat(),target=TARGET,phase=phase,
                passed=transfer['passed'] and all(r['passed'] for r in source),transfer_cases=transfer['cases'],
                source_verifiers_passed=sum(r['passed'] for r in source),source_verifiers_total=len(source))
    (E/phase/'summary.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps(report,ensure_ascii=False))
    return report

if __name__=='__main__':main()
