import unreal

root = '/Game/Combat/Projectiles/Profiles'
tools = unreal.AssetToolsHelpers.get_asset_tools()
def ensure(name, mesh, transform):
    asset = unreal.load_asset(root + '/' + name)
    if not asset:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', unreal.AscendProjectileProfile)
        asset = tools.create_asset(name, root, unreal.AscendProjectileProfile, factory)
        assert asset
        asset.set_editor_property('mesh', unreal.load_asset(mesh))
        asset.set_editor_property('mesh_transform', transform)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset

light = ensure('DA_Archer_Projectile_Light', '/Engine/BasicShapes/Sphere',
               unreal.Transform(scale=unreal.Vector(.15,.15,.15)))
heavy = ensure('DA_Archer_Projectile_Heavy', '/Game/ArtAsset/Animations/Archer/Demo/Characters/Mannequins/Meshes/Arrow',
               unreal.Transform(location=unreal.Vector(-40,0,0), rotation=unreal.Rotator(0,-90,0), scale=unreal.Vector(.9,.9,.9)))
profile = unreal.load_asset('/Game/Combat/Melee/Profiles/DA_Archer_Manny')
assert profile
for field, asset in [('ranged_light_attack', light), ('ranged_heavy_attack', heavy), ('enemy_ranged_attack', light)]:
    attack = profile.get_editor_property(field)
    attack.set_editor_property('projectile_profile', asset)
    profile.set_editor_property(field, attack)
unreal.EditorAssetLibrary.save_loaded_asset(profile)
unreal.log('PROJECTILE_PROFILES_SETUP_COMPLETE')

