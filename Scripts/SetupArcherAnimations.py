"""Adapt purchased Archer combo attacks onto project Manny and bind ranged profile."""
import unreal,json
from pathlib import Path
COMBO='05'  # Select the purchased combo set to preview.
ROOT='/Game/Combat/Ranged'
SRC='/Game/ArtAsset/Animations/Archer'
tools=unreal.AssetToolsHelpers.get_asset_tools()
def load(path):
 a=unreal.load_asset(path);assert a,path;return a
source=load(SRC+'/Demo/Characters/Mannequins/Meshes/SKM_Manny_Simple')
target=load('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple')
rig_path=ROOT+'/Rigs/IKR_ArcherSource'
rig=unreal.load_asset(rig_path) or unreal.EditorAssetLibrary.duplicate_asset('/Game/Combat/Melee/Rigs/IKR_Manny',rig_path)
assert unreal.IKRigController.get_controller(rig).set_skeletal_mesh(source)
unreal.EditorAssetLibrary.save_loaded_asset(rig)
rtg_path=ROOT+'/Rigs/RTG_Archer_Manny'
rtg=unreal.load_asset(rtg_path) or unreal.EditorAssetLibrary.duplicate_asset('/Game/Combat/Melee/Rigs/RTG_UAL2_Manny',rtg_path)
c=unreal.IKRetargeterController.get_controller(rtg)
for side,ik in [(unreal.RetargetSourceOrTarget.SOURCE,rig),(unreal.RetargetSourceOrTarget.TARGET,load('/Game/Combat/Melee/Rigs/IKR_Manny'))]:
 c.set_ik_rig(side,ik);c.assign_ik_rig_to_all_ops(side,ik);c.reset_retarget_pose('Default Pose',[],side)
c.auto_map_chains(unreal.AutoMapChainType.EXACT,True)
c.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET)
unreal.EditorAssetLibrary.save_loaded_asset(rtg)
registry=unreal.AssetRegistryHelpers.get_asset_registry()
originals=[a for a in registry.get_assets_by_path(SRC+'/Animation/Sequence/02_Attack/'+COMBO+'_Combo_Attack_'+COMBO,recursive=True) if str(a.asset_class_path.asset_name)=='AnimSequence']
assert len(originals)==5
sequences={}
missing=[]
for data in originals:
 name='Manny_Archer_'+str(data.asset_name)
 existing=unreal.load_asset(ROOT+'/Manny/'+name)
 if existing:
  sequences[name]=existing
 else:
  assert data.get_asset().get_editor_property('skeleton')==source.get_editor_property('skeleton')
  missing.append(data)
outputs=unreal.IKRetargetBatchOperation.duplicate_and_retarget(missing,source,target,rtg,prefix='Manny_Archer_',include_referenced_assets=False,overwrite_existing_files=True) if missing else []
assert len(outputs)==len(missing)
for data in outputs:
 a=data.get_asset();name=a.get_name();dest=ROOT+'/Manny/'+name
 if a.get_path_name().split('.')[0]!=dest: assert unreal.EditorAssetLibrary.rename_asset(a.get_path_name(),dest)
 assert a.get_editor_property('skeleton')==target.get_editor_property('skeleton')
 unreal.EditorAssetLibrary.save_loaded_asset(a,only_if_is_dirty=False)
 sequences[name]=a
bp=load('/Game/Characters/Mannequins/Animations/ABP_Manny')
assert unreal.AscendMeleeEditorLibrary.ensure_montage_slot(bp,'RangedSlot')
unreal.EditorAssetLibrary.save_loaded_asset(bp,only_if_is_dirty=False)
montages=[]
for index in range(1,5):
 name='AM_Manny_Archer_Combo_'+COMBO+'_'+str(index).zfill(2)
 path=ROOT+'/Montages/'+name
 montage=unreal.load_asset(path)
 seq=sequences['Manny_Archer_AS_Combo_Attack_'+COMBO+'_'+str(index).zfill(2)+'_Seq']
 assert unreal.AscendMeleeEditorLibrary.bake_pelvis_root_motion(seq)
 seq.set_editor_property('enable_root_motion',True)
 seq.set_editor_property('force_root_lock',True)
 unreal.EditorAssetLibrary.save_loaded_asset(seq,only_if_is_dirty=False)
 if not montage:
  factory=unreal.AnimMontageFactory();factory.set_editor_property('source_animation',seq);factory.set_editor_property('target_skeleton',seq.get_editor_property('skeleton'))
  montage=tools.create_asset(name,ROOT+'/Montages',unreal.AnimMontage,factory)
 assert unreal.AscendMeleeEditorLibrary.set_montage_slot(montage,'RangedSlot')
 # The first three points match the next segment's starting pose. Stage four
 # can cancel its long roll recovery after the shot, instead of waiting 2.67s.
 timing05=[(0.233,0.333),(0.233,0.533),(0.233,0.533),(0.167,0.300)]
 release,combo_open=timing05[index-1] if COMBO=='05' else (seq.get_play_length()*0.25,seq.get_play_length()*0.55)
 assert unreal.AscendMeleeEditorLibrary.set_ranged_timing(montage,release,combo_open,seq.get_play_length()-0.15,0.10)
 unreal.EditorAssetLibrary.save_loaded_asset(montage,only_if_is_dirty=False)
 montages.append(montage)
profile=load('/Game/Combat/Melee/Profiles/DA_Archer_Manny')
profile.set_editor_property('ranged_light_montages',montages)
heavy_path=ROOT+'/Montages/AM_Manny_Archer_Heavy_'+COMBO
heavy=unreal.load_asset(heavy_path) or unreal.EditorAssetLibrary.duplicate_asset(montages[-1].get_path_name(),heavy_path)
assert unreal.AscendMeleeEditorLibrary.set_ranged_release_time(heavy,0.20 if COMBO=='05' else montages[-1].get_play_length()*0.25)
unreal.EditorAssetLibrary.save_loaded_asset(heavy,only_if_is_dirty=False)
profile.set_editor_property('ranged_heavy_montage',heavy)
heavy_stats=profile.get_editor_property('ranged_heavy_attack')
heavy_stats.set_editor_property('pierce_enemies',True)
profile.set_editor_property('ranged_heavy_attack',heavy_stats)
unreal.EditorAssetLibrary.save_loaded_asset(profile,only_if_is_dirty=False)
player=unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/BP_AscendPlayerCharacter'))
archer_bp=load('/Game/Blueprints/AI/BP_Archer')
archer=unreal.get_default_object(archer_bp.generated_class())
for prop in ['skeletal_mesh_asset','anim_class']:
 archer.get_component_by_class(unreal.SkeletalMeshComponent).set_editor_property(prop,player.get_component_by_class(unreal.SkeletalMeshComponent).get_editor_property(prop))
unreal.EditorAssetLibrary.save_loaded_asset(archer_bp,only_if_is_dirty=False)
report={'status':'complete','profile':profile.get_path_name(),'montages':[m.get_path_name() for m in montages],'heavy':heavy.get_path_name(),'retargeter':rtg.get_path_name()}
Path(unreal.Paths.project_saved_dir(),'ArcherAnimationSetup.json').write_text(json.dumps(report,indent=2))
unreal.log('ARCHER_ANIMATION_SETUP_COMPLETE')


