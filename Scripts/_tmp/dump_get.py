import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
model = rig.get_default_model()
out = {}
for n in model.get_nodes():
    nm = n.get_name()
    if nm not in ('BRW_GET', 'BRW_TILT_L', 'BRW_TILT_R', 'BRW_TILT_L_1'):
        continue
    rows = []
    for p in n.get_pins():
        pname = str(p).split('.')[-1]
        try:
            subs = p.get_sub_pins() or []
        except Exception:
            subs = []
        for s in subs:
            sl = [str(l).split('RigVMLink_')[-1][:3] for l in (s.get_links() or [])]
            if sl:
                rows.append([pname + '.' + str(s).split('.')[-1], sl])
        links = [str(l).split('RigVMLink_')[-1][:3] for l in (p.get_links() or [])]
        if links:
            rows.append([pname, links])
    out[nm] = rows
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
