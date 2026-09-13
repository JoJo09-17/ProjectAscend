"""Retarget UAL2 onto the project's Manny and UE4 character skeletons."""
import unreal
import json
import traceback
from pathlib import Path

ROOT = '/Game/Combat/Melee'
SOURCE_ROOT = '/Game/ThirdParty/Quaternius/UAL2'
tools = unreal.AssetToolsHelpers.get_asset_tools()
registry = unreal.AssetRegistryHelpers.get_asset_registry()
report = {}


def make_rig(name, mesh, spine_end, neck_end, head):
    path = ROOT + '/Rigs/' + name
    rig = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else tools.create_asset(name, ROOT + '/Rigs', unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    controller = unreal.IKRigController.get_controller(rig)
    assert controller.set_skeletal_mesh(mesh)
    for chain in controller.get_retarget_chains():
        controller.remove_retarget_chain(chain.chain_name)
    assert controller.set_retarget_root('pelvis')
    chains = [('Root', 'root', 'root'), ('Spine', 'spine_01', spine_end), ('Neck', 'neck_01', neck_end), ('Head', head, head)]
    for side, suffix in [('Left', 'l'), ('Right', 'r')]:
        chains += [(side+'Clavicle', 'clavicle_'+suffix, 'clavicle_'+suffix), (side+'Arm', 'upperarm_'+suffix, 'hand_'+suffix), (side+'Leg', 'thigh_'+suffix, 'foot_'+suffix), (side+'Toe', 'ball_'+suffix, 'ball_'+suffix)]
        chains += [(side+finger.title(), finger+'_01_'+suffix, finger+'_03_'+suffix) for finger in ['thumb','index','middle','ring','pinky']]
    for chain, start, end in chains:
        controller.add_retarget_chain(chain, start, end, 'None')
    unreal.EditorAssetLibrary.save_loaded_asset(rig)
    return rig


try:
    source = unreal.load_asset(SOURCE_ROOT + '/InPlace/SkeletalMeshes/UAL2_Standard')
    source_rig = make_rig('IKR_UAL2', source, 'spine_03', 'neck_01', 'Head')
    for label, mesh_path, spine, neck in [
        ('Manny', '/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple', 'spine_05', 'neck_02'),
        ('UE4', '/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin', 'spine_03', 'neck_01')]:
        target = unreal.load_asset(mesh_path)
        target_rig = make_rig('IKR_'+label, target, spine, neck, 'head')
        name = 'RTG_UAL2_'+label
        path = ROOT+'/Rigs/'+name
        retargeter = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else tools.create_asset(name, ROOT+'/Rigs', unreal.IKRetargeter, unreal.IKRetargetFactory())
        c = unreal.IKRetargeterController.get_controller(retargeter)
        c.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
        c.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_rig)
        # UE 5.7 does not propagate SetIKRig(Target) into the operation stack.
        # Without these assignments the batch export succeeds but FK/IK ops can
        # evaluate against a stale or empty target rig and produce broken poses.
        c.assign_ik_rig_to_all_ops(unreal.RetargetSourceOrTarget.SOURCE, source_rig)
        c.assign_ik_rig_to_all_ops(unreal.RetargetSourceOrTarget.TARGET, target_rig)
        c.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
        c.reset_retarget_pose('Default Pose', [], unreal.RetargetSourceOrTarget.TARGET)
        c.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET)
        unreal.EditorAssetLibrary.save_loaded_asset(retargeter)
        report[label] = []
        for variant in ['InPlace', 'RootMotion']:
            originals = [a for a in registry.get_assets_by_path(SOURCE_ROOT+'/'+variant, recursive=True) if str(a.asset_class_path.asset_name) == 'AnimSequence']
            outputs = unreal.IKRetargetBatchOperation.duplicate_and_retarget(originals, source, target, retargeter, prefix=label+'_', include_referenced_assets=False, overwrite_existing_files=True)
            assert len(outputs) == len(originals), (label, variant, len(outputs))
            for data in outputs:
                asset = data.get_asset()
                destination = ROOT+'/'+label+'/'+variant+'/'+asset.get_name()
                assert unreal.EditorAssetLibrary.rename_asset(asset.get_path_name(), destination)
                assert asset.get_editor_property('skeleton') == target.skeleton
                assert unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
                report[label].append(destination)
    report['status'] = 'complete'
except Exception:
    report['status'] = 'failed'
    report['error'] = traceback.format_exc()
    unreal.log_error(report['error'])
Path(unreal.Paths.project_saved_dir(), 'MeleeRetargetReport.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('MELEE_RETARGET_' + report['status'])
