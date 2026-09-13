import unreal
world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/DevelopmentMap')
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if isinstance(actor, unreal.AscendCharacterBase) and not isinstance(actor, unreal.AscendEnemyCharacter):
        combat = actor.get_component_by_class(unreal.AscendMeleeCombatComponent)
        unreal.log('PLAYER_PROFILE_CHECK ' + actor.get_actor_label() + ' ' + (combat.profile.get_path_name() if combat.profile else 'None'))
        assert combat.profile and combat.profile.combat_style == unreal.AscendCombatProfileStyle.ARCHER
unreal.log('PLAYER_PROFILE_CHECK_COMPLETE')
