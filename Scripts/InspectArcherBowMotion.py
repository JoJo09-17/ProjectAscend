import unreal,json
from pathlib import Path
out={}
for i in range(1,5):
 a=unreal.load_asset('/Game/Combat/Ranged/Manny/Manny_Archer_AS_Combo_Attack_05_'+str(i).zfill(2)+'_Seq')
 rows=[]
 for f in range(int(a.get_play_length()*15)+1):
  t=f/15
  pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,t,unreal.AnimPoseEvaluationOptions())
  points=[]
  for bone in ['hand_l','hand_r','head','pelvis']:
   p=unreal.AnimPoseExtensions.get_bone_pose(pose,bone,unreal.AnimPoseSpaces.WORLD).translation
   points.append([round(p.x,1),round(p.y,1),round(p.z,1)])
  rows.append([round(t,3)]+points)
 out[str(i)]=rows
Path(unreal.Paths.project_saved_dir(),'ArcherBowMotion.json').write_text(json.dumps(out))
unreal.log('ARCHER_BOW_MOTION_COMPLETE')
