"""相同节点结构下同步分段系数和旋转基，不改变手柄布局；完整复核/重载。"""
import json,types
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
m=types.ModuleType('mouth_build')
exec(compile((ROOT/'Scripts/build_mouth_controls.py').read_text(encoding='utf-8'),'build_mouth_controls.py','exec'),m.__dict__)


def main():
    p=json.loads((m.EVIDENCE/'parameters.json').read_text(encoding='utf-8'))
    rig=unreal.load_asset(m.ASSET);model=rig.get_default_model();ctrl=rig.get_controller(model)
    assert len(model.get_nodes())==1226
    auto=rig.get_auto_vm_recompile()
    try:
        rig.set_auto_vm_recompile(False)
        for b,d in p['bones'].items():
            if d['rotating']:
                assert d['euler_order']=='xyz'
                basis=d['basis'];inverse=[-v for v in basis[:3]]+[basis[3]]
                for pin,value in (('MOUTH_QB_'+b+'.A',basis),('MOUTH_QD_'+b+'.B',inverse)):
                    qstr='(X=%.12f,Y=%.12f,Z=%.12f,W=%.12f)'%tuple(value)
                    assert ctrl.set_pin_default_value(pin,qstr,False,False)
            for ti,t in enumerate(d['terms']):
                for channel,vec in (('P',t['p']),('E',t['e'])):
                    n='MOUTH_'+channel+'S_'+b+'_'+str(ti)
                    expected=sum(v*v for v in vec)>=(1e-10 if channel=='P' else 1e-8)
                    assert bool(model.find_node_by_name(n))==expected,n
                    if not expected:continue
                    if channel=='E':vec=[v*d['euler_sign'] for v in vec]
                    assert ctrl.set_pin_default_value(n+'.Value',m.vstr(vec),False,False)
        rig.recompile_vm()
    finally:rig.set_auto_vm_recompile(auto)
    reload=types.ModuleType('mouth_reload')
    exec(compile((ROOT/'Scripts/reload_mouth_target.py').read_text(encoding='utf-8'),'reload_mouth_target.py','exec'),reload.__dict__)
    reload.main()


if __name__=='__main__':main()
