import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
hc = rig.get_hierarchy_controller()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_R')
board = unreal.RigElementKey(type=unreal.RigElementType.NULL, name='BrowBoard')
with unreal.ScopedEditorTransaction('Parent CTRL_BROW_R to BrowBoard'):
    ok = hc.set_parent(k, board, True, False, False)
print('set_parent:', ok)
p = h.get_first_parent(k)
print('parent now:', str(p.name) if p else None)
off = h.get_global_control_offset_transform(k)
print('offset:', off.translation)
unreal.EditorAssetLibrary.save_loaded_asset(rig, False)
print('saved')
