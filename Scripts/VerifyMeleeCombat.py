"""Verify saved melee assets, skeleton compatibility, notifies and actor defaults."""
import json
from pathlib import Path
import unreal

out = {}
player_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/BP_AscendPlayerCharacter')
enemy_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/AI/BP_Saber')
archer_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/AI/BP_Archer')
assert player_class and enemy_class and archer_class
for label, cls in [('enemy', enemy_class)]:
    cdo = unreal.get_default_object(cls)
    mesh = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    combat = cdo.get_component_by_class(unreal.AscendMeleeCombatComponent)
    assert combat and combat.profile
    attacks = list(combat.profile.light_attacks) + [combat.profile.heavy_attack]
    assert len(attacks) == 4
    rows = []
    for entry in attacks:
        montage = entry.montage
        assert montage and montage.get_editor_property('skeleton') == mesh.get_editor_property('skeletal_mesh_asset').skeleton
        assert str(unreal.AscendMeleeEditorLibrary.get_montage_slot(montage)) == 'MeleeSlot'
        assert unreal.AscendMeleeEditorLibrary.get_melee_window_count(montage) == 1
        rows.append({'montage': montage.get_path_name(), 'length': montage.get_play_length(), 'damage': entry.damage})
    out[label] = {'profile': combat.profile.get_path_name(), 'skeleton': mesh.get_editor_property('skeletal_mesh_asset').skeleton.get_path_name(), 'attacks': rows}
enemy_cdo = unreal.get_default_object(enemy_class)
assert enemy_cdo.get_editor_property('combat_style') == unreal.AscendEnemyCombatStyle.MELEE
out['enemy']['combat_style'] = str(enemy_cdo.get_editor_property('combat_style'))
archer_cdo = unreal.get_default_object(archer_class)
assert archer_cdo.get_editor_property('combat_style') == unreal.AscendEnemyCombatStyle.RANGED
assert archer_cdo.get_component_by_class(unreal.AscendMeleeCombatComponent).profile.combat_style == unreal.AscendCombatProfileStyle.ARCHER
out['archer'] = {
    'class': archer_class.get_path_name(),
    'combat_style': str(archer_cdo.get_editor_property('combat_style')),
    'profile': archer_cdo.get_component_by_class(unreal.AscendMeleeCombatComponent).profile.get_path_name(),
}
assert enemy_cdo.get_component_by_class(unreal.AscendMeleeCombatComponent).profile.combat_style == unreal.AscendCombatProfileStyle.SABER
player = unreal.get_default_object(player_class)
profile = player.get_component_by_class(unreal.AscendMeleeCombatComponent).profile
assert profile and profile.combat_style == unreal.AscendCombatProfileStyle.ARCHER
for attack in [profile.ranged_light_attack, profile.ranged_heavy_attack, profile.enemy_ranged_attack]:
    assert attack.damage > 0 and attack.projectile_speed > 0 and attack.projectile_radius > 0
out['player'] = {'profile': profile.get_path_name(), 'combat_style': str(profile.combat_style)}
out['status'] = 'complete'
Path(unreal.Paths.project_saved_dir(), 'MeleeVerification.json').write_text(json.dumps(out, indent=2), encoding='utf-8')
unreal.log('MELEE_VERIFICATION_COMPLETE')

