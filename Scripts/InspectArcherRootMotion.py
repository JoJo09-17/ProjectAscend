import unreal,json
from pathlib import Path
out={}
for side,base in [('source','/Game/ArtAsset/Animations/Archer/Animation/Sequence/02_Attack/05_Combo_Attack_05/AS_Combo_Attack_05_'),('target','/Game/Combat/Ranged/Manny/Manny_Archer_AS_Combo_Attack_05_')]:
 for i in range(1,5):
  a=unreal.load_asset(base+str(i).zfill(2)+'_Seq')
  a.set_editor_property('enable_root_motion',False);a.set_editor_property('force_root_lock',False)
  rows=[]
  for t in [0,.3,.7,1.5,a.get_play_length()]:
   pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,min(t,a.get_play_length()),unreal.AnimPoseEvaluationOptions())
   row={'time':t}
   for bone in ['root','pelvis']:
    p=unreal.AnimPoseExtensions.get_bone_pose(pose,bone,unreal.AnimPoseSpaces.WORLD).translation
    row[bone]=[p.x,p.y,p.z]
   rows.append(row)
  out[side+str(i)]=rows
Path(unreal.Paths.project_saved_dir(),'ArcherRootInspection.json').write_text(json.dumps(out,indent=2))
unreal.log('ROOT_INSPECTION_COMPLETE')
