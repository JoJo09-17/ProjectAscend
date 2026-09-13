"""Idempotent authoring: create missing Toon samples without replacing user assets."""
import unreal
import json
from pathlib import Path

ROOT = '/Game/Materials/Toon'
tools = unreal.AssetToolsHelpers.get_asset_tools()
edit = unreal.MaterialEditingLibrary

def asset(name, cls, factory):
    path = ROOT + '/' + name
    existing = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if existing:
        return existing, False
    return tools.create_asset(name, ROOT, cls, factory), True

def curve(name):
    path = ROOT + '/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.CSVImportFactory()
    settings = unreal.CSVImportSettings()
    settings.set_editor_property('import_type', unreal.CSVImportType.ECSV_CURVE_FLOAT)
    factory.set_editor_property('automated_import_settings', settings)
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', str(Path(__file__).parent / 'ToonCurves' / (name + '.csv')))
    task.set_editor_property('destination_path', ROOT)
    task.set_editor_property('destination_name', name)
    task.set_editor_property('automated', True)
    task.set_editor_property('save', True)
    task.set_editor_property('factory', factory)
    tools.import_asset_tasks([task])
    obj = unreal.load_asset(path)
    assert obj and abs(obj.get_float_value(1) - 1) < .001
    return obj

soft = curve('C_Toon_Smooth')
bands = curve('C_Toon_ThreeBands')
step = curve('C_Toon_Hard')
factory = unreal.DataAssetFactory()
factory.set_editor_property('data_asset_class', unreal.JoJoToonProfileLibrary)
library, created = asset('DA_ToonProfiles', unreal.JoJoToonProfileLibrary, factory)
if created:
    base = unreal.JoJoToonProfile()
    # Default keeps the analytic response; optional samples exercise texture ramps.
    smooth = unreal.JoJoToonProfile()
    smooth.set_editor_property('shadow_softness', .2)
    smooth.set_editor_property('shadow_ramp', soft)
    smooth.set_editor_property('highlight_ramp', soft)
    striped = unreal.JoJoToonProfile()
    striped.set_editor_property('shadow_ramp', bands)
    striped.set_editor_property('shadow_softness', .45)
    striped.set_editor_property('highlight_ramp', step)
    striped.set_editor_property('rim_ramp', soft)
    striped.set_editor_property('metal_ramp', bands)
    library.set_editor_property('profiles', [base, smooth, striped])
    unreal.EditorAssetLibrary.save_loaded_asset(library)

material, created = asset('M_ToonSurface', unreal.Material, unreal.MaterialFactoryNew())
if created:
    def scalar(name, value, x, y):
        node = edit.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
        node.set_editor_property('parameter_name', name)
        node.set_editor_property('default_value', value)
        return node
    color = edit.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -500, -200)
    color.set_editor_property('parameter_name', 'BaseColor')
    color.set_editor_property('default_value', unreal.LinearColor(.45, .45, .45, 1))
    edit.connect_material_property(color, '', unreal.MaterialProperty.MP_BASE_COLOR)
    for name, value, prop, y in [
        ('Roughness', .5, unreal.MaterialProperty.MP_ROUGHNESS, 0),
        ('Metallic', 0, unreal.MaterialProperty.MP_METALLIC, 150),
        ('Specular', .5, unreal.MaterialProperty.MP_SPECULAR, 300),
    ]:
        edit.connect_material_property(scalar(name, value, -500, y), '', prop)
    assert unreal.AscendToonSettings.connect_toon_inputs(material,
        scalar('ToonProfile', 1, -500, 450), scalar('ToonDetailScale', .1, -500, 600))
    edit.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

for name, profile, metallic in [('MI_Toon_Default', 1, 0), ('MI_Toon_Soft', 2, 0), ('MI_Toon_Bands', 3, 0), ('MI_Toon_Metal', 1, 1), ('MI_Toon_PBR', 0, 0)]:
    instance, created = asset(name, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    if created:
        edit.set_material_instance_parent(instance, material)
        edit.set_material_instance_scalar_parameter_value(instance, 'ToonProfile', profile)
        edit.set_material_instance_scalar_parameter_value(instance, 'Metallic', metallic)
        unreal.EditorAssetLibrary.save_loaded_asset(instance)

result = {'library': library.get_path_name(), 'profiles': len(library.get_editor_property('profiles')),
          'master': material.get_path_name(), 'parameters': [str(x) for x in edit.get_scalar_parameter_names(material)],
          'substrate': unreal.SystemLibrary.get_console_variable_int_value('r.Substrate')}
assert result['substrate'] == 0
assert 'ToonProfile' in result['parameters']
Path(unreal.Paths.project_saved_dir(), 'ToonBRDFAssets.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('TOON_BRDF_ASSETS_VERIFIED')
