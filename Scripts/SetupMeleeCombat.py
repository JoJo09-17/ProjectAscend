"""Create melee montages/profiles and bind them to the player and a melee enemy archetype."""
import json
import traceback
from pathlib import Path
import unreal

ROOT = '/Game/Combat/Melee'
REPORT = Path(unreal.Paths.project_saved_dir()) / 'MeleeSetupReport.json'
tools = unreal.AssetToolsHelpers.get_asset_tools()
result = {'status': 'failed'}


def asset(path):
    value = unreal.load_asset(path)
    assert value, 'Missing asset: ' + path
    return value


def make_montage(name, sequence_path, window):
    path = ROOT + '/Montages/' + name
    montage = unreal.load_asset(path)
    if not montage:
        sequence = asset(sequence_path)
        factory = unreal.AnimMontageFactory()
        factory.set_editor_property('source_animation', sequence)
        factory.set_editor_property('target_skeleton', sequence.get_editor_property('skeleton'))
        montage = tools.create_asset(name, ROOT + '/Montages', unreal.AnimMontage, factory)
    assert montage
    assert unreal.AscendMeleeEditorLibrary.set_montage_slot(montage, 'MeleeSlot')
    # Keep setup idempotent by replacing only our notify class.
    start, duration = window
    assert unreal.AscendMeleeEditorLibrary.set_melee_window(montage, start, duration)
    assert unreal.EditorAssetLibrary.save_loaded_asset(montage, only_if_is_dirty=False)
    return montage


def attack(montage, damage, multiplier=1.0, play_rate=1.0):
    entry = unreal.AscendMeleeAttack()
    entry.set_editor_property('montage', montage)
    entry.set_editor_property('damage', damage)
    entry.set_editor_property('damage_multiplier', multiplier)
    entry.set_editor_property('play_rate', play_rate)
    return entry


def make_profile(name, prefix):
    path = ROOT + '/Profiles/' + name
    profile = unreal.load_asset(path)
    if not profile:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', unreal.AscendMeleeProfile)
        profile = tools.create_asset(name, ROOT + '/Profiles', unreal.AscendMeleeProfile, factory)
    base = ROOT + '/' + prefix + '/InPlace/' + prefix + '_UAL2_StandardSword_'
    montages = [
        make_montage('AM_' + prefix + '_Sword_A', base + 'Regular_A', (0.08, 0.25)),
        make_montage('AM_' + prefix + '_Sword_B', base + 'Regular_B', (0.10, 0.30)),
        make_montage('AM_' + prefix + '_Sword_C', base + 'Regular_C', (0.28, 0.48)),
    ]
    heavy = make_montage('AM_' + prefix + '_Sword_Heavy', base + 'Heavy_Combo', (0.36, 0.48))
    profile.set_editor_property('light_attacks', [attack(montages[0], 16), attack(montages[1], 18), attack(montages[2], 24)])
    profile.set_editor_property('heavy_attack', attack(heavy, 36, 1.0, 1.15))
    profile.set_editor_property('weapon_bone', 'hand_r')
    profile.set_editor_property('blade_length', 100.0)
    profile.set_editor_property('hit_half_width', 7.0)
    # Manny's hand X axis follows the forearm. Rotate the authored blade onto
    # the hand's outward sword axis so the visible mesh and swept box coincide.
    profile.set_editor_property('weapon_offset', unreal.Transform(location=[3, 0, 0], rotation=unreal.Rotator(0, 110, 90), scale=[1, 1, 1]))
    profile.set_editor_property('sword_material', unreal.load_asset('/Engine/BasicShapes/BasicShapeMaterial'))
    profile.set_editor_property('trail_material', unreal.load_asset('/Game/ParagonGreystone/FX/Materials/Heroes/Greystone/Abilities/M_Greystone_Spiral_Trail'))
    unreal.EditorAssetLibrary.save_loaded_asset(profile, only_if_is_dirty=False)
    return profile, montages + [heavy]


