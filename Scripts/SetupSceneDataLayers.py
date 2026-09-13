"""Create runtime scene Data Layer assets and instances for DevelopmentMap."""
import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/DevelopmentMap')
assert world

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_layers = unreal.get_editor_subsystem(unreal.DataLayerEditorSubsystem)
specs = [
    ('DL_Scene_Base', unreal.Color(90, 200, 120, 255), unreal.DataLayerRuntimeState.ACTIVATED, True),
    ('DL_Scene_Variant_A', unreal.Color(80, 150, 255, 255), unreal.DataLayerRuntimeState.UNLOADED, False),
    ('DL_Scene_Variant_B', unreal.Color(220, 110, 255, 255), unreal.DataLayerRuntimeState.UNLOADED, False),
]

for name, color, runtime_state, initially_visible in specs:
    asset_path = '/Game/DataLayers/' + name
    asset = unreal.load_asset(asset_path)
    if not asset:
        asset = asset_tools.create_asset(name, '/Game/DataLayers', unreal.DataLayerAsset, unreal.DataLayerFactory())
        assert asset
    asset.set_editor_property('data_layer_type', unreal.DataLayerType.RUNTIME)
    asset.set_editor_property('debug_color', color)
    unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)

    instance = editor_layers.get_data_layer_instance(asset)
    if not instance:
        params = unreal.DataLayerCreationParameters()
        params.set_editor_property('data_layer_asset', asset)
        params.set_editor_property('is_private', False)
        instance = editor_layers.create_data_layer_instance(params)
    assert instance
    editor_layers.set_data_layer_initial_runtime_state(instance, runtime_state)
    editor_layers.set_data_layer_is_initially_visible(instance, initially_visible)
    unreal.log('SCENE_DATA_LAYER_READY ' + name + ' ' + str(runtime_state))

assert unreal.EditorLevelLibrary.save_current_level()

trigger_path = '/Game/Blueprints/World/BP_DataLayerTrigger'
if not unreal.EditorAssetLibrary.does_asset_exist(trigger_path):
    parent_class = unreal.load_class(None, '/Script/ProjectAscend.AscendDataLayerTrigger')
    assert parent_class
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent_class)
    trigger_asset = asset_tools.create_asset(
        'BP_DataLayerTrigger', '/Game/Blueprints/World', unreal.Blueprint, factory)
    assert trigger_asset
    assert unreal.EditorAssetLibrary.save_loaded_asset(trigger_asset, only_if_is_dirty=False)
unreal.log('SCENE_DATA_LAYERS_AND_TRIGGER_READY')
