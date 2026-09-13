import unreal,json
from pathlib import Path
out={}
for style in ['Saber','Archer']:
 profile=unreal.load_asset('/Game/Combat/Melee/Profiles/DA_'+style+'_Manny')
 cdo=unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/AI/BP_'+style))
 assert cdo.get_component_by_class(unreal.AscendMeleeCombatComponent).profile==profile
 skeleton=cdo.get_component_by_class(unreal.SkeletalMeshComponent).get_editor_property('skeletal_mesh_asset').get_editor_property('skeleton')
 out[style]={}
 for direction,prop in [('F','hit_front_montage'),('B','hit_back_montage'),('L','hit_left_montage'),('R','hit_right_montage')]:
  montage=profile.get_editor_property(prop)
  assert montage and montage.get_editor_property('skeleton')==skeleton
  assert str(unreal.AscendMeleeEditorLibrary.get_montage_slot(montage))=='HitReactionSlot'
  assert unreal.AscendMeleeEditorLibrary.get_melee_window_count(montage)==0
  timing=unreal.AscendMeleeEditorLibrary.get_ranged_timing(montage)
  assert timing.x<0 and timing.y<0 and timing.z<0
  out[style][direction]={'montage':montage.get_path_name(),'duration':montage.get_play_length()}
out['status']='complete'
Path(unreal.Paths.project_saved_dir(),'EnemyHitReactionVerification.json').write_text(json.dumps(out,indent=2))
unreal.log('ENEMY_HIT_REACTION_VERIFICATION_COMPLETE')
