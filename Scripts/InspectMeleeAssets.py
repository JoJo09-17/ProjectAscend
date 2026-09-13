import unreal
import json
from pathlib import Path

result = {}
for name in ['BP_AscendPlayerCharacter', 'BP_AscendEnemy']:
    cls = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/' + name)
    cdo = unreal.get_default_object(cls)
    mesh = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    sk = mesh.get_editor_property('skeletal_mesh_asset')
    anim_class = mesh.get_editor_property('anim_class')
    result[name] = {'mesh': sk.get_path_name(), 'anim_class': str(anim_class), 'skeleton': sk.skeleton.get_path_name(), 'relative_rotation': str(mesh.get_editor_property('relative_rotation')), 'ability_set': str(cdo.get_editor_property('ability_set')), 'bones': [str(mesh.get_bone_name(i)) for i in range(mesh.get_num_bones())], 'sockets': [str(s) for s in mesh.get_all_socket_names()]}
result['retarget_api'] = unreal.IKRetargetBatchOperation.duplicate_and_retarget.__doc__
result['rig_api'] = {n: getattr(unreal.IKRigController, n).__doc__ for n in dir(unreal.IKRigController) if any(t in n for t in ['auto', 'retarget_chain', 'skeletal_mesh', 'retarget_root'])}
result['retarget_controller'] = {n: getattr(unreal.IKRetargeterController, n).__doc__ for n in dir(unreal.IKRetargeterController) if any(t in n for t in ['auto', 'retarget_op', 'ik_rig', 'preview_mesh', 'chain_mapping', 'retarget_pose'])}
Path(unreal.Paths.project_saved_dir(), 'MeleeAssetInspection.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('MELEE_INSPECTION_COMPLETE')
