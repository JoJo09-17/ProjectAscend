import unreal, sys, json, traceback
from pathlib import Path

sys.path.insert(0, str(Path(unreal.Paths.engine_plugins_dir()) / 'FX/CascadeToNiagaraConverter/Content/Python'))
import CascadeToNiagaraConverter as converter
import CascadeToNiagaraHelperMethods as helpers
import Paths
utils = unreal.FXConverterUtilitiesLibrary
source = unreal.load_asset('/Game/StylizedToonShotFX04/Particles/P_ky_shotIce1')
assert source
destination = '/Game/Combat/Projectiles/FX/NS_Archer_IceShot'
assert not unreal.EditorAssetLibrary.does_asset_exist(destination), 'Refusing to overwrite an authored Niagara asset'
# Converted runtime assets must not depend on a disabled editor converter plugin.
quat_path='/Game/Combat/Projectiles/FX/Modules/MakeQuatFromEuler'
quat=unreal.load_asset(quat_path)
if not quat:
    quat=unreal.EditorAssetLibrary.duplicate_asset(Paths.di_quaternion_from_euler,quat_path)
    assert quat
    unreal.EditorAssetLibrary.save_loaded_asset(quat)
Paths.di_quaternion_from_euler=quat.get_path_name()
report = {'source': source.get_path_name(), 'destination': destination, 'emitters': [], 'conversion_errors': []}
size_groups={}
for emitter in utils.get_cascade_system_emitters(source):
    lod = utils.get_cascade_emitter_lod_level(emitter, 0)
    required = utils.get_lod_level_required_module(lod)
    modules = utils.get_lod_level_modules(lod)
    sizes=[m for m in modules if m.get_class().get_name()=='ParticleModuleSizeMultiplyLife' and utils.get_particle_module_is_enabled(m)]
    for m in sizes: size_groups[m.get_path_name()]=sizes
    report['emitters'].append({
        'name': str(utils.get_cascade_emitter_name(emitter)),
        'enabled': utils.get_lod_level_is_enabled(lod),
        'required': str(utils.get_particle_module_required_per_emitter_props(required)),
        'renderer': str(utils.get_particle_module_required_per_renderer_props(required)),
        'modules': [{'class': m.get_class().get_name(), 'enabled': utils.get_particle_module_is_enabled(m)} for m in modules],
        'typedata': str(utils.get_lod_level_type_data_module(lod)),
    })
Path(unreal.Paths.project_saved_dir(), 'IceShotNiagaraConversion.json').write_text(json.dumps(report,indent=2))
original_exit = converter.ConverterContextMgr.__exit__
original_import=converter.import_fx_converter_classes
processed=set()
def compound_size(cls,args):
    emitter=args.get_niagara_emitter_context()
    key=emitter.get_path_name()
    if key in processed: return
    processed.add(key)
    mesh=emitter.find_renderer('MeshRenderer') is not None
    input_type=unreal.NiagaraScriptInputType.VEC3 if mesh else unreal.NiagaraScriptInputType.FLOAT
    combined=None
    for module in size_groups[args.get_cascade_module().get_path_name()]:
        distribution,x,y,z=utils.get_particle_module_size_multiply_life_props(module)
        assert x and y and z, 'This effect uses scale on every axis'
        options=helpers.DistributionConversionOptions()
        if not mesh:
            options.set_target_type_width(input_type); options.set_target_vector_component('x')
        value=helpers.create_script_input_for_distribution(distribution,options)
        if combined is None: combined=value
        else:
            multiply=utils.create_script_context(unreal.CreateScriptContextArgs(utils.create_asset_data(Paths.di_multiply_vector if mesh else Paths.di_multiply_float)))
            multiply.set_parameter('A',combined);multiply.set_parameter('B',value)
            combined=utils.create_script_input_dynamic(multiply,input_type)
    path=Paths.script_scale_mesh_size if mesh else Paths.script_scale_sprite_size
    script_args=unreal.CreateScriptContextArgs(utils.create_asset_data(path)) if mesh else unreal.CreateScriptContextArgs(utils.create_asset_data(path),[1,1])
    script=emitter.find_or_add_module_script('CompoundSizeOverLife',script_args,unreal.ScriptExecutionCategory.PARTICLE_UPDATE)
    if mesh: script.set_parameter('Scale Factor',combined)
    else:
        script.set_parameter('Scale Sprite Size Mode',utils.create_script_input_enum(Paths.enum_niagara_scale_sprite_size,'Uniform'))
        script.set_parameter('Uniform Scale Factor',combined)
def refined_import(path,base):
    classes=original_import(path,base)
    for cls in classes:
        if cls.__name__=='CascadeSizeMultiplyLifeConverter': cls.convert=classmethod(compound_size)
    return classes
converter.import_fx_converter_classes=refined_import
def capture_exit(self, typ, value, tb):
    if tb:
        report['conversion_errors'].append({'module': str(self.cascade_module), 'error': ''.join(traceback.format_exception(typ,value,tb))})
    return original_exit(self,typ,value,tb)
converter.ConverterContextMgr.__exit__ = capture_exit
results = unreal.ConvertCascadeToNiagaraResults()
converter.convert_cascade_to_niagara(source, results)
converted = unreal.load_asset('/Game/StylizedToonShotFX04/Particles/P_ky_shotIce1_Converted')
assert converted
assert unreal.EditorAssetLibrary.rename_asset(converted.get_path_name(), destination)
assert unreal.AscendMeleeEditorLibrary.ensure_niagara_direction_parameter(converted, 'User.ProjectileDirection')
unreal.EditorAssetLibrary.save_loaded_asset(converted, only_if_is_dirty=False)
report['cancelled'] = results.get_editor_property('cancelled_by_user')
Path(unreal.Paths.project_saved_dir(), 'IceShotNiagaraConversion.json').write_text(json.dumps(report,indent=2))
unreal.log('ICE_SHOT_NIAGARA_REBUILT')
unreal.SystemLibrary.quit_editor()
