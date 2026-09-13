import unreal,json
from pathlib import Path
fx=unreal.load_asset('/Game/Combat/ImpactFX/Profiles/DA_EnemyImpact_Default')
system=fx.get_editor_property('niagara_system')
assert system
before=[str(n) for n in unreal.AscendMeleeEditorLibrary.get_niagara_user_parameter_names(system)]
assert unreal.AscendMeleeEditorLibrary.ensure_niagara_direction_parameter(system,'User.ProjectileDirection')
fx.set_editor_property('niagara_direction_parameter','User.ProjectileDirection')
unreal.EditorAssetLibrary.save_loaded_asset(system,only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_loaded_asset(fx,only_if_is_dirty=False)
Path(unreal.Paths.project_saved_dir(),'ImpactDirectionSetup.json').write_text(json.dumps({'system':system.get_path_name(),'before':before,'parameter':'User.ProjectileDirection','status':'complete'},indent=2))
unreal.log('IMPACT_DIRECTION_SETUP_COMPLETE')
