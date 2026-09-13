import unreal,json
from pathlib import Path
root='/Game/ArtAsset/Animations/Archer/Animation/Sequence/02_Attack/01_Combo_Attack_01'
reg=unreal.AssetRegistryHelpers.get_asset_registry()
out={'attacks':[],'meshes':[]}
for data in reg.get_assets_by_path(root,recursive=True):
 a=data.get_asset()
 if isinstance(a,unreal.AnimSequence):
  sk=a.get_editor_property('skeleton')
  out['attacks'].append({'path':a.get_path_name(),'skeleton':sk.get_path_name(),'length':a.get_play_length(),'bones':[]})
for data in reg.get_assets_by_path('/Game/ArtAsset',recursive=True):
 if str(data.asset_class_path.asset_name)=='SkeletalMesh':
  m=data.get_asset();out['meshes'].append({'path':m.get_path_name(),'skeleton':m.get_editor_property('skeleton').get_path_name()})
Path(unreal.Paths.project_saved_dir(),'ArcherAssetInspection.json').write_text(json.dumps(out,indent=2))

