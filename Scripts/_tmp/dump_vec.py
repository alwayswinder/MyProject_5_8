import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
model = rig.get_default_model()
out = {}
for n in model.get_nodes():
    if n.get_name() not in ('BRW_VADD_meibu_L', 'BRW_VADD_meibu_R', 'Add', 'Add_1', 'BRW_SET_meibu_L'):
        continue
    rows = []
    for p in n.get_pins():
        nm = str(p).split('.')[-1]
        links = [str(l).split('RigVMLink_')[-1][:4] for l in (p.get_links() or [])]
        subs = []
        try:
            for s in (p.get_sub_pins() or []):
                sn = str(s).split('.')[-1]
                sl = [str(l).split('RigVMLink_')[-1][:4] for l in (s.get_links() or [])]
                if sl:
                    subs.append([sn, sl])
        except Exception:
            pass
        if links or subs:
            rows.append({'pin': nm, 'links': links, 'sub_links': subs})
    out[n.get_name()] = rows
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
