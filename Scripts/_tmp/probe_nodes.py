import json
import unreal
names = [n for n in dir(unreal) if n.startswith('RigVMFunction_MathVector') or n.startswith('RigVMFunction_MathTransform')]
out = {'vector': names,
       'has_getctrl': [n for n in dir(unreal) if 'GetControl' in n],
       'break_like': [n for n in dir(unreal) if 'Break' in n][:40]}
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
