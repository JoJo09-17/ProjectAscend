"""Inspect IK retargeter operation stacks and current mappings."""
import json
from pathlib import Path
import unreal

report = {}
for rig_name in ['UAL2', 'Manny', 'UE4']:
    rig = unreal.load_asset('/Game/Combat/Melee/Rigs/IKR_' + rig_name)
    rig_controller = unreal.IKRigController.get_controller(rig)
    report['rig_' + rig_name] = {
        'root': str(rig_controller.get_retarget_root()),
        'chains': [{
            'name': str(chain.chain_name),
            'start': str(rig_controller.get_retarget_chain_start_bone(chain.chain_name)),
            'end': str(rig_controller.get_retarget_chain_end_bone(chain.chain_name)),
        } for chain in rig_controller.get_retarget_chains()],
    }
for label in ['Manny', 'UE4']:
    retargeter = unreal.load_asset('/Game/Combat/Melee/Rigs/RTG_UAL2_' + label)
    controller = unreal.IKRetargeterController.get_controller(retargeter)
    report[label] = {
        'op_count': controller.get_num_retarget_ops(),
        'ops': [str(controller.get_op_name(i)) for i in range(controller.get_num_retarget_ops())],
        'op_target_rigs': [
            str(controller.get_target_ik_rig_for_op(controller.get_op_name(i)))
            for i in range(controller.get_num_retarget_ops())
        ],
        'source_pose': str(controller.get_current_retarget_pose_name(unreal.RetargetSourceOrTarget.SOURCE)),
        'target_pose': str(controller.get_current_retarget_pose_name(unreal.RetargetSourceOrTarget.TARGET)),
    }

Path(unreal.Paths.project_saved_dir(), 'RetargeterState.json').write_text(
    json.dumps(report, indent=2), encoding='utf-8')
unreal.log('RETARGETER_STATE_COMPLETE ' + json.dumps(report))
