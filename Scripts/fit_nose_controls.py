"""确认鼻子三轴为线性平移，并生成世界轴操作映射。普通 Python 运行。"""
import json
import math
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
EVIDENCE=ROOT/'Docs/Evidence/Nose'


def main():
    raw=(ROOT/'Saved/Codex/nose_probe_output.txt').read_text(encoding='utf-8-sig')
    ref=json.loads(raw[raw.index('{'):])
    baseline=ref['baseline']
    moving=[]
    max_source_rotation=max_source_scale=0.0
    for b,rest in baseline.items():
        max_p=0.0
        for sample in ref['samples']:
            t=sample['bones'][b]
            max_p=max(max_p,math.dist(t['p'],rest['p']))
            dot=abs(sum(a*v for a,v in zip(t['q'],rest['q'])))/math.sqrt(sum(a*a for a in t['q'])*sum(a*a for a in rest['q']))
            max_source_rotation=max(max_source_rotation,math.degrees(2*math.acos(min(1,dot))))
            max_source_scale=max(max_source_scale,math.dist(t['scale'],rest['scale']))
        if max_p>1e-5:
            moving.append(b)
    assert moving==['bijian'],moving
    assert max_source_rotation<1e-4 and max_source_scale<1e-6
    worst=0.0
    for sample in ref['samples']:
        assert not sample['morph_changes'],sample['morph_changes']
        x,y,z=sample['input']
        for b,rest in baseline.items():
            delta=[-z,-x,y] if b=='bijian' else [0,0,0]
            expected=[a+d for a,d in zip(rest['p'],delta)]
            worst=max(worst,math.dist(expected,sample['bones'][b]['p']))
    assert worst<0.001,worst
    ref['non_target_max_position_change_cm']=max(math.dist(rest['p'],s['bones'][b]['p']) for b,rest in baseline.items() if b not in moving for s in ref['samples'])
    for s in ref['samples']:
        s['bones']={b:s['bones'][b] for b in moving}
    ref['baseline']={b:baseline[b] for b in moving}
    p=ref['control_rest']['p']
    params=dict(control='CTRL_NOSE',display_name='鼻子',source=ref['source'],bones=moving,
                control_rest=[p[0],-p[1],p[2]],control_rotation=[0,0,0,1],
                minimum=[-10,0,-10],maximum=[10,10,10],gain=0.1,
                source_to_ue_input='UE=( -Max.Z, Max.X, Max.Y ) * 10',
                max_fit_position_error_cm=worst,max_source_rotation_change_deg=max_source_rotation,
                max_source_scale_change=max_source_scale,sample_cases=len(ref['samples']))
    EVIDENCE.mkdir(exist_ok=True)
    (EVIDENCE/'max_samples.json').write_text(json.dumps(ref,ensure_ascii=False,indent=2),encoding='utf-8')
    (EVIDENCE/'parameters.json').write_text(json.dumps(params,ensure_ascii=False,indent=2),encoding='utf-8')
    print(params)


if __name__=='__main__':
    main()
