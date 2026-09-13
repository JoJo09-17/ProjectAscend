import unreal,json
from pathlib import Path
p='/Game/ArtAsset/Animations/Archer/Demo/Characters/Mannequins/Meshes/Arrow'
a=unreal.load_asset(p)
out={'class':a.get_class().get_name(),'path':p}
if isinstance(a,unreal.StaticMesh):
 b=a.get_bounding_box();out['min']=[b.min.x,b.min.y,b.min.z];out['max']=[b.max.x,b.max.y,b.max.z]
Path(unreal.Paths.project_saved_dir(),'ArrowAssetInspection.json').write_text(json.dumps(out))
