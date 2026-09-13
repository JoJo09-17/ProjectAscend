"""Create a replaceable default impact preset and author timing on hit montages."""
import unreal,json
from pathlib import Path
root='/Game/Combat/ImpactFX'
tools=unreal.AssetToolsHelpers.get_asset_tools()
material=unreal.load_asset(root+'/Materials/M_ImpactDecal_Default')
if not material:
 material=tools.create_asset('M_ImpactDecal_Default',root+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
 material.set_editor_property('material_domain',unreal.MaterialDomain.MD_DEFERRED_DECAL)
 material.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
 lib=unreal.MaterialEditingLibrary
 uv=lib.create_material_expression(material,unreal.MaterialExpressionTextureCoordinate,-600,0)
 center=lib.create_material_expression(material,unreal.MaterialExpressionConstant2Vector,-600,150)
 center.set_editor_property('r',.5);center.set_editor_property('g',.5)
 mask=lib.create_material_expression(material,unreal.MaterialExpressionSphereMask,-350,0)
 mask.set_editor_property('attenuation_radius',.45);mask.set_editor_property('hardness_percent',65)
 lib.connect_material_expressions(uv,'',mask,'A');lib.connect_material_expressions(center,'',mask,'B')
 color=lib.create_material_expression(material,unreal.MaterialExpressionVectorParameter,-350,250)
 color.set_editor_property('parameter_name','DecalColor');color.set_editor_property('default_value',unreal.LinearColor(.12,.015,.008,1))
 lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
 lib.connect_material_property(mask,'',unreal.MaterialProperty.MP_OPACITY)
 rough=lib.create_material_expression(material,unreal.MaterialExpressionConstant,-350,400)
 rough.set_editor_property('r',.8)
 lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 lib.recompile_material(material)
 unreal.EditorAssetLibrary.save_loaded_asset(material,only_if_is_dirty=False)
profile=unreal.load_asset(root+'/Profiles/DA_EnemyImpact_Default')
if not profile:
 factory=unreal.DataAssetFactory();factory.set_editor_property('data_asset_class',unreal.AscendImpactFXProfile)
 profile=tools.create_asset('DA_EnemyImpact_Default',root+'/Profiles',unreal.AscendImpactFXProfile,factory)
 profile.set_editor_property('particle_system',unreal.load_asset('/Game/ParagonGreystone/FX/Particles/Greystone/Abilities/Ultimate/FX/P_Greystone_HToKill_Impact'))
 profile.set_editor_property('vfx_scale',unreal.Vector(.15,.15,.15))
 profile.set_editor_property('decal_material',material)
 unreal.EditorAssetLibrary.save_loaded_asset(profile,only_if_is_dirty=False)
report={'profile':profile.get_path_name(),'material':material.get_path_name(),'montages':[]}
for style in ['Saber','Archer']:
 combat=unreal.load_asset('/Game/Combat/Melee/Profiles/DA_'+style+'_Manny')
 if not combat.get_editor_property('hit_fx_profile'):
  combat.set_editor_property('hit_fx_profile',profile)
  unreal.EditorAssetLibrary.save_loaded_asset(combat,only_if_is_dirty=False)
 for prop in ['hit_front_montage','hit_back_montage','hit_left_montage','hit_right_montage']:
  montage=combat.get_editor_property(prop)
  assert unreal.AscendMeleeEditorLibrary.set_impact_fx_timing(montage,.06,.12)
  unreal.EditorAssetLibrary.save_loaded_asset(montage,only_if_is_dirty=False)
  report['montages'].append(montage.get_path_name())
report['status']='complete'
Path(unreal.Paths.project_saved_dir(),'ImpactFXSetup.json').write_text(json.dumps(report,indent=2))
unreal.log('IMPACT_FX_SETUP_COMPLETE')
