"""拟合左右脸颊的正负分支和眼袋 X 门控项；普通 Python 运行。"""
import json
import math
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
EVIDENCE=ROOT/'Docs/Evidence/Cheeks'


def evaluate(params,inputs):
    result={b:[0,0,0] for b in params['bones']}
    for c,v in zip(params['controls'],inputs):
        for axis,ix in [('X',0),('Z',2)]:
            feature=axis+('P' if v[ix]>=0 else 'N')
            for b,d in c['features'][feature].items():
                result[b]=[a+abs(v[ix])*x for a,x in zip(result[b],d)]
        if v[0]>=c['gate_threshold_source_x']:
            amount=max(0,c['gate_z_sign']*v[2])
            for b,d in c['features']['GATE'].items():
                result[b]=[a+amount*x for a,x in zip(result[b],d)]
    return result


def main():
    if '--prepare' in sys.argv:
        names=json.loads((ROOT/'Docs/Evidence/LipControls/previous_controls.json').read_text(encoding='utf-8'))['neutral']
        folder=ROOT/'Saved/Codex'
        folder.mkdir(parents=True,exist_ok=True)
        (folder/'cheek_bone_names.txt').write_text('\n'.join(names),encoding='utf-8')
        print('Prepared %d bone names'%len(names))
        return
    raw=(ROOT/'Saved/Codex/cheek_probe_output.txt').read_text(encoding='utf-8-sig')
    ref=json.loads(raw[raw.index('{'):])
    assert not ref['missing'],ref['missing']
    baseline=ref['baseline']
    bones=[b for b,t in baseline.items() if any(math.dist(t['p'],s['bones'][b]['p'])>1e-5 for s in ref['samples'])]
    controls=[]
    def ue(v):
        return [v[0],-v[1],v[2]]
    for ci,src in enumerate(ref['controls']):
        c=dict(control=['CTRL_CHEEK_R','CTRL_CHEEK_L'][ci],source=src['source'],
               display_name=['右脸颊','左脸颊'][ci],control_rest=ue(src['rest']['p']),
               control_rotation=[src['rest']['q'][0],-src['rest']['q'][1],src['rest']['q'][2],src['rest']['q'][3]],
               source_basis_x=ue(src['basis_x']),source_basis_z=ue(src['basis_z']),
               gate_threshold_source_x=-src['frozen_input'][0],gate_z_sign=-1 if ci==0 else 1,features={})
        # Flip the blue handle axis; compensate its input so physical drags
        # continue to produce the already validated deformation.
        q=c['control_rotation']
        c['source_control_rotation']=q[:]
        c['control_rotation']=[q[3],q[2],-q[1],-q[0]]
        c['input_sign']=[1,1,-1]
        for axis,ix in [('X',0),('Z',2)]:
            for branch,sign in [('P',1),('N',-1)]:
                sample=next(s for s in ref['samples'] if s['input'][ci][ix]==sign and sum(abs(v) for p in s['input'] for v in p)==1)
                c['features'][axis+branch]={}
                for b in bones:
                    d=ue([a-v for a,v in zip(sample['bones'][b]['p'],baseline[b]['p'])])
                    if math.dist(d,[0,0,0])>1e-5:
                        c['features'][axis+branch][b]=d
        # Z affects the lower eye bag only in the nonnegative-X branch.
        eye='yandai2_'+('R' if ci==0 else 'L')
        gate_feature='ZN' if ci==0 else 'ZP'
        if ci==0:
            sample=next(s for s in ref['samples'] if s['input'][0]==[1,0,-1] and s['input'][1]==[-1,0,1])
            total=ue([a-v for a,v in zip(sample['bones'][eye]['p'],baseline[eye]['p'])])
            gate=[a-v for a,v in zip(total,c['features']['XP'][eye])]
        else:
            gate=c['features'][gate_feature][eye]
        c['features'][gate_feature].pop(eye,None)
        c['features']['GATE']={eye:gate}
        controls.append(c)
    params=dict(controls=controls,bones=bones,source_to_ue='position (x,-y,z), quaternion (x,-y,z,w)')
    worst=0.0
    for s in ref['samples']:
        assert not s['morph_changes'],s['morph_changes']
        delta=evaluate(params,s['input'])
        for b,t in s['bones'].items():
            expected=[a+d for a,d in zip(ue(baseline[b]['p']),delta.get(b,[0,0,0]))]
            worst=max(worst,math.dist(expected,ue(t['p'])))
    assert worst<0.001,('Piecewise fit failed',worst)
    params.update(sample_cases=len(ref['samples']),max_fit_position_error_cm=worst)
    ref['non_target_max_position_change_cm']=max(math.dist(t['p'],s['bones'][b]['p']) for s in ref['samples'] for b,t in baseline.items() if b not in bones)
    for s in ref['samples']:
        s['bones']={b:s['bones'][b] for b in bones}
    ref['baseline']={b:baseline[b] for b in bones}
    EVIDENCE.mkdir(exist_ok=True)
    (EVIDENCE/'max_samples.json').write_text(json.dumps(ref,ensure_ascii=False,indent=2),encoding='utf-8')
    (EVIDENCE/'parameters.json').write_text(json.dumps(params,ensure_ascii=False,indent=2),encoding='utf-8')
    print(dict(bones=bones,sample_cases=len(ref['samples']),max_fit_position_error_cm=worst,
               non_target_max_position_change_cm=ref['non_target_max_position_change_cm']))


if __name__=='__main__':
    main()
