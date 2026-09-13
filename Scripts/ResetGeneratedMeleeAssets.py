"""Remove only generated melee outputs before a clean corrected retarget pass."""
import unreal

for blueprint_path in [
    '/Game/Blueprints/BP_AscendPlayerCharacter',
    '/Game/Blueprints/AI/BP_Saber',
    '/Game/Blueprints/AI/BP_AscendMeleeEnemy',
]:
    cls = unreal.EditorAssetLibrary.load_blueprint_class(blueprint_path)
    if cls:
        cdo = unreal.get_default_object(cls)
        combat = cdo.get_component_by_class(unreal.AscendMeleeCombatComponent)
        if combat:
            combat.set_editor_property('profile', None)
        unreal.EditorAssetLibrary.save_asset(blueprint_path, only_if_is_dirty=False)

for generated_path in [
    '/Game/Combat/Melee/Profiles',
    '/Game/Combat/Melee/Montages',
    '/Game/Combat/Melee/Manny',
    '/Game/Combat/Melee/UE4',
]:
    if unreal.EditorAssetLibrary.does_directory_exist(generated_path):
        # UE may return false when the now-empty virtual folder cannot itself
        # be removed; the contained generated assets are still deleted.
        unreal.EditorAssetLibrary.delete_directory(generated_path)

unreal.log('GENERATED_MELEE_ASSETS_RESET_COMPLETE')
