"""Shared space conversion for reference panel drawing on head-following rigs."""
import unreal

def attach_head_relative_rails(editor,head_name):
 graph=editor.get_default_model();controller=editor.get_controller(graph)
 rails=[n for n in graph.get_nodes() if str(n.get_name()).endswith('_RAIL')]
 assert rails and all(n.find_pin('WorldOffset') for n in rails)
 assert not graph.find_node_by_name('DESIGN_HEAD_DELTA'),'Head drawing space already installed'
 def node(path,name):
  result=controller.add_unit_node_from_struct_path(path,position=unreal.Vector2D(-4000,-1000),node_name=name,setup_undo_redo=False)
  assert result
  return str(result.get_name())
 def default(pin,value):assert controller.set_pin_default_value(pin,str(value),True,False)
 def link(a,b):assert controller.add_link(a,b,False,create_cast_node=True)
 for initial,label in ((True,'INITIAL'),(False,'CURRENT')):
  n=node('/Script/ControlRig.RigUnit_GetTransform','DESIGN_HEAD_'+label)
  default(n+'.Item.Type','Bone');default(n+'.Item.Name',head_name);default(n+'.Space','GlobalSpace');default(n+'.bInitial','True' if initial else 'False')
 inv=node('/Script/RigVM.RigVMFunction_MathTransformInverse','DESIGN_HEAD_INV_INITIAL');link('DESIGN_HEAD_INITIAL.Transform',inv+'.Value')
 delta=node('/Script/RigVM.RigVMFunction_MathTransformMul','DESIGN_HEAD_DELTA');link(inv+'.Result',delta+'.A');link('DESIGN_HEAD_CURRENT.Transform',delta+'.B')
 for rail in rails:link(delta+'.Result',str(rail.get_name())+'.WorldOffset')
 return len(rails)
