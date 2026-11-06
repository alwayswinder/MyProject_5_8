"""从 Max 临时采样拟合六个唇控的全局平移增量；在普通 Python 运行。"""
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'Docs/Evidence/LipControls'


def main():
    raw = (ROOT / 'Saved/Codex/lip_probe_output.txt').read_text(encoding='utf-8-sig')
    ref = json.loads(raw[raw.index('{'):])
    (OUT / 'max_samples.json').write_text(json.dumps(ref, ensure_ascii=False, indent=2), encoding='utf-8')
    names = ['CTRL_LIP_UP_L', 'CTRL_LIP_UP_C', 'CTRL_LIP_UP_R',
             'CTRL_LIP_LOW_R', 'CTRL_LIP_LOW_C', 'CTRL_LIP_LOW_L']
    controls = []
    for name, c in zip(names, ref['controls']):
        delta = {}
        for b, rest in ref['baseline'].items():
            d = [v-r for v, r in zip(c['samples'][-1]['bones'][b]['p'], rest['p'])]
            if math.dist(d, [0, 0, 0]) > 1e-5:
                delta[b] = [d[0], -d[1], d[2]]
        p = c['rest']['p']
        controls.append(dict(control=name, source=c['source'], sign=c['sign'],
                             control_rest=[p[0], -p[1], p[2]], delta_full=delta))
    cases = [(sample, [sample['s'] if i == j else 0 for i in range(6)])
             for j, c in enumerate(ref['controls']) for sample in c['samples']]
    cases += [(s, s['s']) for s in ref['combinations']]
    residual = 0.0
    for sample, weights in cases:
        assert not sample['morph_changes'], sample['morph_changes']
        for b, rest in ref['baseline'].items():
            expected = [rest['p'][0], -rest['p'][1], rest['p'][2]]
            for c, s in zip(controls, weights):
                expected = [a+s*d for a, d in zip(expected, c['delta_full'].get(b, [0, 0, 0]))]
            got = sample['bones'][b]['p']
            residual = max(residual, math.dist(expected, [got[0], -got[1], got[2]]))
    assert residual < 0.001, residual
    params = dict(controls=controls, bones=list(ref['baseline']),
                  source_to_ue='position (x,-y,z), quaternion (x,-y,z,w)',
                  max_fit_position_error_cm=residual, sample_cases=len(cases),
                  morph_channels_checked=ref['morph_channels_checked'], morph_changes=0)
    (OUT / 'parameters.json').write_text(json.dumps(params, ensure_ascii=False, indent=2), encoding='utf-8')
    print({k:v for k,v in params.items() if k not in ('controls', 'bones')})


if __name__ == '__main__':
    main()
