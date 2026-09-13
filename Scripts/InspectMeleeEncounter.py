"""Report loaded DevelopmentMap combat actors and spawner entries."""
import json
from pathlib import Path
import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/DevelopmentMap')
assert world
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
rows = []
for actor in actors:
    row = {
        'label': actor.get_actor_label(),
        'class': actor.get_class().get_path_name(),
        'location': [actor.get_actor_location().x, actor.get_actor_location().y, actor.get_actor_location().z],
    }
    if isinstance(actor, unreal.AscendMonsterSpawner):
        entries = []
        for entry in actor.get_editor_property('spawn_entries'):
            entries.append({
                'class': entry.monster_class.get_path_name() if entry.monster_class else None,
                'count': entry.count,
                'radius': entry.spawn_radius,
                'offset': [entry.local_offset.x, entry.local_offset.y, entry.local_offset.z],
            })
        row['spawn_entries'] = entries
    rows.append(row)

Path(unreal.Paths.project_saved_dir(), 'MeleeEncounterInspection.json').write_text(
    json.dumps(rows, indent=2), encoding='utf-8')
unreal.log('MELEE_ENCOUNTER_INSPECTION_COMPLETE')
