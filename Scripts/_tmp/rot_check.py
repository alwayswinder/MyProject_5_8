import json
import unreal
M = unreal.MathLibrary

def q(r):
    x = r.quaternion()
    return [round(x.x,3), round(x.y,3), round(x.z,3), round(x.w,3)]

out = {}
out['kwargs roll=-90'] = q(unreal.Rotator(roll=-90.0, pitch=0.0, yaw=0.0))
out['kwargs yaw=90'] = q(unreal.Rotator(roll=0.0, pitch=0.0, yaw=90.0))
out['kwargs pitch=-90'] = q(unreal.Rotator(roll=0.0, pitch=-90.0, yaw=0.0))
out['positional(0,0,-90)'] = q(unreal.Rotator(0,0,-90))
out['from quat cycle'] = q(unreal.Quat(0.5,0.5,0.5,0.5).rotator())
print('JSONSTART'); print(json.dumps(out, indent=1)); print('JSONEND')
