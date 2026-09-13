"""Add a navigation bounds volume covering the DevelopmentMap combat area."""
import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/DevelopmentMap')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

nav_volumes = [actor for actor in actors if isinstance(actor, unreal.NavMeshBoundsVolume)]
if nav_volumes:
    nav_volume = nav_volumes[0]
else:
    combat_points = []
    for actor in actors:
        class_name = actor.get_class().get_name()
        if 'AscendEnemy' in class_name or 'PlayerStart' in class_name:
            combat_points.append(actor.get_actor_location())

    if combat_points:
        min_x = min(point.x for point in combat_points)
        max_x = max(point.x for point in combat_points)
        min_y = min(point.y for point in combat_points)
        max_y = max(point.y for point in combat_points)
        center = unreal.Vector((min_x + max_x) * 0.5, (min_y + max_y) * 0.5, 0.0)
        extent_x = max((max_x - min_x) * 0.5 + 1500.0, 3000.0)
        extent_y = max((max_y - min_y) * 0.5 + 1500.0, 3000.0)
    else:
        center = unreal.Vector(0.0, 0.0, 0.0)
        extent_x = extent_y = 4000.0

    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    nav_volume = actor_subsystem.spawn_actor_from_class(unreal.NavMeshBoundsVolume, center)
    assert nav_volume
    nav_volume.set_actor_label('NavMeshBoundsVolume_CombatArena')
    # A newly spawned volume uses a 200uu cube brush (100uu half extent).
    nav_volume.set_actor_scale3d(unreal.Vector(extent_x / 100.0, extent_y / 100.0, 10.0))

assert unreal.EditorLevelLibrary.save_current_level()
unreal.log('COMBAT_NAVMESH_SAVED ' + str(nav_volume.get_actor_location()) + ' ' + str(nav_volume.get_actor_scale3d()))
