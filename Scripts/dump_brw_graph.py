"""Dump the BRW graph (nodes, pins, links, default values)."""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    ctrl = rig.get_controller(rig.get_default_model())
    out = []
    for node in rig.get_default_model().get_nodes():
        name = node.get_name()
        if not (name.startswith('BRW_') or name in ('GetTransform', 'GetTransform_1', 'Add', 'Add_1', 'Begin')):
            continue
        item = {'node': name, 'class': node.get_class().get_name(), 'pins': []}
        for p in node.get_pins():
            try:
                pn = p.get_name()
            except AttributeError:
                continue
            rec = {'pin': pn}
            try:
                rec['type'] = p.get_cpp_type().get_name()
            except Exception:
                pass
            try:
                val = ctrl.get_pin_default_value('%s.%s' % (name, pn), False)
                rec['default'] = val
            except Exception as exc:
                rec['default_err'] = repr(exc)[:120]
            try:
                links = []
                for l in p.get_links():
                    links.append('%s.%s' % (l.get_outer_node().get_name() if l.get_outer_node() else '?', l.get_name()))
                if links:
                    rec['links'] = links
            except Exception as exc:
                rec['links_err'] = repr(exc)[:100]
            item['pins'].append(rec)
        out.append(item)
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
