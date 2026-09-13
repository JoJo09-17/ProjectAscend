"""Create the level-design Blueprint wrapper for AscendMonsterSpawner."""
import unreal

asset_path = '/Game/Blueprints/AI/BP_MonsterSpawner'
if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
    unreal.log('MONSTER_SPAWNER_BLUEPRINT_EXISTS ' + asset_path)
else:
    parent_class = unreal.load_class(None, '/Script/ProjectAscend.AscendMonsterSpawner')
    assert parent_class
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        'BP_MonsterSpawner', '/Game/Blueprints/AI', unreal.Blueprint, factory)
    assert asset
    assert unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    unreal.log('MONSTER_SPAWNER_BLUEPRINT_CREATED ' + asset_path)
