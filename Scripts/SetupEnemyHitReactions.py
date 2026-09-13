"""Retarget purchased directional hit reactions and configure both enemy profiles."""
import unreal,json
from pathlib import Path
root='/Game/Combat/HitReaction'
source=unreal.load_asset('/Game/ArtAsset/Animations/Archer/Demo/Characters/Mannequins/Meshes/SKM_Manny_Simple')
target=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple')
rtg=unreal.load_asset('/Game/Combat/Ranged/Rigs/RTG_Archer_Manny')
registry=unreal.AssetRegistryHelpers.get_asset_registry()
tools=unreal.AssetToolsHelpers.get_asset_tools()
report={}
for style,folder,prefix in [('Saber','01_Hit','AS_Hit_'),('Archer','02_Hit_Combat','AS_Hit_Combat_')]:
 mapping={}
 for direction,property in [('F','hit_front_montage'),('B','hit_back_montage'),('L','hit_left_montage'),('R','hit_right_montage')]:
  path='/Game/ArtAsset/Animations/Archer/Animation/Sequence/08_Hit/'+folder+'/'+prefix+direction+'_Seq'
  name='Manny_'+style+'_Hit_'+direction
  seq=unreal.load_asset(root+'/Manny/'+name)
  if not seq:
   data=registry.get_asset_by_object_path(path+'.'+prefix+direction+'_Seq')
   assert data.is_valid(),path
   results=unreal.IKRetargetBatchOperation.duplicate_and_retarget([data],source,target,rtg,prefix='HitReaction_',include_referenced_assets=False,overwrite_existing_files=False)
   assert len(results)==1
   seq=results[0].get_asset()
   assert unreal.EditorAssetLibrary.rename_asset(seq.get_path_name(),root+'/Manny/'+name)
  assert seq.get_editor_property('skeleton')==target.get_editor_property('skeleton')
  assert unreal.AscendMeleeEditorLibrary.bake_pelvis_root_motion(seq)
  unreal.EditorAssetLibrary.save_loaded_asset(seq,only_if_is_dirty=False)
  name='AM_Manny_'+style+'_Hit_'+direction
  montage=unreal.load_asset(root+'/Montages/'+name)
  if not montage:
   factory=unreal.AnimMontageFactory();factory.set_editor_property('source_animation',seq);factory.set_editor_property('target_skeleton',seq.get_editor_property('skeleton'))
   montage=tools.create_asset(name,root+'/Montages',unreal.AnimMontage,factory)
  assert unreal.AscendMeleeEditorLibrary.set_montage_slot(montage,'HitReactionSlot')
  # Use the short pose blend authored by the montage factory.
  blend=montage.get_editor_property('blend_in');blend.set_editor_property('blend_time',0.06);montage.set_editor_property('blend_in',blend)
  blend=montage.get_editor_property('blend_out');blend.set_editor_property('blend_time',0.12);montage.set_editor_property('blend_out',blend)
  unreal.EditorAssetLibrary.save_loaded_asset(montage,only_if_is_dirty=False)
  mapping[property]=montage
 profile=unreal.load_asset('/Game/Combat/Melee/Profiles/DA_'+style+'_Manny')
 for property,montage in mapping.items(): profile.set_editor_property(property,montage)
 unreal.EditorAssetLibrary.save_loaded_asset(profile,only_if_is_dirty=False)
 report[style]={p:m.get_path_name() for p,m in mapping.items()}
bp=unreal.load_asset('/Game/Characters/Mannequins/Animations/ABP_Manny')
assert unreal.AscendMeleeEditorLibrary.ensure_montage_slot(bp,'HitReactionSlot')
unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
Path(unreal.Paths.project_saved_dir(),'EnemyHitReactionSetup.json').write_text(json.dumps(report,indent=2))
unreal.log('ENEMY_HIT_REACTION_SETUP_COMPLETE')
