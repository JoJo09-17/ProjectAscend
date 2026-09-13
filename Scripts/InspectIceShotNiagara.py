import unreal,json
from pathlib import Path
utils=unreal.FXConverterUtilitiesLibrary
source=unreal.load_asset('/Game/StylizedToonShotFX04/Particles/P_ky_shotIce1')
system=unreal.load_asset('/Game/Combat/Projectiles/FX/NS_Archer_IceShot')
report={'emitters':[],'niagara':{}}
for emitter in utils.get_cascade_system_emitters(source):
 lod=utils.get_cascade_emitter_lod_level(emitter,0)
 info={'name':str(utils.get_cascade_emitter_name(emitter)), 'size_curves':[]}
 for m in utils.get_lod_level_modules(lod):
  if m.get_class().get_name()=='ParticleModuleSizeMultiplyLife':
   dist,x,y,z=utils.get_particle_module_size_multiply_life_props(m)
   entry={'axes':[x,y,z],'distribution':str(dist)}
   for prop in ['constant','min','max','constant_curve']:
    try: entry[prop]=str(dist.get_editor_property(prop))
    except Exception: pass
   info['size_curves'].append(entry)
 report['emitters'].append(info)
for prop in ['emitter_handles','system_spawn_script','system_update_script']:
 try: report['niagara'][prop]=str(system.get_editor_property(prop))
 except Exception as error: report['niagara'][prop]=str(error)
report['methods']=[m for m in dir(system) if 'compile' in m or 'emitter' in m]
Path(unreal.Paths.project_saved_dir(),'IceShotNiagaraInspection.json').write_text(json.dumps(report,indent=2))
unreal.SystemLibrary.quit_editor()
