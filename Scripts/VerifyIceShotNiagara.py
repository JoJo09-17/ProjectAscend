import unreal, json, time
from pathlib import Path

system=unreal.load_asset('/Game/Combat/Projectiles/FX/NS_Archer_IceShot')
assert system
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actor=sub.spawn_actor_from_class(unreal.NiagaraActor,unreal.Vector(0,0,200))
component=actor.get_component_by_class(unreal.NiagaraComponent)
component.set_asset(system)
component.set_variable_vec3('User.ProjectileDirection',unreal.Vector(1,0,0))
component.set_force_solo(True)
component.activate(True)
actor.set_actor_label('IceShotVerification')
report={'system':system.get_path_name(),'particle_counts':{},'user_parameters':[str(n) for n in unreal.AscendMeleeEditorLibrary.get_niagara_user_parameter_names(system)]}
options=unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,include_soft_package_references=True)
registry=unreal.AssetRegistryHelpers.get_asset_registry()
report['dependencies']=[str(n) for n in registry.get_dependencies('/Game/Combat/Projectiles/FX/NS_Archer_IceShot',options)]
assert not any(n.startswith('/CascadeToNiagaraConverter/') for n in report['dependencies'])
def counts():
 cache=unreal.NiagaraSimCacheFunctionLibrary.create_niagara_sim_cache(component)
 result=unreal.NiagaraSimCacheFunctionLibrary.capture_niagara_sim_cache_immediate(cache,unreal.NiagaraSimCacheCreateParameters(),component)
 report['capture_result']=str(result)
 report['active']=component.is_active()
 report['capture_doc']=unreal.NiagaraSimCacheFunctionLibrary.capture_niagara_sim_cache_immediate.__doc__
 cache=result[1] if isinstance(result,tuple) else result
 assert cache and cache.get_num_frames()>0, 'Niagara simulation cache could not capture a live system'
 return {name:len(cache.read_position_attribute('Position',name)) for name in ['shot','shotCore','dustS','smoke']}
started=time.monotonic()
def verify(delta):
 global actor,component
 if time.monotonic()-started<3: return
 unreal.unregister_slate_post_tick_callback(handle)
 try:
  component.set_force_solo(True)
  component.activate(True)
  component.advance_simulation(1,1/60)
  report['particle_counts']=counts()
  # A moving source is necessary for the original dust/smoke spawn-per-distance trails.
  for step in range(1,31):
   actor.set_actor_location(unreal.Vector(step*10,0,200),False,False)
   component.advance_simulation(1,1/60)
  moved_counts=counts()
  for name in ['dustS','smoke']: report['particle_counts'][name]=max(report['particle_counts'][name],moved_counts[name])
  assert all(report['particle_counts'].values()), 'One or more layers produced no particles'
  root='/Game/Combat/Projectiles/Profiles'
  name='DA_Archer_Projectile_IceShot'
  da=unreal.load_asset(root+'/'+name)
  if not da:
   factory=unreal.DataAssetFactory();factory.set_editor_property('data_asset_class',unreal.AscendProjectileProfile)
   da=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,root,unreal.AscendProjectileProfile,factory)
   da.set_editor_property('flight_niagara',system)
   unreal.EditorAssetLibrary.save_loaded_asset(da)
  report['projectile_profile']=da.get_path_name()
  report['status']='complete'
 except Exception as error:
  report['error']=str(error)
 Path(unreal.Paths.project_saved_dir(),'IceShotNiagaraVerification.json').write_text(json.dumps(report,indent=2))
 sub.destroy_actor(actor)
 unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(verify)
