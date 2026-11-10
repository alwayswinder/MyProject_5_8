"""增量添加鼻子控制：世界轴操作，仅驱动 bijian，保留现有 38 控制。"""
import datetime
import json
from pathlib import Path
import shutil
import types
import unreal

ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
EVIDENCE=ROOT/'Docs/Evidence/Nose'
base=types.ModuleType('lip_helpers')
exec(compile((ROOT/'Scripts/build_lip_controls.py').read_text(encoding='utf-8'),'build_lip_controls.py','exec'),base.__dict__)
ASSET = '/Game/_MyTest/Mouth/Rig/NewControlRigRuntimeAsset'
key,vstr,instance,solve,pose,packed=base.key,base.vstr,base.instance,base.solve,base.pose,base.packed
PREFIX='NOSE_'


def main():
    params=json.loads((EVIDENCE/'parameters.json').read_text(encoding='utf-8'))
    rig=__compat.load(ASSET)
    h,hc=rig.get_hierarchy(),rig.get_hierarchy_controller()
    model=rig.get_default_model()
    ctrl=rig.get_controller(model)
    assert not any(str(n.get_name()).startswith(PREFIX) for n in model.get_nodes()),'Nose already exists'
    assert h.contains(key('bijian'))
    nodes_before=len(model.get_nodes())
    old_exec=base.helper.exec_links(model)
    tails={b.split('.')[0] for a,b in old_exec}-{a.split('.')[0] for a,b in old_exec}
    assert len(tails)==1,tails
    tail=tails.pop()+'.ExecutePin'
    backup=ROOT/'Saved/Codex/Backup'/('Nose_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S'))
    backup.mkdir(parents=True)
    disk=ROOT/'Content/_MyTest/Mouth/Rig/NewControlRigRuntimeAsset.uasset'
    shutil.copy2(disk,backup/'disk_before.uasset')
    assert rig.save()
    shutil.copy2(disk,backup/'memory_before.uasset')
    cases=[x['values'] for x in json.loads((ROOT/'Docs/Evidence/Cheeks/previous_controls.json').read_text(encoding='utf-8'))['cases']]
    for name in ('CTRL_CHEEK_R','CTRL_CHEEK_L'):
        cases.extend({name:v} for v in [(-10,0,-10),(-10,0,10),(10,0,-10),(10,0,10),(-10,0,0),(10,0,0),(0,0,-10),(0,0,10)])
    bi,bh=instance(rig)
    previous=[]
    for values in cases:
        solve(bi,bh,values)
        previous.append(dict(values=values,bones=packed(bh)))
    neutral=previous[0]['bones']
    captured=dict(neutral=neutral,cases=[dict(values=x['values'],bones={b:t for b,t in x['bones'].items() if t!=neutral[b]}) for x in previous])
    (EVIDENCE/'previous_controls.json').write_text(json.dumps(captured,ensure_ascii=False,indent=2),encoding='utf-8')
    report=dict(asset=ASSET,backup=str(backup),nodes_before=nodes_before,old_execution_links=old_exec,
                regression_cases=len(cases),saved=False)
    (EVIDENCE/'build_report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    def unit(cls,name,x,y):
        n=ctrl.add_unit_node_from_struct_path(getattr(unreal,cls).static_struct().get_path_name(),'Execute',unreal.Vector2D(x,y),PREFIX+name,False)
        assert n is not None,name
        return str(n.get_name())
    def default(pin,value):
        assert ctrl.set_pin_default_value(pin,value,False,False),pin
    def link(a,b):
        assert ctrl.add_link(a,b,False),(a,b)
    auto=rig.get_auto_vm_recompile()
    rig.modify()
    try:
        rig.set_auto_vm_recompile(False)
        with unreal.ScopedEditorTransaction('Add Max nose control'):
            settings=unreal.RigControlSettings()
            settings.control_type=unreal.RigControlType.POSITION
            settings.display_name=params['display_name']
            settings.shape_name='Box_Thick'
            settings.shape_color=unreal.LinearColor(0.3,1.0,0.55,1)
            settings.primary_axis=unreal.RigControlAxis.Y
            settings.limit_enabled=[unreal.RigControlLimitEnabled(minimum=True,maximum=True)]*3
            settings.minimum_value=h.make_control_value_from_vector(unreal.Vector(*params['minimum']))
            settings.maximum_value=h.make_control_value_from_vector(unreal.Vector(*params['maximum']))
            settings.draw_limits=False
            ck=hc.add_control(params['control'],key('Root'),settings,h.make_control_value_from_vector(unreal.Vector()),False,False)
            assert h.contains(ck)
            offset=unreal.Transform()
            offset.translation=unreal.Vector(*params['control_rest'])
            for initial in (True,False):
                h.set_control_offset_transform(ck,offset,initial,True)
                shape=unreal.Transform()
                shape.scale3d=unreal.Vector(0.13,0.08,0.13)
                h.set_control_shape_transform(ck,shape,initial)
            get=unit('RigUnit_GetControlVector','GET',200,-80000)
            default(get+'.Control',params['control'])
            default(get+'.Space','LocalSpace')
            clamp=unit('RigVMFunction_MathVectorClamp','CLAMP',440,-80000)
            link(get+'.Vector',clamp+'.Value')
            default(clamp+'.Minimum',vstr(params['minimum']))
            default(clamp+'.Maximum',vstr(params['maximum']))
            scale=unit('RigVMFunction_MathVectorScale','DELTA',680,-80000)
            link(clamp+'.Result',scale+'.Value')
            default(scale+'.Factor',str(params['gain']))
            bt=unit('RigUnit_GetTransform','BT_bijian',680,-80300)
            default(bt+'.Item','(Type=Bone,Name="bijian")')
            default(bt+'.Space','GlobalSpace')
            default(bt+'.bInitial','True')
            add=unit('RigVMFunction_MathVectorAdd','ADD_bijian',920,-80000)
            link(bt+'.Transform.Translation',add+'.A')
            link(scale+'.Result',add+'.B')
            make=unit('RigVMFunction_MathTransformMake','MAKE_bijian',1160,-80000)
            link(add+'.Result',make+'.Translation')
            link(bt+'.Transform.Rotation',make+'.Rotation')
            link(bt+'.Transform.Scale3D',make+'.Scale')
            setter=unit('RigUnit_SetTransform','SET_bijian',1400,-80000)
            default(setter+'.Item','(Type=Bone,Name="bijian")')
            default(setter+'.Space','GlobalSpace')
            default(setter+'.bInitial','False')
            default(setter+'.Weight','1.0')
            default(setter+'.bPropagateToChildren','False')
            link(make+'.Result',setter+'.Value')
            for output in {a for a,b in base.helper.exec_links(model)}|{a for a,b in old_exec}:
                ctrl.break_all_links(output,False,False,False)
            for a,b in old_exec:
                link(a,b)
            link(tail,setter+'.ExecutePin')
            assert sorted(base.helper.exec_links(model))==sorted(old_exec+[(tail,setter+'.ExecutePin')])
            rig.recompile_vm()
            assert __compat.status_text(rig)!=unreal.BlueprintStatus.BS_ERROR
    finally:
        rig.set_auto_vm_recompile(auto)
    module=types.ModuleType('nose_reload')
    pass
    None


if __name__=='__main__':
    main()
