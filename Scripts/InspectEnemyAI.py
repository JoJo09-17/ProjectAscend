"""Read the persisted enemy Blueprint defaults used by the demo map."""
import unreal

blueprint = unreal.load_asset('/Game/Blueprints/BP_AscendEnemy')
assert blueprint
enemy_class = blueprint.generated_class()
enemy = unreal.get_default_object(enemy_class)
movement = enemy.get_component_by_class(unreal.CharacterMovementComponent)

unreal.log('ENEMY_AI_CLASS=' + str(enemy.get_editor_property('ai_controller_class')))
unreal.log('ENEMY_AUTO_POSSESS=' + str(enemy.get_editor_property('auto_possess_ai')))
unreal.log('ENEMY_MAX_WALK_SPEED=' + str(movement.get_editor_property('max_walk_speed')))
unreal.log('ENEMY_MOVEMENT_MODE=' + str(movement.get_editor_property('default_land_movement_mode')))
unreal.log('ENEMY_AI_DEFAULTS_VERIFIED')