try:
    manny_profile, manny_montages = make_profile('DA_Saber_Manny', 'Manny')
    import sys
    sys.path.insert(0, str(Path(unreal.Paths.project_dir()) / 'Scripts'))
    from ConfigureCombatProfiles import configure_archer
    archer_profile = configure_archer()
    ue4_profile, ue4_montages = make_profile('DA_Melee_UE4', 'UE4')

    anim_bp = asset('/Game/Characters/Mannequins/Animations/ABP_Manny')
    assert unreal.AscendMeleeEditorLibrary.ensure_montage_slot(anim_bp, 'MeleeSlot')
    unreal.EditorAssetLibrary.save_loaded_asset(anim_bp, only_if_is_dirty=False)

    player_class = unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/BP_AscendPlayerCharacter')
    player = unreal.get_default_object(player_class)
    player.get_component_by_class(unreal.AscendMeleeCombatComponent).set_editor_property('profile', archer_profile)
    unreal.EditorAssetLibrary.save_asset('/Game/Blueprints/BP_AscendPlayerCharacter', only_if_is_dirty=False)

    old_enemy_path = '/Game/Blueprints/AI/BP_AscendMeleeEnemy'
    enemy_path = '/Game/Blueprints/AI/BP_Saber'
    if not unreal.EditorAssetLibrary.does_asset_exist(enemy_path) and unreal.EditorAssetLibrary.does_asset_exist(old_enemy_path):
        assert unreal.EditorAssetLibrary.rename_asset(old_enemy_path, enemy_path)
    enemy_bp = unreal.load_asset(enemy_path)
    if not enemy_bp:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/BP_AscendEnemy'))
        enemy_bp = tools.create_asset('BP_Saber', '/Game/Blueprints/AI', unreal.Blueprint, factory)
    enemy = unreal.get_default_object(enemy_bp.generated_class())
    enemy.set_editor_property('combat_style', unreal.AscendEnemyCombatStyle.MELEE)
    enemy.set_editor_property('melee_attack_range', 150.0)
    enemy.set_editor_property('melee_attack_cooldown', 1.5)
    enemy_mesh = enemy.get_component_by_class(unreal.SkeletalMeshComponent)
    player_mesh = player.get_component_by_class(unreal.SkeletalMeshComponent)
    enemy_mesh.set_editor_property('skeletal_mesh_asset', player_mesh.get_editor_property('skeletal_mesh_asset'))
    enemy_mesh.set_editor_property('anim_class', player_mesh.get_editor_property('anim_class'))
    enemy.get_component_by_class(unreal.AscendMeleeCombatComponent).set_editor_property('profile', manny_profile)
    unreal.EditorAssetLibrary.save_loaded_asset(enemy_bp, only_if_is_dirty=False)

    archer_path = '/Game/Blueprints/AI/BP_Archer'
    archer_bp = unreal.load_asset(archer_path)
    if not archer_bp:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprints/BP_AscendEnemy'))
        archer_bp = tools.create_asset('BP_Archer', '/Game/Blueprints/AI', unreal.Blueprint, factory)
    archer = unreal.get_default_object(archer_bp.generated_class())
    archer.set_editor_property('combat_style', unreal.AscendEnemyCombatStyle.RANGED)
    archer.get_component_by_class(unreal.AscendMeleeCombatComponent).set_editor_property('profile', archer_profile)
    unreal.EditorAssetLibrary.save_loaded_asset(archer_bp, only_if_is_dirty=False)

    result = {
        'status': 'complete',
        'profiles': [manny_profile.get_path_name(), ue4_profile.get_path_name()],
        'montages': [m.get_path_name() for m in manny_montages + ue4_montages],
        'player_profile': player.get_component_by_class(unreal.AscendMeleeCombatComponent).profile.get_path_name(),
        'saber': enemy_path,
        'archer': archer_path,
        'enemy_profile': enemy.get_component_by_class(unreal.AscendMeleeCombatComponent).profile.get_path_name(),
        'anim_blueprint': anim_bp.get_path_name(),
    }
except Exception:
    result['error'] = traceback.format_exc()
    unreal.log_error(result['error'])

REPORT.write_text(json.dumps(result, indent=2), encoding='utf-8')
assert result['status'] == 'complete', result.get('error')
unreal.log('MELEE_SETUP_COMPLETE')

