"""Add the configured melee enemy archetype to DevelopmentMap's encounter spawner."""
import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/DevelopmentMap')
assert world
saber_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/AI/BP_Saber')
archer_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/AI/BP_Archer')
base_enemy_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/BP_AscendEnemy')
old_melee_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/AI/BP_AscendMeleeEnemy')
assert saber_class and archer_class and base_enemy_class

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
spawners = [actor for actor in actors if isinstance(actor, unreal.AscendMonsterSpawner)]
assert spawners, 'DevelopmentMap has no AscendMonsterSpawner'

for spawner in spawners:
    entries = list(spawner.get_editor_property('spawn_entries'))
    ranged_count = sum(entry.count for entry in entries if entry.monster_class in [base_enemy_class, archer_class]) or 2
    entries = [entry for entry in entries if entry.monster_class not in [base_enemy_class, archer_class, saber_class, old_melee_class]]
    melee = unreal.AscendMonsterSpawnEntry()
    melee.set_editor_property('monster_class', saber_class)
    melee.set_editor_property('count', 1)
    melee.set_editor_property('spawn_radius', 0.0)
    melee.set_editor_property('minimum_spacing', 175.0)
    melee.set_editor_property('local_offset', unreal.Vector(-550.0, 40.0, 0.0))
    ranged = unreal.AscendMonsterSpawnEntry()
    ranged.set_editor_property('monster_class', archer_class)
    ranged.set_editor_property('count', ranged_count)
    ranged.set_editor_property('spawn_radius', 500.0)
    ranged.set_editor_property('minimum_spacing', 150.0)
    ranged.set_editor_property('local_offset', unreal.Vector(0.0, 0.0, 0.0))
    spawner.set_editor_property('spawn_entries', [melee, ranged] + entries)

assert unreal.EditorLevelLibrary.save_current_level()
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log('MELEE_ENCOUNTER_SETUP_COMPLETE ' + str(len(spawners)))
