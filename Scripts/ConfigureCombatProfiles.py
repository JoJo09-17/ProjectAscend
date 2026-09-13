"""Migrate named combat profiles without replacing authored Saber settings."""
import json
from pathlib import Path
import unreal
ROOT = '/Game/Combat/Melee/Profiles/'

def configure_archer():
    path = ROOT + 'DA_Archer_Manny'
    profile = unreal.load_asset(path)
    if not profile:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', unreal.AscendMeleeProfile)
        profile = unreal.AssetToolsHelpers.get_asset_tools().create_asset('DA_Archer_Manny', ROOT.rstrip('/'), unreal.AscendMeleeProfile, factory)
        for field, values in [('ranged_light_attack', (10, 2600, 12)), ('ranged_heavy_attack', (35, 1700, 28)), ('enemy_ranged_attack', (6, 1200, 14))]:
            attack = unreal.AscendRangedAttack()
            for key, value in zip(('damage', 'projectile_speed', 'projectile_radius'), values):
                attack.set_editor_property(key, value)
            profile.set_editor_property(field, attack)
    profile.set_editor_property('combat_style', unreal.AscendCombatProfileStyle.ARCHER)
    assert unreal.EditorAssetLibrary.save_loaded_asset(profile, only_if_is_dirty=False)
    return profile

def migrate():
    old = ROOT + 'DA_Melee_Manny'
    new = ROOT + 'DA_Saber_Manny'
    if not unreal.EditorAssetLibrary.does_asset_exist(new):
        assert unreal.EditorAssetLibrary.rename_asset(old, new)
    saber = unreal.load_asset(new)
    assert saber
    saber.set_editor_property('combat_style', unreal.AscendCombatProfileStyle.SABER)
    assert unreal.EditorAssetLibrary.save_loaded_asset(saber, only_if_is_dirty=False)
    archer = configure_archer()
    out = {}
    for path, profile in [('/Game/Blueprints/BP_AscendPlayerCharacter', archer), ('/Game/Blueprints/AI/BP_Saber', saber), ('/Game/Blueprints/AI/BP_Archer', archer)]:
        bp = unreal.load_asset(path)
        cdo = unreal.get_default_object(bp.generated_class())
        cdo.get_component_by_class(unreal.AscendMeleeCombatComponent).set_editor_property('profile', profile)
        assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
        out[path] = profile.get_path_name()
    out['status'] = 'complete'
    Path(unreal.Paths.project_saved_dir(), 'CombatProfilesReport.json').write_text(json.dumps(out, indent=2), encoding='utf-8')
    unreal.log('COMBAT_PROFILES_COMPLETE')

if __name__ == '__main__':
    migrate()
