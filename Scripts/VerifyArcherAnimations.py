import unreal,json
from pathlib import Path
profile=unreal.load_asset('/Game/Combat/Melee/Profiles/DA_Archer_Manny')
assert len(profile.ranged_light_montages)==4
assert [m.get_name() for m in profile.ranged_light_montages] == ['AM_Manny_Archer_Combo_05_' + str(i).zfill(2) for i in range(1,5)]
out={}
for path in ['/Game/Blueprints/BP_AscendPlayerCharacter','/Game/Blueprints/AI/BP_Archer']:
 cdo=unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(path))
 mesh=cdo.get_component_by_class(unreal.SkeletalMeshComponent)
 assert cdo.get_component_by_class(unreal.AscendMeleeCombatComponent).profile==profile
 for montage in list(profile.ranged_light_montages)+[profile.ranged_heavy_montage]:
  assert montage.get_editor_property('skeleton')==mesh.get_editor_property('skeletal_mesh_asset').get_editor_property('skeleton')
  assert str(unreal.AscendMeleeEditorLibrary.get_montage_slot(montage))=='RangedSlot'
  assert unreal.AscendMeleeEditorLibrary.get_melee_window_count(montage)==0
  timing=unreal.AscendMeleeEditorLibrary.get_ranged_timing(montage)
  if montage==profile.ranged_heavy_montage:
   assert montage.get_name()=='AM_Manny_Archer_Heavy_05'
   assert abs(timing.x-0.20)<0.001 and timing.y<0 and timing.z<0
   assert profile.ranged_heavy_attack.get_editor_property('pierce_enemies')
  else:
   assert 0 < timing.x < timing.y < timing.z < montage.get_play_length()
 out[path]={'profile':profile.get_path_name(),'mesh':mesh.get_editor_property('skeletal_mesh_asset').get_path_name()}
out['status']='complete'
out['normal_attack_montages']=[m.get_path_name() for m in profile.ranged_light_montages]
Path(unreal.Paths.project_saved_dir(),'ArcherAnimationVerification.json').write_text(json.dumps(out,indent=2))
unreal.log('ARCHER_ANIMATION_VERIFICATION_COMPLETE')


